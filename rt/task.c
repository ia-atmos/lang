// Tasks: tree, spawn, resume, emit, await, abort.

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "atmos.h"

ATask*  a_cur = NULL;
ATrail* a_trl = NULL;
void*   a_join_key = NULL;

// Root of the task tree.
static ATasks ROOT;

// Emit counter: a task only reacts to emits that start after its
// current await was established.
static uint32_t TIME = 1;

// Clock: time to the nearest timer (a lower bound, reported by the
// timers themselves), and what is left of the current tick after the
// piece in progress.
static double CLK_MIN  = INFINITY;
static double CLK_LATE = 0;

// Main chunk, and the program arguments.
static ATask* MAIN = NULL;
static int    ARGC = 0;
static char** ARGV = NULL;

// Exit that a child delivers to a trail resumed with R_EXIT.
static AStatus EXIT = A_OK;

// Resumes in course, from the innermost: each keeps the trail that
// led to it and the depth of the block that trail was in.
typedef struct RFrame {
    ATrail* from;
    int     depth;
    struct RFrame* up;
} RFrame;

static RFrame* RSTK = NULL;

static AStatus node_abort (ANode* n);
static void    pin_deep (AVal v, int d, int how);
static void    clr_deep (AVal v, uint32_t d);
static int     adopt_ok (AVal v, int d, int root);
static void    pin_kid  (AVal v, int d);
static AStatus adopt_err (const char* what);
static void    adopt_end (void);
static void    evt_own (AVal evt);
static AStatus trail_abort (ATask* t, ATrail* tr);
static AStatus trail_resume (ATask* t, ATrail* tr, AKind k);
static void    wait_set (ATrail* tr, AStatus st);
static void    exit_park (ATask* t, ATrail* tr, AStatus st);
static void    trail_stop (ATask* t, ATrail* tr);
static void    node_stop (ANode* n);
static int     trail_bdepth (ATask* t, int i);
static ATrailX* trail_x (ATrail* tr);
static AStatus emit_top   (ANode* n, AVal evt);
static AStatus emit_node  (ANode* n, uint32_t time, AVal evt);
static AStatus emit_node0  (ANode* n, uint32_t time, AVal evt);
static AStatus emit_node1  (ANode* n, uint32_t time, AVal evt);
static AStatus emit_trail0 (ATask* t, ATrail* tr, uint32_t time, AVal evt);
static AStatus emit_trail1 (ATask* t, ATrail* tr, uint32_t time, AVal evt);

// The running trail was affected by what it just did: it was aborted,
// or a child exited. It is told only once: from then on it unwinds,
// and its defers execute normally.
AStatus a_chk_aborted (AStatus st) {
    ATrail* tr = a_trl;
    if (st!=A_OK || tr==NULL) {
        return st;
    }
    if (tr->aborted == 1) {
        tr->aborted = 2;
        return A_ABORT;
    }
    if (tr->pend && tr->aborted==0) {
        tr->pend = 0;
        a_pay_restore(&tr->x->ppay);
        tr->x->ppay.v   = NULL;
        tr->x->ppay.n   = 0;
        tr->x->ppay.cap = 0;
        return tr->x->pst;
    }
    return st;
}

#define chk_aborted a_chk_aborted

///////////////////////////////////////////////////////////////////////////////
// PATTERNS
///////////////////////////////////////////////////////////////////////////////

// A pattern is a tree. Most awaits have a single leaf, which then
// lives inside the trail, with no allocation: an `ARoot` is a leaf, or
// points to a tree.

typedef enum {
    PK_NONE,
    // value, classified when the await starts
    PK_VAL,
    // any event / never
    PK_TRUE, PK_FALSE,
    // event ?? value
    PK_IS,
    // termination of a task
    PK_TASK,
    // duration / any clock tick
    PK_CLK, PK_TICK,
    // :any ts / :all ts
    PK_ANY, PK_ALL,
    // termination of trails `lo` to `lo+n-1` of the awaiting task
    PK_TRAILS,

    // from here on, not a leaf

    // an `ARoot` that points to a tree
    PK_HEAP,
    // T(...): becomes PK_TASK when the await starts
    PK_SPAWN,
    // [p] until (f) / [p] while (f)
    PK_UNTIL, PK_WHILE,
    // !p / p && q / p || q
    PK_NOT, PK_AND, PK_OR
} PKind;

struct APatt {
    // kind, and value, task, prototype, or predicate
    ARoot  r;
    // count given to the pattern, which `r.cnt` starts from
    int32_t n0;
    // arguments of a spawn
    AVal   args;
    APatt* a;
    APatt* b;
    // &&: operands already satisfied, and their first results
    int    da;
    int    db;
    AVal   ra;
    AVal   rb;
};

static APatt* patt_new (PKind kind) {
    APatt* p = calloc(1, sizeof(APatt));
    p->r.kind = kind;
    return p;
}

APatt* a_pt_val (AVal v) {
    APatt* p = patt_new(PK_VAL);
    p->r.v = v;
    a_inc(v);
    return p;
}

APatt* a_pt_spawn (AVal proto, AVal args) {
    APatt* p = patt_new(PK_SPAWN);
    p->r.v  = proto;
    p->args = args;
    a_inc(proto);
    a_inc(args);
    return p;
}

APatt* a_pt_pred (int until, APatt* base, AVal f) {
    APatt* p = patt_new(until ? PK_UNTIL : PK_WHILE);
    p->a = base;
    p->r.v = f;
    a_inc(f);
    return p;
}

APatt* a_pt_pool (int all, AVal ts) {
    APatt* p = patt_new(all ? PK_ALL : PK_ANY);
    p->r.v = ts;
    a_inc(ts);
    return p;
}

APatt* a_pt_not (APatt* a) {
    APatt* p = patt_new(PK_NOT);
    p->a = a;
    return p;
}

APatt* a_pt_bin (int and, APatt* a, APatt* b) {
    APatt* p = patt_new(and ? PK_AND : PK_OR);
    p->a = a;
    p->b = b;
    return p;
}

// Count of a pattern, as `ARoot.cnt` keeps it: negative for no times.
// Fails if it is not a number.
#define CNT_ERR INT32_MIN
static int32_t cnt_of (AVal n) {
    if (!a_isnum(n)) {
        return CNT_ERR;
    }
    double d = a_dbl(n);
    if (d < 1) {
        return -1;
    }
    return (d >= INT32_MAX) ? INT32_MAX : (int32_t) d;
}

static AStatus cnt_err (void) {
    return a_error("invalid await : expected number");
}

APatt* a_pt_cnt (AVal n, APatt* a) {
    int32_t c = cnt_of(n);
    if (c!=CNT_ERR && a->n0>0) {
        // a count of a count
        int64_t m = (c < 0) ? -1 : (int64_t)c * a->n0;
        c = (m > INT32_MAX) ? INT32_MAX : (int32_t) m;
    }
    if (a->n0 >= 0) {
        a->n0 = c;
    }
    return a;
}

// Aborts the tasks the pattern spawned.
static void patt_stop (APatt* p) {
    if (p == NULL) {
        return;
    }
    if (p->r.kind==PK_TASK && p->r.own) {
        node_abort((ANode*)p->r.v.u.o);
    }
    patt_stop(p->a);
    patt_stop(p->b);
}

static void patt_free (APatt* p) {
    if (p == NULL) {
        return;
    }
    patt_stop(p);
    patt_free(p->a);
    patt_free(p->b);
    a_dec(p->r.v);
    a_dec(p->args);
    a_dec(p->ra);
    a_dec(p->rb);
    free(p);
}

// Releases the pattern of an await that is over. The trail is free to
// await again meanwhile, since the abort of a spawned task runs code.
static void root_free (ARoot* root) {
    ARoot r = *root;
    root->kind = PK_NONE;
    root->own  = 0;
    root->v    = a_nil();
    if (r.kind == PK_HEAP) {
        patt_free(r.u.p);
    } else if (r.kind != PK_NONE) {
        if (r.kind==PK_TASK && r.own) {
            node_abort((ANode*)r.v.u.o);
        }
        a_dec(r.v);
    }
}

static void pay1 (AVal v) {
    a_pay_clear();
    a_pay_addc(v);
}

// The results of the await of a task: its final value, and the task.
// A task that the pattern itself spawned (`await T()`) gives its value
// away, as a call does: nobody else has the task, and the value comes
// out with no other reference, so that it may be pinned.
static void pay_task (AVal t, int own) {
    a_pay_clear();
    if (own) {
        a_pay_add(A_TASK_OF(t)->ret);
        A_TASK_OF(t)->ret = a_nil();
    } else {
        a_pay_addc(A_TASK_OF(t)->ret);
    }
    a_pay_addc(t);
}

// Calls a predicate on a value. The result is the first value
// returned (owned).
static AStatus pred (AVal f, AVal it, AVal* res) {
    int base = a_top;
    a_pushc(it);
    AStatus st = a_call(f, base);
    if (st != A_OK) {
        return st;
    }
    *res = a_nil();
    a_ret1(res, base);
    return A_OK;
}

// The base of an until/while was satisfied with `it` (the first value
// in the payload): applies the predicate.
//  - until: holds if the result does, which replaces `it` unless true
//  - while: holds if the result does not, with `it`
static AStatus patt_pred (APatt* p, int* done) {
    AVal it = a_nil();
    a_pay_take(&it, 0);
    a_pay_clear();
    AVal res;
    AStatus st = pred(p->r.v, it, &res);
    if (st != A_OK) {
        a_dec(it);
        return st;
    }
    int ok = a_truthy(res);
    if (p->r.kind == PK_UNTIL) {
        *done = ok;
        if (ok) {
            pay1((res.type==A_BOOL) ? it : res);
        }
    } else {
        *done = !ok;
        if (!ok) {
            pay1(it);
        }
    }
    a_dec(it);
    a_dec(res);
    return A_OK;
}

static AStatus patt_start (APatt* p, int* done);

// The `n` trails of a task from `lo` on have all terminated.
static int trails_off (ATask* t, int lo, int n) {
    for (int i=lo; i<lo+n; i++) {
        if (t->trs[i].status != TR_OFF) {
            return 0;
        }
    }
    return 1;
}

// A timer expires with how late it is, or reports the time it still
// needs.
static void leaf_clk (ARoot* r, int* done) {
    if (r->u.clk <= 0) {
        pay1(a_numi(CLK_LATE - r->u.clk));
        *done = 1;
    } else if (r->u.clk < CLK_MIN) {
        CLK_MIN = r->u.clk;
    }
}

// Establishes a leaf once. Sets `done` if it is satisfied before any
// event, with the results in the payload.
static AStatus leaf_start1 (ARoot* r, int* done) {
    *done = 0;
    switch (r->kind) {
        case PK_VAL:
            switch (r->v.type) {
                case A_BOOL:
                    r->kind = r->v.u.b ? PK_TRUE : PK_FALSE;
                    break;
                case A_TAB:
                    r->kind = PK_IS;
                    break;
                case A_STR:
                    r->kind = !strcmp(A_STR_OF(r->v)->s,"clock") ?
                        PK_TICK : PK_IS;
                    break;
                case A_INT:
                case A_NUM:
                    r->kind = PK_CLK;
                    return leaf_start1(r, done);
                case A_XTASK:
                    r->kind = PK_TASK;
                    return leaf_start1(r, done);
                case A_NIL:
                    return a_error("invalid await : invalid event pattern");
                case A_TASKS:
                    return a_error("invalid await : unexpected tasks pool");
                case A_FUNC:
                    return a_error("invalid await : unexpected function");
                default:
                    return a_error("invalid await : unexpected %s",
                        a_typename(r->v));
            }
            break;

        case PK_CLK:
            // a timer counts down from its full duration
            r->u.clk = a_dbl(r->v);
            leaf_clk(r, done);
            break;

        case PK_TASK:
            if (A_TASK_OF(r->v)->status == T_DONE) {
                pay_task(r->v, r->own);
                *done = 1;
            }
            break;

        case PK_ANY:
        case PK_ALL:
            if (r->v.type != A_TASKS) {
                return a_error("invalid await : expected tasks pool");
            }
            // a pool that is already empty has drained: no value, no task
            if (r->kind==PK_ALL && a_tasks_len(A_TASKS_OF(r->v))==0) {
                a_pay_clear();
                a_pay_add(a_nil());
                a_pay_add(a_nil());
                a_pay_addc(r->v);
                *done = 1;
            }
            break;

        case PK_TRAILS:
            *done = trails_off(a_cur, r->u.trs.lo, r->u.trs.n);
            if (*done) {
                a_pay_clear();
            }
            break;

        default:
            break;
    }
    return A_OK;
}

// Establishes a leaf. With a count, each time it is satisfied at once
// counts, and it only holds at the last one.
static AStatus leaf_start (ARoot* r, int* done) {
    if (r->cnt < 0) {
        // no times
        a_pay_clear();
        *done = 1;
        return (r->cnt == CNT_ERR) ? cnt_err() : A_OK;
    }
    AStatus st = leaf_start1(r, done);
    while (st==A_OK && *done && r->cnt>1) {
        r->cnt--;
        st = leaf_start1(r, done);
    }
    return st;
}

static void leaf_match1 (ARoot* r, ATask* t, AVal evt, int* done);

// Tests an event against a leaf. With a count, it starts again until
// its last time.
static void leaf_match (ARoot* r, ATask* t, AVal evt, int* done) {
    leaf_match1(r, t, evt, done);
    if (*done && r->cnt>1) {
        r->cnt--;
        // the kind is known by now: it cannot fail
        (void) leaf_start(r, done);
    }
}

// Tests an event against a leaf once. Sets `done` if it is satisfied,
// with the results in the payload.
static void leaf_match1 (ARoot* r, ATask* t, AVal evt, int* done) {
    *done = 0;
    switch (r->kind) {
        case PK_TRUE:
            pay1(evt);
            *done = 1;
            break;

        case PK_IS:
            if (a_is(evt, r->v)) {
                pay1(evt);
                *done = 1;
            }
            break;

        case PK_TASK:
            if (a_eq(evt, r->v)) {
                pay_task(r->v, r->own);
                *done = 1;
            }
            break;

        case PK_CLK:
            if (a_isnum(evt)) {
                r->u.clk -= a_dbl(evt);
                leaf_clk(r, done);
            }
            break;

        case PK_TICK:
            if (a_isnum(evt)) {
                a_pay_clear();
                a_pay_addc(evt);
                a_pay_add(a_numi(CLK_LATE));
                *done = 1;
            }
            break;

        case PK_ANY:
        case PK_ALL:
            // a task of the pool terminated (and it was the last one)
            if (evt.type==A_XTASK && A_TASK_OF(evt)->pool==A_TASKS_OF(r->v)) {
                if (r->kind==PK_ANY || a_tasks_len(A_TASKS_OF(r->v))==0) {
                    pay_task(evt, 0);
                    a_pay_addc(r->v);
                    *done = 1;
                }
            }
            break;

        case PK_TRAILS:
            // a trail of the task terminated (and it was the last one)
            if (evt.type==A_XTASK && A_TASK_OF(evt)->desc==NULL &&
                trails_off(t,r->u.trs.lo,r->u.trs.n))
            {
                a_pay_clear();
                *done = 1;
            }
            break;

        default:
            break;
    }
}

// Starts an until/while again after its predicate rejected the base.
static AStatus patt_again (APatt* p) {
    int done = 0;
    patt_stop(p->a);
    AStatus st = patt_start(p->a, &done);
    // a base that is satisfied at once waits for the next event
    a_pay_clear();
    return st;
}

// Establishes a pattern once. Sets `done` if it is satisfied before
// any event, with the results in the payload.
static AStatus patt_start1 (APatt* p, int* done) {
    AStatus st = A_OK;
    *done = 0;
    switch (p->r.kind) {
        case PK_SPAWN: {
            // the task runs inside the awaiting task
            if (p->r.v.type != A_TASK) {
                return a_error("invalid spawn : expected task prototype");
            }
            int base = a_top;
            a_args_unpack(p->args);
            AVal t = a_nil();
            st = a_spawn(&t, p->r.v, a_nil(), base);
            a_set(&p->r.v, t);
            p->r.kind = PK_TASK;
            // no task, if the spawn failed before it was created (as
            // with a prototype that was finalized)
            p->r.own  = (t.type != A_NIL);
            if (st != A_OK) {
                return st;
            }
            return leaf_start1(&p->r, done);
        }

        case PK_UNTIL:
        case PK_WHILE:
            if (p->a == NULL) {
                // the predicate alone is first tested with no event
                pay1(a_nil());
                return patt_pred(p, done);
            }
            st = patt_start(p->a, done);
            if (st==A_OK && *done) {
                st = patt_pred(p, done);
                if (st==A_OK && !*done) {
                    st = patt_again(p);
                }
            }
            break;

        case PK_NOT:
            st = patt_start(p->a, done);
            *done = 0;
            a_pay_clear();
            break;

        case PK_AND:
            st = patt_start(p->a, &p->da);
            if (st==A_OK && p->da) {
                a_pay_take(&p->ra, 0);
            }
            if (st == A_OK) {
                st = patt_start(p->b, &p->db);
            }
            if (st==A_OK && p->db) {
                a_pay_take(&p->rb, 0);
            }
            if (st==A_OK && p->da && p->db) {
                a_pay_clear();
                a_pay_addc(p->ra);
                a_pay_addc(p->rb);
                *done = 1;
            }
            break;

        case PK_OR:
            st = patt_start(p->a, done);
            if (st==A_OK && !*done) {
                st = patt_start(p->b, done);
            }
            break;

        default:
            return leaf_start1(&p->r, done);
    }
    return st;
}

// Starts a pattern, again or for the first time, from the count it
// has left. Each time it is satisfied at once counts.
static AStatus patt_run (APatt* p, int* done) {
    if (p->r.cnt < 0) {
        // no times
        a_pay_clear();
        *done = 1;
        return (p->r.cnt == CNT_ERR) ? cnt_err() : A_OK;
    }
    for (;;) {
        p->da = 0;
        p->db = 0;
        AStatus st = patt_start1(p, done);
        if (st!=A_OK || !*done || p->r.cnt<=1) {
            return st;
        }
        p->r.cnt--;
        patt_stop(p);
    }
}

// Establishes a pattern. Sets `done` if it is satisfied before any
// event, with the results in the payload.
static AStatus patt_start (APatt* p, int* done) {
    p->r.cnt = p->n0;
    return patt_run(p, done);
}

static AStatus patt_match1 (APatt* p, AVal evt, int* done);

// Tests an event against a pattern. Sets `done` if it is satisfied,
// with the results in the payload. With a count, it starts again
// until its last time.
static AStatus patt_match (APatt* p, AVal evt, int* done) {
    AStatus st = patt_match1(p, evt, done);
    if (st==A_OK && *done && p->r.cnt>1) {
        p->r.cnt--;
        patt_stop(p);
        st = patt_run(p, done);
    }
    return st;
}

static AStatus patt_match1 (APatt* p, AVal evt, int* done) {
    AStatus st = A_OK;
    *done = 0;
    switch (p->r.kind) {
        case PK_UNTIL:
        case PK_WHILE:
            if (p->a == NULL) {
                pay1(evt);
                return patt_pred(p, done);
            }
            st = patt_match(p->a, evt, done);
            if (st==A_OK && *done) {
                st = patt_pred(p, done);
                if (st==A_OK && !*done) {
                    st = patt_again(p);
                }
            }
            return st;

        case PK_NOT:
            st = patt_match(p->a, evt, done);
            if (st != A_OK) {
                return st;
            }
            if (*done) {
                // the operand matched: it starts again
                *done = 0;
                patt_stop(p->a);
                st = patt_start(p->a, done);
                *done = 0;
                a_pay_clear();
                return st;
            }
            pay1(evt);
            *done = 1;
            return A_OK;

        case PK_AND:
            if (!p->da) {
                st = patt_match(p->a, evt, &p->da);
                if (st==A_OK && p->da) {
                    a_pay_take(&p->ra, 0);
                }
            }
            if (st==A_OK && !p->db) {
                st = patt_match(p->b, evt, &p->db);
                if (st==A_OK && p->db) {
                    a_pay_take(&p->rb, 0);
                }
            }
            if (st==A_OK && p->da && p->db) {
                a_pay_clear();
                a_pay_addc(p->ra);
                a_pay_addc(p->rb);
                *done = 1;
            }
            return st;

        case PK_OR:
            st = patt_match(p->a, evt, done);
            if (st==A_OK && !*done) {
                st = patt_match(p->b, evt, done);
            }
            return st;

        default:
            leaf_match1(&p->r, a_cur, evt, done);
            return A_OK;
    }
}

///////////////////////////////////////////////////////////////////////////////
// TREE
///////////////////////////////////////////////////////////////////////////////

// The tree has tasks and pools as nodes. A task keeps its children in
// spawn order, each with the trail that spawned it. The trails of a
// task form a static tree of their own, with siblings in source order,
// which is also the order they start in. The children of a trail are
// then its nested trails and the nodes it spawned, merged in the order
// they started: a trail keeps how many children the task had by then.

static void node_init (ANode* n, AType type) {
    memset(n, 0, sizeof(ANode));
    n->hdr.rc   = 1;
    n->hdr.type = type;
}

// Appends a child, spawned by the running trail if the parent is the
// running task. Takes over one reference.
static void node_add (ANode* up, ANode* n) {
    if (up->ndns == up->cdns) {
        up->cdns = (up->cdns == 0) ? 4 : up->cdns*2;
        up->dns  = realloc(up->dns, up->cdns * sizeof(ANode*));
    }
    up->dns[up->ndns++] = n;
    n->up = up;
    if (up->hdr.type == A_TASKS) {
        ATasks* ts = (ATasks*) up;
        if (ts->seq == UINT32_MAX) {
            // out of numbers: the tasks in the pool take the first ones
            ts->seq = 0;
            for (int i=0; i<up->ndns-1; i++) {
                up->dns[i]->seq = ts->seq++;
            }
        }
        n->seq = ts->seq++;
    }
    if (up == (ANode*)a_cur) {
        n->trail = a_trl->idx;
        a_trl->nkids++;
    }
}

// Removes the dead children, releasing them.
static void node_sweep (ANode* n) {
    ATrail* trs = (n->hdr.type == A_XTASK) ? ((ATask*)n)->trs : NULL;
    int nt = (trs != NULL) ? ((ATask*)n)->desc->ntrails : 0;
    int k = 0;
    for (int i=0; i<n->ndns; i++) {
        ANode* dn = n->dns[i];
        if (dn->dead) {
            if (trs != NULL) {
                trs[dn->trail].nkids--;
                // the trails that started after it move back along
                // with the children that did
                for (int j=1; j<nt; j++) {
                    if (trs[j].rank > (uint32_t)k) {
                        trs[j].rank--;
                    }
                }
            }
            dn->up = NULL;
            a_dec(a_obj(&dn->hdr));
        } else {
            n->dns[k++] = dn;
        }
    }
    n->ndns  = k;
    n->dirty = 0;
}

// A child died: it leaves now, or when the traversals end.
static void node_died (ANode* n) {
    n->dead = 1;
    if (a_nouts > 0) {
        a_out_ended(&n->hdr);
    }
    ANode* up = n->up;
    if (up != NULL) {
        up->dirty = 1;
        if (up->trav == 0) {
            node_sweep(up);
        }
    }
}

static void node_trav (ANode* n, int inc) {
    n->trav += inc;
    if (n->trav==0 && n->dirty) {
        node_sweep(n);
    }
}

// Releases the mute state of a node.
static void node_untoggle (ANode* n) {
    for (int i=0; i<n->nfilt; i++) {
        patt_free(n->filt[i]);
    }
    free(n->filt);
    n->filt  = NULL;
    n->nfilt = 0;
    n->toggled = 0;
}

static ATrailX* trail_x (ATrail* tr) {
    if (tr->x == NULL) {
        tr->x = calloc(1, sizeof(ATrailX));
    }
    return tr->x;
}

// The solos registered in a stage, in all stages: with none, an emit
// pays nothing for stage and solo.
static int NSOLO = 0;

// A solo that ends leaves the stack of its stage; a stage that ends
// leaves its solos without one (they end with it, in its block).
static void grab_off (ATrail* tr, ATrailX* x) {
    if (x->gkind == A_GRAB && x->gup != NULL && x->gup->x != NULL) {
        ATrailX* u = x->gup->x;
        for (int i=0; i<u->gn; i++) {
            if (u->gs[i] == tr) {
                memmove(&u->gs[i], &u->gs[i+1], (u->gn-i-1) * sizeof(ATrail*));
                u->gn--;
                NSOLO--;
                break;
            }
        }
    } else if (x->gkind == A_GRABBABLE) {
        for (int i=0; i<x->gn; i++) {
            if (x->gs[i]->x != NULL) {
                x->gs[i]->x->gup = NULL;
            }
        }
        NSOLO -= x->gn;
    }
    for (int i=0; i<x->ngfilt; i++) {
        patt_free(x->gfilt[i]);
    }
    free(x->gfilt);
    free(x->gs);
    free(x->gpath);
}

///////////////////////////////////////////////////////////////////////////////
// CONTEXT
///////////////////////////////////////////////////////////////////////////////

// A context is a trail whose `enter` runs before code that lives in its
// subtree runs, and whose `leave` runs before code outside runs again.
// The contexts applied now, from the outermost: the switch is lazy, so
// two trails of the same context resumed in a row pay nothing between
// them, and a subtree in which nothing awakes pays nothing at all.

#define CTX_MAX 64
static ATrail* CTX[CTX_MAX];
static int     NCTX = 0;
// contexts alive: with none, and none applied, the hooks cost nothing
static int     LIVE = 0;
// running `enter` or `leave`: `emit` and `spawn` are errors there
static int     INCTX = 0;
// set by the traversal of an emit just before it resumes a trail: the
// switch back to the resumer waits for the end of the traversal
static int     LAZY = 0;

#define CTX_ON() (LIVE>0 || NCTX>0)

// The emit being traversed, for the filters of `context <P>`: time 0
// outside an emit (a spawn, an abort from code), where they never pass.
static uint32_t EVT_TIME = 0;
static AVal     EVT_VAL;

static AStatus filt_start (APatt** fs, int n);
static AStatus filt_pass  (APatt** fs, int n, AVal evt, int* pass);

// Whether a context applies to the emit being traversed.
static int ctx_pass (ATrailX* x) {
    if (x->ncfilt == 0) {
        return 1;
    }
    if (EVT_TIME == 0) {
        return 0;
    }
    if (x->ctime != EVT_TIME) {
        // the match clears the payload: keep the one being delivered
        // (the value of a task that ended, of an await)
        APay pay;
        a_pay_save(&pay);
        int pass = 0;
        if (filt_pass(x->cfilt, x->ncfilt, EVT_VAL, &pass) != A_OK) {
            pass = 0;
        }
        a_pay_restore(&pay);
        x->ctime = EVT_TIME;
        x->cpass = pass;
    }
    return x->cpass;
}

// The contexts around trail `i` of task `t`, from the innermost.
static int ctx_chain (ATask* t, int i, ATrail** out) {
    int n = 0;
    while (t != NULL) {
        if (t->trs != NULL) {
            const ATrailD* ds = t->desc->trails;
            for (int j=i; j>=0; j=ds[j].up) {
                ATrail* tr = &t->trs[j];
                if (tr->x!=NULL && tr->x->ctx && tr->status!=TR_OFF &&
                    ctx_pass(tr->x))
                {
                    if (n == CTX_MAX) {
                        return n;
                    }
                    out[n++] = tr;
                }
            }
        }
        // where the task lives: the trail of the task that holds it, or
        // that holds its pool
        ANode* nd = &t->node;
        t = NULL;
        while (nd->up != NULL) {
            i  = nd->trail;
            nd = nd->up;
            if (nd->hdr.type == A_XTASK) {
                t = (ATask*) nd;
                break;
            }
        }
    }
    return n;
}

static AStatus ctx_call (AVal f, AVal arg, int has, AVal* res) {
    int base = a_top;
    if (has) {
        a_pushc(arg);
    }
    INCTX++;
    AStatus st = a_call(f, base);
    INCTX--;
    if (st != A_OK) {
        return st;
    }
    AVal r = a_nil();
    a_ret1(&r, base);
    if (res != NULL) {
        *res = r;
    } else {
        a_dec(r);
    }
    return A_OK;
}

// Leaves the applied contexts down to `n` of them, innermost first.
static AStatus ctx_leave (int n) {
    AStatus ret = A_OK;
    while (NCTX > n) {
        ATrailX* x = CTX[--NCTX]->x;
        AVal v = x->cval;
        x->cval = a_nil();
        AStatus st = ctx_call(x->leave, v, 1, NULL);
        a_dec(v);
        if (ret == A_OK) {
            ret = st;
        }
    }
    return ret;
}

// Applies the contexts around trail `i` of task `t` (none for NULL).
static AStatus ctx_switch (ATask* t, int i) {
    ATrail* ch[CTX_MAX];
    int n = (t == NULL) ? 0 : ctx_chain(t, i, ch);
    // common prefix, from the outermost
    int p = 0;
    while (p<NCTX && p<n && CTX[p]==ch[n-1-p]) {
        p++;
    }
    AStatus ret = ctx_leave(p);
    for (int k=p; k<n && ret==A_OK; k++) {
        ATrail* tr = ch[n-1-k];
        ATrailX* x = tr->x;
        CTX[NCTX++] = tr;
        AStatus st = ctx_call(x->enter, a_nil(), 0, &x->cval);
        if (st != A_OK) {
            NCTX--;
            ret = st;
        }
    }
    return ret;
}

// Back to the contexts of the code that runs now.
static AStatus ctx_here (void) {
    if (!CTX_ON()) {
        return A_OK;
    }
    return ctx_switch(a_cur, (a_trl == NULL) ? 0 : a_trl->idx);
}

AStatus a_trail_context (ATask* t, int i, AVal enter, AVal leave) {
    if (enter.type != A_FUNC) {
        return a_error("invalid context : expected function : got %s",
            a_typename(enter));
    }
    if (leave.type != A_FUNC) {
        return a_error("invalid context : expected function : got %s",
            a_typename(leave));
    }
    ATrailX* x = trail_x(&t->trs[i]);
    x->ctx = 1;
    a_cpy(&x->enter, enter);
    a_cpy(&x->leave, leave);
    x->cval = a_nil();
    LIVE++;
    return A_OK;
}

// context X {...}: X holds the pair, enter at 0 and leave at 1.
AStatus a_trail_context_pair (ATask* t, int i, AVal pair) {
    if (pair.type != A_TAB) {
        return a_error("invalid context : expected table : got %s",
            a_typename(pair));
    }
    AVal en = a_nil();
    AVal lv = a_nil();
    if (a_index(&en, pair, a_int(0)) != A_OK) {
        return A_ERROR;
    }
    if (a_index(&lv, pair, a_int(1)) != A_OK) {
        a_dec(en);
        return A_ERROR;
    }
    AStatus st = a_trail_context(t, i, en, lv);
    a_dec(en);
    a_dec(lv);
    return st;
}

AStatus a_trail_context_filt (ATask* t, int i, APatt** fs, int n) {
    ATrailX* x = trail_x(&t->trs[i]);
    x->cfilt  = malloc((n > 0 ? n : 1) * sizeof(APatt*));
    memcpy(x->cfilt, fs, n * sizeof(APatt*));
    x->ncfilt = n;
    x->ctime  = 0;
    x->cpass  = 0;
    return filt_start(x->cfilt, x->ncfilt);
}

// A context trail ends: left if applied, with those inside it.
static void ctx_drop (ATrail* tr) {
    for (int k=0; k<NCTX; k++) {
        if (CTX[k] == tr) {
            ctx_leave(k);
            break;
        }
    }
    ATrailX* x = tr->x;
    a_dec(x->enter);
    a_dec(x->leave);
    a_dec(x->cval);
    for (int i=0; i<x->ncfilt; i++) {
        patt_free(x->cfilt[i]);
    }
    free(x->cfilt);
    x->cfilt  = NULL;
    x->ncfilt = 0;
    LIVE--;
}

static AStatus ctx_chk (const char* op) {
    if (INCTX > 0) {
        return a_error("invalid %s : inside enter or leave of a context", op);
    }
    return A_OK;
}

// Releases the state of a trail, which becomes inactive.
static void trail_off (ATrail* tr) {
    if (tr->root.kind != PK_NONE) {
        root_free(&tr->root);
    }
    if (tr->x!=NULL && tr->x->ctx) {
        ctx_drop(tr);
    }
    ATrailX* x = tr->x;
    if (x != NULL) {
        tr->x = NULL;
        for (int i=0; i<x->nfilt; i++) {
            patt_free(x->filt[i]);
        }
        free(x->filt);
        grab_off(tr, x);
        a_dec(x->tog);
        a_pay_drop(&x->wpay);
        a_pay_drop(&x->ppay);
        for (int i=0; i<x->wn; i++) {
            a_dec(x->wstk[i]);
        }
        free(x->wstk);
        free(x);
    }
    tr->pend    = 0;
    tr->status  = TR_OFF;
    tr->aborted = 0;
    tr->waiting = 0;
    tr->doomed  = 0;
    tr->toggled = 0;
}

// Releases the frame of a task that terminated.
static void frame_free (ATask* t) {
    AFrame* f = t->frame;
    if (f == NULL) {
        return;
    }
    AVal* L = t->L;
    t->frame = NULL;
    t->L     = NULL;
    t->trs   = NULL;
    for (int i=0; i<f->nt; i++) {
        trail_off(&f->trs[i]);
    }
    a_clrs(L, f->n);
    free(f);
}

void a_node_free (ANode* n) {
    node_untoggle(n);
    if (n->hdr.type == A_XTASK) {
        ATask* t = (ATask*) n;
        frame_free(t);
        if (t->proto != NULL) {
            a_dec(a_obj(&t->proto->hdr));
        }
        a_dec(t->pub);
        a_dec(t->ret);
        if (t->pool != NULL) {
            // leaves the list of the pool
            if (t->zprev != NULL) {
                t->zprev->znext = t->znext;
            } else {
                t->pool->all = t->znext;
            }
            if (t->znext != NULL) {
                t->znext->zprev = t->zprev;
            }
        }
    } else if (n->hdr.type == A_TASKS) {
        // the tasks that outlive the pool no longer refer to it
        ATask* t = ((ATasks*)n)->all;
        while (t != NULL) {
            ATask* nxt = t->znext;
            t->pool  = NULL;
            t->zprev = NULL;
            t->znext = NULL;
            t = nxt;
        }
    }
    free(n->dns);
}

// Depth of the block that a task or pool is attached to: its pin
// block, or the block of the spawn while it is not pinned.
static int node_adepth (ANode* n) {
    if (n->hdr.type==A_XTASK && ((ATask*)n)->pool!=NULL) {
        // a task in a pool follows the pin of the pool
        return node_adepth(&((ATask*)n)->pool->node);
    } else if (n->hdr.pinned) {
        return n->hdr.depth;
    } else if (n->hdr.type == A_XTASK) {
        return ((ATask*)n)->depth - 1;
    } else {
        return 0;
    }
}

// `n` is the running task or one of its ancestors.
static int is_up (ANode* n) {
    for (ANode* x=(ANode*)a_cur; x!=NULL; x=x->up) {
        if (x == n) {
            return 1;
        }
    }
    return 0;
}

// Iteration over the children of a trail, in the order they started.
// The task must be under traversal meanwhile, so that its children
// keep their positions. Children that start meanwhile may or may not
// be visited.
typedef struct KIt {
    ATask* t;
    int    up;
    int    cnt;
    int    k;
    int    j;
} KIt;

static void kit_init (KIt* it, ATask* t, ATrail* tr) {
    it->t   = t;
    it->up  = tr->idx;
    it->cnt = t->node.ndns;
    it->k   = (tr->nkids > 0) ? 0 : it->cnt;
    it->j   = t->desc->trails[tr->idx].kid;
}

// Next child: 1 for a trail, 2 for a node, 0 at the end.
static int kit_next (KIt* it, ATrail** tr, ANode** n) {
    ATask* t = it->t;
    if (t->trs == NULL) {
        // the task terminated meanwhile
        return 0;
    }
    while (it->k<it->cnt && t->node.dns[it->k]->trail!=it->up) {
        it->k++;
    }
    while (it->j>=0 && t->trs[it->j].status==TR_OFF) {
        it->j = t->desc->trails[it->j].next;
    }
    int hk = (it->k < it->cnt);
    if (it->j>=0 && (!hk || t->trs[it->j].rank<=(uint32_t)it->k)) {
        *tr = &t->trs[it->j];
        it->j = t->desc->trails[it->j].next;
        return 1;
    }
    if (hk) {
        *n = t->node.dns[it->k++];
        return 2;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// RESUME
///////////////////////////////////////////////////////////////////////////////

// Termination of a transparent block: it has no identity and is not
// an event. Only what awaits it by structure goes on: its parent, if
// it awaits the trails of a `par :all` or of a `toggle` and this was
// the last one. The parent is resumed directly, with no emit.
static AStatus trail_wake (ATask* t, int idx) {
    int up = t->desc->trails[idx].up;
    if (up < 0) {
        return A_OK;
    }
    ATrail* p = &t->trs[up];
    if (p->status!=TR_SUSPENDED || p->waiting || p->doomed ||
        p->root.kind!=PK_TRAILS ||
        idx<p->root.u.trs.lo || idx>=p->root.u.trs.lo+p->root.u.trs.n ||
        !trails_off(t, p->root.u.trs.lo, p->root.u.trs.n))
    {
        return A_OK;
    }
    a_pay_clear();
    return trail_resume(t, p, R_EVENT);
}

// The trail terminated with `st`. Returns the status its resumer
// continues with.
static AStatus trail_end (ATask* t, ATrail* tr, AStatus st) {
    // the result of a transparent block arrives in the payload: its
    // destination, if any, already has it
    if (tr->idx>0 && st==A_OK) {
        a_pay_clear();
    }
    if (tr->root.kind != PK_NONE) {
        root_free(&tr->root);
    }

    // its blocks already ended what it started: these are leftovers,
    // such as a pool that was never pinned
    if (tr->nkids>0 || t->desc->trails[tr->idx].kid>=0) {
        node_trav(&t->node, 1);
        KIt it;
        kit_init(&it, t, tr);
        ATrail* c = NULL;
        ANode*  n = NULL;
        int kind;
        while ((kind = kit_next(&it,&c,&n)) != 0) {
            if (kind == 1) {
                trail_abort(t, c);
            } else {
                node_abort(n);
                if (n->hdr.type==A_TASKS && !n->dead) {
                    node_died(n);
                }
            }
        }
        node_trav(&t->node, -1);
    }

    ATrail* w = (tr->x != NULL) ? tr->x->waiter : NULL;
    int idx = tr->idx;
    trail_off(tr);

    if (idx > 0) {
        // a transparent block
        switch (st) {
            case A_OK:
                st = trail_wake(t, idx);
                break;
            case A_ABORT:
                st = A_OK;
                break;
            default:
                break;
        }
    } else {
        // the task itself: the frame goes, the header stays
        t->status = T_DONE;
        frame_free(t);
        // its final value is kept by the task: pinned with it, if unpinned
        {
            ANode* o = (t->pool != NULL) ? &t->pool->node : &t->node;
            int d = node_adepth(&t->node);
            if (o->hdr.pinned && a_pdepth(t->ret)<=d) {
                pin_kid(t->ret, d);
            }
        }
        a_dec(a_obj(&t->proto->hdr));
        t->proto = NULL;

        // owner: the enclosing task (beyond its pool)
        ANode* own = t->node.up;
        if (own!=NULL && own->hdr.type==A_TASKS) {
            own = own->up;
        }
        node_died(&t->node);

        switch (st) {
            case A_OK:
                // termination is an emit of the task itself to the
                // subtree of its owner
                st = emit_top((own != NULL) ? own : &ROOT.node,
                    a_obj(&t->node.hdr));
                break;
            case A_ABORT:
                st = A_OK;
                break;
            default:
                break;
        }
    }

    // a block waited for this trail to go on with its exit: an exit of
    // the trail replaces the pending one, as if it had ended in time
    if (w != NULL) {
        if (st != A_OK) {
            wait_set(w, st);
        }
        st = trail_resume(A_TRAIL_TASK(w), w, R_LATE);
    }
    return st;
}

static AStatus trail_resume (ATask* t, ATrail* tr, AKind k) {
    ATask*  ptask  = a_cur;
    ATrail* ptrail = a_trl;
    AVal me = a_obj(&t->node.hdr);
    a_inc(me);
    a_cur = t;
    a_trl = tr;
    tr->status = TR_RUNNING;
    if (tr->idx == 0) {
        t->status = T_RUNNING;
    }
    int lin = a_lin;
    // the contexts of the trail; back to those of the resumer at the
    // end, unless the traversal of an emit resumes it (then at the end
    // of the traversal)
    int lazy = LAZY;
    LAZY = 0;
    AStatus cst = A_OK;
    if (CTX_ON()) {
        cst = ctx_switch(t, tr->idx);
        if (cst!=A_OK && k==R_EVENT) {
            // raised at the await of the trail
            EXIT = cst;
            k = R_EXIT;
            cst = A_OK;
        }
    }
    RFrame rf = { ptrail, a_depth, RSTK };
    RSTK = &rf;
    AStatus st = t->desc->trails[tr->idx].resume(t, tr, k);
    RSTK = rf.up;
    a_lin   = lin;
    a_depth = rf.depth;
    a_cur = ptask;
    a_trl = ptrail;
    if (!lazy && CTX_ON()) {
        AStatus st2 = ctx_switch(ptask, (ptrail == NULL) ? 0 : ptrail->idx);
        if (cst == A_OK) {
            cst = st2;
        }
    }
    if (st == A_AWAIT) {
        tr->status = TR_SUSPENDED;
        if (tr->idx == 0) {
            t->status = T_SUSPENDED;
        }
        st = A_OK;
    } else {
        st = trail_end(t, tr, st);
    }
    if (st == A_OK) {
        // an error in `enter` or `leave` is raised in the resumer
        st = cst;
    }
    a_dec(me);
    return st;
}

///////////////////////////////////////////////////////////////////////////////
// ABORT
///////////////////////////////////////////////////////////////////////////////

// Replaces the exit that a waiting trail keeps: the payload becomes
// the current one.
static void wait_set (ATrail* tr, AStatus st) {
    ATrailX* x = trail_x(tr);
    a_pay_drop(&x->wpay);
    for (int i=0; i<x->wn; i++) {
        a_dec(x->wstk[i]);
    }
    free(x->wstk);
    x->wstk = NULL;
    x->wn   = 0;
    a_pay_save(&x->wpay);
    x->wst = st;
}

// Children first, in the order they started, then the trail itself: a
// suspended trail unwinds now; a running one, when control returns to
// it.
static AStatus trail_abort (ATask* t, ATrail* tr) {
    if (tr->status == TR_OFF) {
        return A_OK;
    }
    AStatus st = A_OK;
    if (tr->nkids>0 || t->desc->trails[tr->idx].kid>=0) {
        node_trav(&t->node, 1);
        KIt it;
        kit_init(&it, t, tr);
        ATrail* c = NULL;
        ANode*  n = NULL;
        int kind;
        while (st==A_OK && (kind = kit_next(&it,&c,&n))!=0) {
            st = (kind == 1) ? trail_abort(t, c) : node_abort(n);
        }
        node_trav(&t->node, -1);
        if (t->trs==NULL || tr->status==TR_OFF) {
            return st;
        }
    }

    if (tr->waiting) {
        // already leaving a block: goes on as an abort
        if (tr->x->wst != A_ABORT) {
            wait_set(tr, A_ABORT);
        }
    } else if (tr->status == TR_SUSPENDED) {
        AStatus st2 = trail_resume(t, tr, R_ABORT);
        if (st == A_OK) {
            st = st2;
        }
    } else if (tr->status==TR_RUNNING && !tr->aborted) {
        // unwinds when control returns to it
        tr->aborted = 1;
    }
    return st;
}

static AStatus node_abort (ANode* n) {
    if (n->dead) {
        return A_OK;
    }
    AStatus st = A_OK;
    AVal me = a_obj(&n->hdr);
    a_inc(me);
    if (n->hdr.type == A_XTASK) {
        ATask* t = (ATask*) n;
        if (t->trs != NULL) {
            st = trail_abort(t, &t->trs[0]);
        }
    } else {
        node_trav(n, 1);
        int cnt = n->ndns;
        for (int i=0; i<cnt && st==A_OK; i++) {
            st = node_abort(n->dns[i]);
        }
        node_trav(n, -1);
    }
    a_dec(me);
    return st;
}

AStatus a_abort (AVal v) {
    if (ctx_chk("abort") != A_OK) {
        return A_ERROR;
    }
    if (v.type!=A_XTASK && v.type!=A_TASKS) {
        return a_error("invalid abort : expected task");
    }
    return chk_aborted(node_abort((ANode*)v.u.o));
}

///////////////////////////////////////////////////////////////////////////////
// EMIT
///////////////////////////////////////////////////////////////////////////////

// Starts the filters of a node or trail that was toggled off.
static AStatus filt_start (APatt** fs, int n) {
    AStatus st = A_OK;
    for (int i=0; i<n && st==A_OK; i++) {
        int done = 0;
        st = patt_start(fs[i], &done);
        a_pay_clear();
    }
    return st;
}

// What is toggled off only reacts to events that match a filter,
// which then starts again.
static AStatus filt_pass (APatt** fs, int n, AVal evt, int* pass) {
    AStatus st = A_OK;
    *pass = 0;
    for (int i=0; i<n && st==A_OK; i++) {
        int done = 0;
        st = patt_match(fs[i], evt, &done);
        if (st==A_OK && done) {
            *pass = 1;
            patt_stop(fs[i]);
            st = patt_start(fs[i], &done);
        }
        a_pay_clear();
    }
    return st;
}

// An emit that a solo captures: while it traverses the subtree of the
// stage, it only follows the path down to the solo (the trails on
// the path do not awake), and then the whole subtree of the solo. The
// path is a copy: the solo may end in the middle of the emit. A nested
// emit has its own time, so it is not affected.
typedef struct ACap {
    uint32_t time;
    ATrail*  g;
    void**   path;
    int      n;
    int      in;
} ACap;
static ACap CAP = { 0, NULL, NULL, 0, 0 };

// Whether the emit at `time` is captured, outside the solo.
static inline int cap_out (uint32_t time) {
    return CAP.g!=NULL && CAP.time==time && !CAP.in;
}

// Whether the emit at `time` skips a child (a trail or a node). With no
// capture going on, it costs one load and one test.
static int cap_skip_ (uint32_t time, void* p);
static inline int cap_skip (uint32_t time, void* p) {
    return CAP.g!=NULL && cap_skip_(time, p);
}
static int cap_skip_ (uint32_t time, void* p) {
    if (!cap_out(time)) {
        return 0;
    }
    for (int i=0; i<CAP.n; i++) {
        if (CAP.path[i] == p) {
            return 0;
        }
    }
    return 1;
}

// The emit reaches a stage: the most recent of its solos whose
// patterns match the event, if the stage's match too, captures it.
// A stage or solo with no pattern takes every event.
static AStatus grab_pass (ATrailX* x, AVal evt, int* pass) {
    if (x->ngfilt == 0) {
        *pass = 1;
        return A_OK;
    }
    return filt_pass(x->gfilt, x->ngfilt, evt, pass);
}

static AStatus cap_find (ATrail* tr, AVal evt, uint32_t time) {
    ATrailX* x = tr->x;
    int pass = 0;
    AStatus st = grab_pass(x, evt, &pass);
    for (int k=x->gn-1; st==A_OK && pass && k>=0; k--) {
        ATrailX* g = x->gs[k]->x;
        int p = 0;
        st = grab_pass(g, evt, &p);
        if (st==A_OK && p) {
            CAP.time = time;
            CAP.g    = x->gs[k];
            CAP.n    = g->gpn;
            CAP.in   = 0;
            CAP.path = malloc((g->gpn > 0 ? g->gpn : 1) * sizeof(void*));
            memcpy(CAP.path, g->gpath, g->gpn * sizeof(void*));
            break;
        }
    }
    return st;
}

// The event is `:T [b]` for the tag of a `mute on`: `b` mutes.
static int toggle_evt (AVal tog, AVal evt, int* on) {
    if (tog.type==A_NIL || evt.type!=A_TAB) {
        return 0;
    }
    AVal tag = A_TAB_OF(evt)->tag;
    AVal b = a_tab_get(A_TAB_OF(evt), a_int(0));
    if (b.type!=A_BOOL || !a_gte(tog, tag)) {
        return 0;
    }
    *on = !b.u.b;
    return 1;
}

// The traversal of an emit is compiled twice, with C (capture) a
// constant: 0 when no solo is registered, so that it pays nothing for
// stage and solo, and 1 otherwise. An emit picks one when it starts.
#define A_INLINE static inline __attribute__((always_inline))
#define EMIT_TRAIL(C) ((C) ? emit_trail1 : emit_trail0)
#define EMIT_NODE(C)  ((C) ? emit_node1  : emit_node0)

// The children of a trail, in the order they started.
A_INLINE AStatus emit_kids (ATask* t, ATrail* tr, uint32_t time, AVal evt,
                            const int C) {
    AStatus st = A_OK;
    if (tr->nkids == 0) {
        // only nested trails, which keep their places
        const ATrailD* ds = t->desc->trails;
        for (int j=ds[tr->idx].kid; j>=0 && st==A_OK; j=ds[j].next) {
            if (C && cap_skip(time, &t->trs[j])) {
                continue;
            }
            st = EMIT_TRAIL(C)(t, &t->trs[j], time, evt);
            if (t->trs == NULL) {
                break;
            }
        }
        return st;
    }
    node_trav(&t->node, 1);
    KIt it;
    kit_init(&it, t, tr);
    ATrail* c = NULL;
    ANode*  n = NULL;
    int kind;
    while (st==A_OK && (kind = kit_next(&it,&c,&n))!=0) {
        if (C && cap_skip(time, (kind == 1) ? (void*)c : (void*)n)) {
            continue;
        }
        st = (kind == 1) ? EMIT_TRAIL(C)(t, c, time, evt)
                         : EMIT_NODE(C)(n, time, evt);
    }
    node_trav(&t->node, -1);
    return st;
}

// Traverses the subtree of a trail: its children in order, then the
// trail itself at its await.

// A solo and a stage change how the emit goes on below them (C only).
A_INLINE AStatus emit_trail_g (ATask* t, ATrail* tr, uint32_t time, AVal evt,
                               const int C) {
    if (tr->status==TR_OFF || tr->doomed) {
        return A_OK;
    }
    AStatus st = A_OK;
    ACap save;
    if (C) {
        save = CAP;
        if (cap_out(time) && tr==CAP.g) {
            CAP.in = 1;
        } else if (tr->x!=NULL && tr->x->gkind==A_GRABBABLE && tr->x->gn>0 &&
                   !cap_out(time)) {
            st = cap_find(tr, evt, time);
            if (st != A_OK) {
                goto out;
            }
        }
    }
    int pass = 1;
    if (tr->toggled) {
        st = filt_pass(tr->x->filt, tr->x->nfilt, evt, &pass);
    }

    if (st==A_OK && pass) {
        if (tr->nkids>0 || t->desc->trails[tr->idx].kid>=0) {
            st = emit_kids(t, tr, time, evt, C);
            if (t->trs == NULL) {
                goto out;
            }
        }
        if (st != A_OK) {
            // a child exited
            if (tr->waiting) {
                // leaving a block: replaces the pending exit
                wait_set(tr, st);
                st = A_OK;
            } else if (tr->status == TR_SUSPENDED) {
                // raised at the await of the parent
                EXIT = st;
                LAZY = 1;
                st = trail_resume(t, tr, R_EXIT);
            } else if (tr->status == TR_RUNNING) {
                // the parent is on the C stack
                exit_park(t, tr, st);
                st = A_OK;
            }
        } else if (tr->status==TR_SUSPENDED && tr->time<time && !tr->waiting &&
                   !(C && cap_out(time))) {
            int done = 0;
            if (tr->root.kind == PK_HEAP) {
                // predicates run in the context of the awaiting trail
                int depth = a_depth;
                ATask*  ptask  = a_cur;
                ATrail* ptrail = a_trl;
                a_cur   = t;
                a_trl   = tr;
                a_depth = tr->adepth;
                if (CTX_ON()) {
                    st = ctx_switch(t, tr->idx);
                }
                if (st == A_OK) {
                    st = patt_match(tr->root.u.p, evt, &done);
                }
                a_cur   = ptask;
                a_trl   = ptrail;
                a_depth = depth;
            } else {
                leaf_match(&tr->root, t, evt, &done);
            }
            if (st != A_OK) {
                EXIT = st;
                LAZY = 1;
                st = trail_resume(t, tr, R_EXIT);
            } else if (done) {
                LAZY = 1;
                st = trail_resume(t, tr, R_EVENT);
            }
        }
    }

    // `mute on`, after the trail has seen the event: a trail
    // toggled on receives a zero tick, which awakes `await(true)` and
    // lets its timers report again
    int on;
    if (st==A_OK && t->trs!=NULL && tr->x!=NULL && tr->status!=TR_OFF &&
        toggle_evt(tr->x->tog,evt,&on))
    {
        if (on && tr->toggled) {
            tr->toggled = 0;
            st = EMIT_TRAIL(C)(t, tr, ++TIME, a_int(0));
        } else if (!on && !tr->toggled) {
            tr->toggled = 1;
            st = filt_start(tr->x->filt, tr->x->nfilt);
        }
    }

out:
    if (C) {
        if (CAP.path != save.path) {
            free(CAP.path);
        }
        CAP = save;
    }
    return st;
}

// Traverses the subtree of a node.
A_INLINE AStatus emit_node_g (ANode* n, uint32_t time, AVal evt, const int C) {
    if (n->dead || n->doomed) {
        return A_OK;
    }
    AVal me = a_obj(&n->hdr);
    a_inc(me);

    AStatus st = A_OK;
    int pass = 1;
    if (n->toggled) {
        st = filt_pass(n->filt, n->nfilt, evt, &pass);
    }
    if (st==A_OK && pass) {
        if (n->hdr.type == A_XTASK) {
            ATask* t = (ATask*) n;
            if (t->trs != NULL) {
                st = EMIT_TRAIL(C)(t, &t->trs[0], time, evt);
            }
        } else {
            node_trav(n, 1);
            int cnt = n->ndns;
            for (int i=0; i<cnt && st==A_OK; i++) {
                if (C && cap_skip(time, n->dns[i])) {
                    continue;
                }
                st = EMIT_NODE(C)(n->dns[i], time, evt);
            }
            node_trav(n, -1);
        }
    }

    a_dec(me);
    return st;
}

static AStatus emit_trail0 (ATask* t, ATrail* tr, uint32_t time, AVal evt) {
    return emit_trail_g(t, tr, time, evt, 0);
}
static AStatus emit_trail1 (ATask* t, ATrail* tr, uint32_t time, AVal evt) {
    return emit_trail_g(t, tr, time, evt, 1);
}
static AStatus emit_node0 (ANode* n, uint32_t time, AVal evt) {
    return emit_node_g(n, time, evt, 0);
}
static AStatus emit_node1 (ANode* n, uint32_t time, AVal evt) {
    return emit_node_g(n, time, evt, 1);
}

// An emit starts the traversal with capture only if a solo exists.
static AStatus emit_node (ANode* n, uint32_t time, AVal evt) {
    return EMIT_NODE(NSOLO != 0)(n, time, evt);
}

AStatus a_toggle (AVal t, AVal mute, APatt** fs, int n) {
    AStatus st = ctx_chk("mute");
    ANode* x = (ANode*) t.u.o;
    // `mute t(true)` silences, `mute t(false)` gives the events back
    AVal on = a_bool(mute.type==A_BOOL && !mute.u.b);
    if (st != A_OK) {
        // inside enter or leave of a context
    } else if (t.type!=A_XTASK && t.type!=A_TASKS) {
        st = a_error("invalid mute : expected task");
    } else if (mute.type != A_BOOL) {
        st = a_error("invalid mute : expected bool argument");
    } else if (on.u.b && n>0) {
        st = a_error("invalid mute : unexpected pattern");
    } else if (on.u.b && !x->toggled) {
        st = a_error("invalid mute : expected muted task");
    } else if (!on.u.b && x->toggled) {
        st = a_error("invalid mute : expected unmuted task");
    }
    if (st != A_OK) {
        for (int i=0; i<n; i++) {
            patt_free(fs[i]);
        }
        return st;
    }

    if (on.u.b) {
        // a zero tick awakes `await(true)` and lets the timers report
        node_untoggle(x);
        uint32_t otime = EVT_TIME;
        AVal     oval  = EVT_VAL;
        EVT_TIME = ++TIME;
        EVT_VAL  = a_int(0);
        AStatus st2 = emit_node(x, EVT_TIME, EVT_VAL);
        EVT_TIME = otime;
        EVT_VAL  = oval;
        AStatus st3 = ctx_here();
        return chk_aborted((st2 == A_OK) ? st3 : st2);
    }
    x->filt = malloc((n > 0 ? n : 1) * sizeof(APatt*));
    memcpy(x->filt, fs, n * sizeof(APatt*));
    x->nfilt = n;
    x->toggled = 1;
    return filt_start(x->filt, x->nfilt);
}

void a_trail_toggle (ATask* t, int i, AVal tag, APatt** fs, int n) {
    ATrail* tr = &t->trs[i];
    if (tr->status == TR_OFF) {
        // the block has already terminated
        for (int j=0; j<n; j++) {
            patt_free(fs[j]);
        }
        return;
    }
    ATrailX* x = trail_x(tr);
    a_cpy(&x->tog, tag);
    x->filt = malloc((n > 0 ? n : 1) * sizeof(APatt*));
    memcpy(x->filt, fs, n * sizeof(APatt*));
    x->nfilt = n;
}

// `stage P {...}` and `solo P {...}`, before the block starts. A solo
// takes the nearest stage above it, in the tree of tasks.
AStatus a_trail_grab (ATask* t, int i, int kind, APatt** fs, int n) {
    ATrail* me = &t->trs[i];
    ATrailX* x = trail_x(me);
    x->gkind  = kind;
    x->gfilt  = malloc((n > 0 ? n : 1) * sizeof(APatt*));
    memcpy(x->gfilt, fs, n * sizeof(APatt*));
    x->ngfilt = n;
    AStatus st = filt_start(x->gfilt, x->ngfilt);
    if (st!=A_OK || kind==A_GRABBABLE) {
        return st;
    }
    int cap = 0;
    int k   = 0;
    void** path = NULL;
    ATrail* up = NULL;
    while (up == NULL) {
        ATrail* tr = &t->trs[i];
        if (tr!=me && tr->x!=NULL && tr->x->gkind==A_GRABBABLE) {
            up = tr;
            break;
        }
        if (k+2 > cap) {
            cap  = (cap == 0) ? 8 : cap*2;
            path = realloc(path, cap * sizeof(void*));
        }
        path[k++] = tr;
        if (i > 0) {
            i = t->desc->trails[i].up;
            continue;
        }
        ANode* nd = &t->node;
        do {
            if (k+1 > cap) {
                cap  = cap*2;
                path = realloc(path, cap * sizeof(void*));
            }
            path[k++] = nd;
            i  = nd->trail;
            nd = nd->up;
        } while (nd!=NULL && nd->hdr.type!=A_XTASK);
        if (nd == NULL) {
            free(path);
            return a_error("invalid solo : expected enclosing stage");
        }
        t = (ATask*) nd;
    }
    x->gpath = path;
    x->gpn   = k;
    x->gup   = up;
    ATrailX* u = up->x;
    u->gs = realloc(u->gs, (u->gn+1) * sizeof(ATrail*));
    u->gs[u->gn++] = me;
    NSOLO++;
    return A_OK;
}

// An emit traverses the tree, and then the code that emitted goes on in
// its own contexts.
static AStatus emit_top (ANode* n, AVal evt) {
    uint32_t otime = EVT_TIME;
    AVal     oval  = EVT_VAL;
    EVT_TIME = ++TIME;
    EVT_VAL  = evt;
    AStatus st = emit_node(n, EVT_TIME, evt);
    EVT_TIME = otime;
    EVT_VAL  = oval;
    AStatus st2 = ctx_here();
    return (st == A_OK) ? st2 : st;
}

// Target of an emit: :task (default), :parent, :global, a task or
// pool, or a number of levels up.
// `anc`: the target is the running task or an ancestor.
static AStatus emit_to (ANode** ret, AVal to, int* anc) {
    ANode* n = (a_cur != NULL) ? &a_cur->node : &ROOT.node;
    double lvl = 0;
    *anc = 0;

    if (to.type == A_NIL) {
        // default
    } else if (to.type==A_XTASK || to.type==A_TASKS) {
        *ret = (ANode*) to.u.o;
        return A_OK;
    } else if (a_isnum(to)) {
        lvl = a_dbl(to);
    } else if (to.type==A_STR && !strcmp(A_STR_OF(to)->s,"task")) {
        // default
    } else if (to.type==A_STR && !strcmp(A_STR_OF(to)->s,"parent")) {
        lvl = 1;
    } else if (to.type==A_STR && !strcmp(A_STR_OF(to)->s,"global")) {
        *ret = &ROOT.node;
        return A_OK;
    } else {
        return a_error("invalid emit : invalid target");
    }

    // pools count
    for (; lvl>0; lvl--) {
        n = n->up;
        if (n == NULL) {
            return a_error("invalid emit : invalid target");
        }
    }
    *ret = n;
    *anc = 1;
    return A_OK;
}

// A tick is split in pieces, each ending at the nearest timer, so
// timers expire in order within a single tick.
static AStatus emit_tick (double us) {
    AStatus st = A_OK;
    double late = CLK_LATE;
    do {
        double d = (CLK_MIN < us) ? CLK_MIN : us;
        us -= d;
        // the timers report again
        CLK_MIN  = INFINITY;
        CLK_LATE = us;
        st = emit_top(&ROOT.node, a_numi(d));
    } while (st==A_OK && us>0);
    CLK_LATE = late;
    return st;
}

// Depth of the outermost block of a trail.
static int trail_bdepth (ATask* t, int i) {
    return t->depth + t->desc->trails[i].level;
}

// A pinned event only reaches tasks that live inside its pin block.
//
// Who emits is inside that block, since it holds the value, and it can
// only name a target that is attached to a block around it, since a
// reference to a task or pool is confined to the block that owns it.
// So the two blocks are in the same chain of blocks, that of who
// emits, where depths compare: the target must be attached at the
// depth of the pin block or deeper.
//
// One more case is valid: the running task or an ancestor (a target
// given by `:task`, `:parent`, or a number of levels), with the value
// pinned in the outermost block of its body, which all of its trails
// are inside. And a value pinned in the outermost block of the
// program reaches all tasks.

// Depth of the block that ends a target.
static int emit_depth (ANode* n) {
    if (n->hdr.type==A_XTASK && ((ATask*)n)->pool!=NULL) {
        // a task in a pool follows the pool
        return emit_depth(&((ATask*)n)->pool->node);
    } else if (n->hdr.pinned) {
        return n->hdr.depth;
    } else if (n->hdr.type==A_XTASK && n->up!=NULL &&
               n->up->hdr.type==A_XTASK)
    {
        // an unpinned task ends with the outermost block of the trail
        // that spawned it
        return trail_bdepth((ATask*)n->up, n->trail);
    } else {
        return 0;
    }
}

static AStatus emit_chk (ANode* n, AVal evt, int anc) {
#ifdef ATM_PIN_UNCHECKED
    (void) n; (void) evt; (void) anc; return A_OK;
#endif
    int p = a_pdepth(evt);
    if (p == 0) {
        return A_OK;
    }
    if (n == &ROOT.node) {
        return (MAIN!=NULL && p<=MAIN->depth) ? A_OK : a_err_pinned(evt);
    }
    if (emit_depth(n) >= p) {
        return A_OK;
    }
    if (anc && n->hdr.type==A_XTASK && ((ATask*)n)->depth==p) {
        return A_OK;
    }
    return a_err_pinned(evt);
}

AStatus a_emit (AVal to, AVal evt) {
    ANode* n = NULL;
    int anc = 0;
    AStatus st = ctx_chk("emit");
    if (st != A_OK) {
        return st;
    }
    st = emit_to(&n, to, &anc);
    if (st != A_OK) {
        return st;
    }
    if (a_isnum(evt)) {
        // time passes for the whole program
        if (n != &ROOT.node) {
            return a_error("invalid emit : expected global");
        }
        return chk_aborted(emit_tick(a_dbl(evt)));
    }
    if (emit_chk(n,evt,anc) != A_OK) {
        return A_ERROR;
    }
    evt_own(evt);
    return chk_aborted(emit_top(n, evt));
}

double a_clock_next (void) {
    return (CLK_MIN < INFINITY) ? CLK_MIN : -1;
}

///////////////////////////////////////////////////////////////////////////////
// SPAWN
///////////////////////////////////////////////////////////////////////////////

int a_tasks_len (ATasks* ts) {
    int n = 0;
    for (int i=0; i<ts->node.ndns; i++) {
        n += !ts->node.dns[i]->dead;
    }
    return n;
}

ATask* a_tasks_next (ATasks* ts, int* pos) {
    while (*pos < ts->node.ndns) {
        ANode* n = ts->node.dns[(*pos)++];
        if (!n->dead) {
            return (ATask*) n;
        }
    }
    return NULL;
}

ATask* a_tasks_iter (ATasks* ts, int64_t* pos) {
    // the children are in the order of `seq`: first one from `pos`
    ANode** dns = ts->node.dns;
    int lo = 0;
    int hi = ts->node.ndns;
    while (lo < hi) {
        int mid = lo + (hi-lo)/2;
        if (dns[mid]->seq < *pos) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    for (; lo<ts->node.ndns; lo++) {
        if (!dns[lo]->dead) {
            *pos = (int64_t)dns[lo]->seq + 1;
            return (ATask*) dns[lo];
        }
    }
    *pos = (int64_t)ts->seq;
    return NULL;
}

AVal a_tasks_tab (AVal ts) {
    AVal t = a_tab();
    int pos = 0;
    ATask* x;
    while ((x = a_tasks_next(A_TASKS_OF(ts), &pos)) != NULL) {
        a_tab_set(A_TAB_OF(t), a_int(A_TAB_OF(t)->an), a_obj(&x->node.hdr));
    }
    return t;
}

// New task under `up`, with the arguments at `[base,a_top)`.
static ATask* task_new (AFunc* f, ANode* up, int base) {
    const AProto* p = f->task;
    ATask* t = calloc(1, sizeof(ATask));
    node_init(&t->node, A_XTASK);
    t->desc  = p;
    t->proto = f;
    a_inc(a_obj(&f->hdr));
    t->frame = calloc(1, sizeof(AFrame) + p->ntrails*sizeof(ATrail) +
        p->nslots*sizeof(AVal));
    t->frame->task = t;
    t->frame->n    = p->nslots;
    t->frame->nt   = p->ntrails;
    t->trs = t->frame->trs;
    t->L   = (AVal*) (t->trs + p->ntrails);
    for (int i=0; i<p->ntrails; i++) {
        t->trs[i].idx = i;
    }
    t->status = T_RUNNING;
    // a task is confined like its prototype, if it is a closure
    t->mindepth = a_pdepth(a_obj(&f->hdr));
    // the line of what confines it, for the message of an escape
    t->node.hdr.lin = f->hdr.lin;

    // parameters, then a table with the extra arguments
    int argc = a_top - base;
    for (int i=0; i<p->npar && i<argc; i++) {
        a_cpy(&t->L[i], a_stk[base+i]);
    }
    if (p->dots) {
        // the extra arguments wait in a table
        int n = (argc > p->npar) ? argc-p->npar : 0;
        t->L[p->npar] = a_tab();
        for (int i=0; i<n; i++) {
            a_pushc(a_stk[base+p->npar+i]);
        }
        a_args_pack(t->L[p->npar], n);
    }
    // a task that receives pinned arguments cannot leave their blocks
    for (int i=0; i<argc; i++) {
        int d = a_pdepth(a_stk[base+i]);
        if (d > t->mindepth) {
            t->mindepth = d;
            t->node.hdr.lin = a_stk[base+i].u.o->lin;
        }
    }
    a_settop(base);

    node_add(up, &t->node);
    return t;
}

// The task being spawned is pinned by its block from birth.
static int SPAWN_PIN = 0;

static int SPAWN_DEPTH = 0;

AStatus a_spawn_pin (AVal* dst, AVal proto, int base) {
    SPAWN_PIN = 1;
    AStatus st = a_spawn(dst, proto, a_nil(), base);
    SPAWN_PIN = 0;
    return st;
}

AStatus a_spawn_in (AVal* dst, AVal proto, int base, AVal h) {
    if (h.type==A_TAB && h.u.o->pinned) {
        SPAWN_PIN   = 2;
        SPAWN_DEPTH = h.u.o->depth;
    }
    AStatus st = a_spawn(dst, proto, a_nil(), base);
    SPAWN_PIN = 0;
    return st;
}

AStatus a_spawn (AVal* dst, AVal proto, AVal pool, int base) {
    if (ctx_chk("spawn") != A_OK) {
        a_settop(base);
        return A_ERROR;
    }
    if (proto.type != A_TASK) {
        a_settop(base);
        return a_error("invalid spawn : expected task prototype");
    }

    if (a_is_fin(proto)) {
        a_settop(base);
        return a_err_fin(proto);
    }
    ANode* up = (a_cur != NULL) ? &a_cur->node : &ROOT.node;
    if (pool.type == A_TASKS) {
        ATasks* ts = A_TASKS_OF(pool);
        // a pool that its block ended takes no more tasks
        if (ts->node.dead || pool.u.o->pinned==3) {
            a_settop(base);
            return a_err_fin(pool);
        }
        up = &ts->node;
        // the task lives as long as the pool: so must its prototype
        // and its arguments
        AObj* po = pool.u.o;
        if (po->pinned) {
            if (a_chk_store(proto, po->depth) != A_OK) {
                a_settop(base);
                return A_ERROR;
            }
            for (int i=base; i<a_top; i++) {
                if (a_chk_store(a_stk[i], po->depth) != A_OK) {
                    a_settop(base);
                    return A_ERROR;
                }
            }
        } else {
            // An unpinned pool takes its floor from what its tasks
            // hold, as a constructor from its fields, up to the block
            // of the names that hold it (its ceiling), where it is
            // checked.
            AVal x = proto;
            for (int i=base-1; i<a_top; i++) {
                if (i >= base) {
                    x = a_stk[i];
                }
                int p = a_pdepth(x);
                if (po->ceil!=0 && p>po->ceil) {
                    a_err_pinned(x);
                    a_settop(base);
                    return A_ERROR;
                }
                if (p > (int)po->depth) {
                    po->depth = p;
                    po->lin   = x.u.o->lin;
                }
            }
        }
        if (ts->max>=0 && a_tasks_len(ts)>=ts->max) {
            a_settop(base);
            a_clr(dst);
            return A_OK;
        }
    } else if (pool.type != A_NIL) {
        a_settop(base);
        return a_error("invalid spawn : expected tasks pool");
    }

    ATask* t = task_new(A_FUNC_OF(proto), up, base);
    if (a_trl!=NULL && t->mindepth>a_trl->conf) {
        a_trl->conf = t->mindepth;
    }
    // the body is one deeper than the block the task is attached to
    t->depth = a_depth + 1;
    if (SPAWN_PIN == 1) {
        // pinned from birth, in the block of the spawn
        t->node.hdr.pinned = 1;
        t->node.hdr.depth  = a_depth;
        t->node.hdr.lin    = a_lin;
    } else if (SPAWN_PIN == 2) {
        // pinned from birth with the table that will hold it, whose
        // block aborts it
        t->node.hdr.pinned = 2;
        t->node.hdr.depth  = SPAWN_DEPTH;
        t->node.hdr.lin    = a_lin;
        if (a_trl!=NULL && a_trl->conf<SPAWN_DEPTH) {
            a_trl->conf = SPAWN_DEPTH;
        }
    }
    SPAWN_PIN = 0;
    // a task in a pool is owned by the pool
    if (pool.type == A_TASKS) {
        t->node.hdr.pinned = 1;
        t->node.hdr.depth  = pool.u.o->depth;
        t->pool = A_TASKS_OF(pool);
        t->znext = t->pool->all;
        if (t->znext != NULL) {
            t->znext->zprev = t;
        }
        t->pool->all = t;
        if (pool.u.o->pinned) {
            t->depth = pool.u.o->depth + 1;
        }
    }
    a_cpy(dst, a_obj(&t->node.hdr));
    return chk_aborted(trail_resume(t, &t->trs[0], R_START));
}

AStatus a_trail_spawn (ATask* t, int i) {
    ATrail* tr = &t->trs[i];
    tr->rank = t->node.ndns;
    tr->conf = 0;
    tr->pc   = 0;
    return chk_aborted(trail_resume(t, tr, R_START));
}

AStatus a_tasks (AVal* dst, AVal max) {
    if (max.type!=A_NIL && !a_isnum(max)) {
        return a_error("invalid tasks limit : expected number");
    }
    ATasks* ts = calloc(1, sizeof(ATasks));
    node_init(&ts->node, A_TASKS);
    ts->max = a_isnum(max) ? (int)a_dbl(max) : -1;
    node_add((a_cur != NULL) ? &a_cur->node : &ROOT.node, &ts->node);
    a_cpy(dst, a_obj(&ts->node.hdr));
    return A_OK;
}

///////////////////////////////////////////////////////////////////////////////
// AWAIT
///////////////////////////////////////////////////////////////////////////////

// The await was satisfied at once, or failed: the results must
// survive the abort of the tasks that the pattern spawned.
static void root_drop (ARoot* r) {
    APay pay;
    a_pay_save(&pay);
    root_free(r);
    a_pay_restore(&pay);
}

AStatus a_await (ATrail* tr, APatt* p) {
    ARoot r;
    memset(&r, 0, sizeof(r));
    r.kind = PK_HEAP;
    r.u.p  = p;
    if (tr->aborted) {
        root_free(&r);
        return A_ABORT;
    }
    tr->time   = TIME;
    tr->adepth = a_depth;
    int done = 0;
    AStatus st = chk_aborted(patt_start(p, &done));
    if (st!=A_OK || done) {
        root_drop(&r);
        return st;
    }
    if (p->r.kind < PK_HEAP) {
        // a single leaf moves into the trail
        r = p->r;
        a_dec(p->args);
        free(p);
    }
    tr->root = r;
    return A_AWAIT;
}

// Awaits on a leaf.
static AStatus await_leaf (ATrail* tr, ARoot* r) {
    if (tr->aborted) {
        root_free(r);
        return A_ABORT;
    }
    tr->time   = TIME;
    tr->adepth = a_depth;
    int done = 0;
    AStatus st = chk_aborted(leaf_start(r, &done));
    if (st!=A_OK || done) {
        root_drop(r);
        return st;
    }
    tr->root = *r;
    return A_AWAIT;
}

AStatus a_await_val (ATrail* tr, AVal v) {
    ARoot r;
    memset(&r, 0, sizeof(r));
    r.kind = PK_VAL;
    r.v    = v;
    a_inc(v);
    return await_leaf(tr, &r);
}

AStatus a_await_cnt (ATrail* tr, AVal n, AVal v) {
    ARoot r;
    memset(&r, 0, sizeof(r));
    r.kind = PK_VAL;
    r.cnt  = cnt_of(n);
    r.v    = v;
    a_inc(v);
    return await_leaf(tr, &r);
}

AStatus a_await_trails (ATrail* tr, int lo, int n) {
    ARoot r;
    memset(&r, 0, sizeof(r));
    r.kind = PK_TRAILS;
    r.u.trs.lo = lo;
    r.u.trs.n  = n;
    return await_leaf(tr, &r);
}

AStatus a_resumed (ATrail* tr, AKind k) {
    // the payload (results or exit) must survive the abort of the
    // tasks the pattern spawned
    root_drop(&tr->root);
    switch (k) {
        case R_ABORT: return A_ABORT;
        case R_EXIT:
            // the error of an awaited task crosses this await
            if (EXIT == A_ERROR) {
                a_trace_await(a_lin);
            }
            return EXIT;
        default:      return A_OK;
    }
}

///////////////////////////////////////////////////////////////////////////////
// ME / PUB / PIN
///////////////////////////////////////////////////////////////////////////////

AVal a_me (void) {
    if (a_cur == NULL) {
        return a_nil();
    }
    AVal v = a_obj(&a_cur->node.hdr);
    a_inc(v);
    return v;
}

AStatus a_pub (AVal** ref) {
    if (a_cur == NULL) {
        return a_error("invalid pub : expected enclosing task");
    }
    *ref = &a_cur->pub;
    return A_OK;
}

AStatus a_pub_set (ATask* t, AVal v) {
    if (t == NULL) {
        t = a_cur;
        if (t == NULL) {
            return a_error("invalid pub : expected enclosing task");
        }
    }
    // the pub is visible wherever the task is
    int d = node_adepth(&t->node);
    if (a_chk_store(v, d) != A_OK) {
        return A_ERROR;
    }
    // an unpinned value is pinned with the task, if it is pinned
    ANode* own = (t->pool != NULL) ? &t->pool->node : &t->node;
    if (own->hdr.pinned) {
        if (!adopt_ok(v, d, 0)) {
            return adopt_err("pin");
        }
        pin_kid(v, d);
        adopt_end();
    } else if (own->hdr.ceil != 0) {
        a_ceil(v, own->hdr.ceil, own->hdr.clin);
    }
    AVal old = t->pub;
    a_inc(v);
    t->pub = v;
    if (old.type>=A_TAB && old.u.o->pinned==2 && (int)old.u.o->depth==d &&
        !(old.type==v.type && old.u.o==v.u.o))
    {
        a_retire(old, d, &t->node.hdr);
    } else {
        a_dec(old);
    }
    return A_OK;
}

// An unpinned value becomes pinned in the block `d`, along with the unpinned
// values it holds: by a declaration (`how` is 1) or by the value that
// holds it (2).
static void pin_deep (AVal v, int d, int how) {
#ifdef ATM_PIN_OFF
    // only a task or a pool is pinned, and not what it holds
    if (v.type!=A_XTASK && v.type!=A_TASKS) {
        return;
    }
#endif
    if (v.type<A_TAB || v.type==A_BOX || v.u.o->pinned) {
        return;
    }
    AObj* o = v.u.o;
    switch (v.type) {
        case A_TAB: {
            o->pinned = how;
            o->depth  = d;
            o->lin    = a_lin;
            uint32_t pos = 0;
            AVal k, x;
            while (a_tab_next(A_TAB_OF(v), &pos, &k, &x)) {
                pin_kid(k, d);
                pin_kid(x, d);
            }
            break;
        }
        case A_FUNC:
        case A_TASK: {
            o->pinned = how;
            o->depth  = d;
            o->lin    = a_lin;
            AFunc* f = A_FUNC_OF(v);
            for (int i=0; i<f->ncap; i++) {
                pin_kid(f->caps[i], d);
            }
            break;
        }
        case A_XTASK:
        case A_TASKS: {
            ANode* n = (ANode*) o;
            // the running task and its ancestors outlive the block
            if (is_up(n)) {
                return;
            }
            o->pinned = how;
            o->depth  = d;
            o->lin    = a_lin;
#ifndef ATM_PIN_OFF
            if (v.type == A_XTASK) {
                pin_kid(A_TASK_OF(v)->pub, d);
                pin_kid(A_TASK_OF(v)->ret, d);
            } else {
                for (ATask* t=A_TASKS_OF(v)->all; t!=NULL; t=t->znext) {
                    pin_kid(t->pub, d);
                    pin_kid(t->ret, d);
                }
            }
#endif
            // held by data: the block of the holder aborts it
            if (how==2 && n->up!=NULL && n->up->hdr.type==A_XTASK) {
                ATask* up = (ATask*) n->up;
                if (up->trs!=NULL && up->trs[n->trail].conf<d) {
                    up->trs[n->trail].conf = d;
                }
            }
            break;
        }
        case A_NAT: {
            o->pinned = how;
            o->depth  = d;
            o->lin    = a_lin;
            ANat* n = A_NAT_OF(v);
            for (int i=0; i<n->nholds; i++) {
                pin_kid(n->holds[i], d);
            }
            break;
        }
        default:
            o->pinned = how;
            o->depth  = d;
            o->lin    = a_lin;
            break;
    }
}

// A value that a pinned value holds, in the block `d`: an unpinned one
// with no name is pinned with it. One with a name is only referenced:
// a name already keeps it, and it may no longer be pinned inside `d`.
static void pin_kid (AVal v, int d) {
#ifdef ATM_PIN_OFF
    // a task or a pool held by data still ends with the block of the
    // holder; nothing else is pinned
    if (v.type==A_XTASK || v.type==A_TASKS) {
        pin_deep(v, d, 2);
    }
    return;
#endif
    if (v.type>=A_TAB && v.type!=A_BOX && !v.u.o->pinned &&
        v.u.o->ceil!=0 && d>(int)v.u.o->ceil)
    {
        // the holder is inside the block of the name
        return;
    }
    pin_deep(v, d, 2);
}

AStatus a_nat_holds (AVal nat, AVal v) {
    a_born(nat, v);
    if (a_chk_store(v, (int)nat.u.o->depth) != A_OK) {
        return A_ERROR;
    }
    if (v.type >= A_STR) {
        ANat* n = A_NAT_OF(nat);
        n->holds = realloc(n->holds, (n->nholds+1) * sizeof(AVal));
        n->holds[n->nholds++] = v;
        a_inc(v);
        if (v.type == A_NAT) {
            A_NAT_OF(v)->nheld++;
        }
    }
    if (nat.u.o->pinned) {
        if (!adopt_ok(v, nat.u.o->depth, 0)) {
            return adopt_err("pin");
        }
        pin_kid(v, nat.u.o->depth);
        adopt_end();
    }
    return A_OK;
}


void a_name_arg (AVal v, int d) {
#ifdef ATM_PIN_UNCHECKED
    (void) v; (void) d; return;
#endif
    if (v.type<A_TAB || v.u.o->pinned) {
        return;
    }
    if (a_cur!=NULL && v.u.o->ceil!=0) {
        int e = emit_depth(&a_cur->node);
        if (e < 1) {
            e = 1;
        }
        if (e < d) {
            d = e;
        }
    }
    a_name(v, d);
}

void a_ceil (AVal v, int d, uint32_t lin) {
    if (v.type<A_TAB || v.type==A_BOX || v.u.o->pinned) {
        return;
    }
    AObj* o = v.u.o;
    if (o->ceil!=0 && o->ceil<=d) {
        return;
    }
    o->ceil = (d < 0xFFFF) ? d : 0xFFFF;
    o->clin = lin;
    // Only the value itself: what it holds is protected by its count.
    // A value held by others than its holder (rc > 1) is not adopted
    // with it (`adopt_rec`, `pin_kid`), and one held only by it goes
    // with it.
}

// The names of the block `d` end: what only they held loses that
// ceiling, deep.
static void ceil_up (AVal v, int d, uint32_t lin) {
    (void) lin;
    if (v.type<A_TAB || v.type==A_BOX || v.u.o->pinned) {
        return;
    }
    AObj* o = v.u.o;
    if (o->ceil==0 || o->ceil<d) {
        return;
    }
    o->ceil = 0;
}

// The places written with `set :out` that wait for the end of a value.
// The entries of a value are chained, and found through a hash of the
// values: a value that ends (most have no entry) is looked up at once.
typedef struct OutE {
    AObj* v;        // not counted: the entry goes when the value ends
    AVal  t;        // a table and a key (counted), or
    AVal  k;
    AVal* slot;     // a variable, with its box (counted) if captured
    AVal  box;
    int   next;     // the next entry of the same value, or free (-1: none)
} OutE;

static OutE* OUTS   = NULL;
static int   OUTS_C = 0;
static int   OUTS_F = -1;   // free entries
int a_nouts = 0;            // entries in use

// value -> first entry (open addressing; empty: NULL)
typedef struct OutH {
    AObj* v;
    int   e;
} OutH;
static OutH* OUTH   = NULL;
static int   OUTH_C = 0;    // a power of 2
static int   OUTH_N = 0;    // keys in use, with tombstones
#define OUTH_DEL ((AObj*)1)

static uint32_t outh_hash (AObj* o) {
    uintptr_t x = (uintptr_t) o;
    x ^= x >> 17;
    x *= 0x9E3779B1u;
    return (uint32_t) (x ^ (x >> 15));
}

// The slot of `o`, or of an empty key where it would go.
static int outh_find (AObj* o) {
    uint32_t m = OUTH_C - 1;
    uint32_t i = outh_hash(o) & m;
    int del = -1;
    while (OUTH[i].v != NULL) {
        if (OUTH[i].v == o) {
            return i;
        }
        if (OUTH[i].v==OUTH_DEL && del<0) {
            del = i;
        }
        i = (i + 1) & m;
    }
    return (del >= 0) ? del : (int)i;
}

static void outh_grow (void) {
    OutH* old = OUTH;
    int   n   = OUTH_C;
    OUTH_C = (n == 0) ? 64 : n*2;
    OUTH   = calloc(OUTH_C, sizeof(OutH));
    OUTH_N = 0;
    for (int i=0; i<n; i++) {
        if (old[i].v!=NULL && old[i].v!=OUTH_DEL) {
            int j = outh_find(old[i].v);
            OUTH[j] = old[i];
            OUTH_N++;
        }
    }
    free(old);
}

static int out_ends (AVal v) {
    if (v.type < A_TAB) {
        return 0;
    }
    if (v.type==A_XTASK || v.type==A_TASKS) {
        return !((ANode*)v.u.o)->dead;
    }
    return v.u.o->pinned!=0 && v.u.o->pinned!=A_FIN;
}

static int out_same (OutE* x, OutE* e) {
    if (e->slot != NULL) {
        return x->slot == e->slot;
    }
    return x->slot==NULL && a_same(x->t, e->t) && a_same(x->k, e->k);
}

// Adds the entry, unless the value already has one for the same place.
// Returns 0 if it had (the counted fields of `e` are not taken).
static int out_add (OutE e) {
    if ((OUTH_N+1)*2 > OUTH_C) {
        outh_grow();
    }
    int h = outh_find(e.v);
    if (OUTH[h].v == e.v) {
        for (int i=OUTH[h].e; i>=0; i=OUTS[i].next) {
            if (out_same(&OUTS[i], &e)) {
                return 0;
            }
        }
    } else {
        if (OUTH[h].v == NULL) {
            OUTH_N++;
        }
        OUTH[h].v = e.v;
        OUTH[h].e = -1;
    }
    if (OUTS_F < 0) {
        int n = OUTS_C;
        OUTS_C = (n == 0) ? 16 : n*2;
        OUTS = realloc(OUTS, OUTS_C * sizeof(OutE));
        for (int i=OUTS_C-1; i>=n; i--) {
            OUTS[i].next = OUTS_F;
            OUTS_F = i;
        }
    }
    int i = OUTS_F;
    OUTS_F = OUTS[i].next;
    e.next = OUTH[h].e;
    OUTS[i] = e;
    OUTH[h].e = i;
    a_nouts++;
    return 1;
}

int a_out_reg_tab (AVal t, AVal k, AVal v) {
    if (!out_ends(v)) {
        return 0;
    }
    if (out_add((OutE) { v.u.o, t, k, NULL, a_nil(), -1 })) {
        a_inc(t);
        a_inc(k);
    }
    return 1;
}

int a_out_reg_var (AVal* slot, AVal box, AVal v, int d) {
    if (!out_ends(v) || d>a_pdepth(v)) {
        return 0;
    }
    if (out_add((OutE) { v.u.o, a_nil(), a_nil(), slot, box, -1 })) {
        a_inc(box);
    }
    return 1;
}

// set :out pub = v: the pub of the running task refers to `v`, with no
// check and no adoption, until `v` ends (registered: `*reg` false), or
// until the block of the `set` ends. `*task` gets the task, counted, the
// holder of the place.
AStatus a_out_pub (AVal* task, AVal v, AVal* reg) {
    ATask* t = a_cur;
    if (t == NULL) {
        return a_error("invalid pub : expected enclosing task");
    }
    a_cpy(task, a_obj(&t->node.hdr));
    AVal old = t->pub;
    a_inc(v);
    t->pub = v;
    int d = node_adepth(&t->node);
    if (old.type>=A_TAB && old.u.o->pinned==2 && (int)old.u.o->depth==d &&
        !(old.type==v.type && old.u.o==v.u.o))
    {
        a_retire(old, d, &t->node.hdr);
    } else {
        a_dec(old);
    }
    a_set(reg, a_bool(!a_out_reg_var(&t->pub, *task, v, d)));
    return A_OK;
}

void a_out_ended (AObj* o) {
    if (OUTH_C == 0) {
        return;
    }
    int h = outh_find(o);
    if (OUTH[h].v != o) {
        return;
    }
    // the chain leaves the index first: a release here may end other
    // values, and change the index
    int i = OUTH[h].e;
    OUTH[h].v = OUTH_DEL;
    AVal v = a_obj(o);
    while (i >= 0) {
        OutE e = OUTS[i];
        OUTS[i].next = OUTS_F;
        OUTS_F = i;
        a_nouts--;
        i = e.next;
        // the place goes back to nil, if it still refers to the value
        if (e.slot != NULL) {
            if (a_same(*e.slot, v)) {
                a_clr(e.slot);
            }
            a_dec(e.box);
        } else {
            if (e.t.type==A_TAB && !a_is_fin(e.t) &&
                a_same(a_tab_get(A_TAB_OF(e.t),e.k), v))
            {
                a_tab_set(A_TAB_OF(e.t), e.k, a_nil());
            }
            a_dec(e.t);
            a_dec(e.k);
        }
    }
}

void a_unname_x (AVal* vs, int n, int d) {
    for (int i=0; i<n; i++) {
        ceil_up(vs[i], d, 0);
    }
}

// An unpinned value, and the unpinned values it holds, may be adopted in the
// block `d` only if it is their ceiling or an outer block: the names
// that hold them must not see them finalized.
static AObj*  ADOPT_BAD = NULL;
static int    ADOPT_SHARED = 0;

// The values that a pin adopts, found by a visit. Each one gets the
// number of references that come to it from the others (`in`): a value
// that has more references than those is also held elsewhere (by a name,
// whose ceiling no longer reaches what that name holds, or by a table of
// another owner), and so it is only referenced, with all it holds. The
// value at the top may also have one reference from where it is going
// (the variable of the pin, or the temporary of a store).
typedef struct AdoptV {
    AObj*    o;
    uint32_t clin;      // saved: the index of the value meanwhile
    int      in;
    int      ext;
} AdoptV;

static AdoptV* ADOPT_VIS = NULL;
static int     ADOPT_N   = 0;
static int     ADOPT_CAP = 0;
// the values left marked (7) as not adopted, until `adopt_end`
static AObj**  ADOPT_EXT   = NULL;
static int     ADOPT_NEXT  = 0;
static int     ADOPT_CEXT  = 0;

#define ADOPT_SEEN 5
#define ADOPT_OUT  7

static int adopt_rec (AVal v, int d, int top);

static int adopt_data (AObj* o) {
    return o->type==A_TAB || o->type==A_FUNC || o->type==A_TASK;
}

// The values that `v` refers to, for `f`.
static void adopt_kids (AVal v, void (*f)(AVal)) {
    switch (v.type) {
        case A_TAB: {
            uint32_t pos = 0;
            AVal k, x;
            while (a_tab_next(A_TAB_OF(v), &pos, &k, &x)) {
                f(k);
                f(x);
            }
            break;
        }
        case A_FUNC:
        case A_TASK: {
            AFunc* fn = A_FUNC_OF(v);
            for (int i=0; i<fn->ncap; i++) {
                f(fn->caps[i]);
            }
            break;
        }
        case A_XTASK:
            f(A_TASK_OF(v)->pub);
            f(A_TASK_OF(v)->ret);
            break;
        case A_NAT: {
            ANat* n = A_NAT_OF(v);
            for (int i=0; i<n->nholds; i++) {
                f(n->holds[i]);
            }
            break;
        }
        default:
            break;
    }
}

static int*    ADOPT_STK   = NULL;
static int     ADOPT_NSTK  = 0;
static int     ADOPT_CSTK  = 0;

static void adopt_out (AVal v) {
    if (v.type<A_TAB || v.type==A_BOX || v.u.o->pinned!=ADOPT_SEEN) {
        return;
    }
    AdoptV* x = &ADOPT_VIS[v.u.o->clin];
    if (x->ext) {
        return;
    }
    x->ext = 1;
    if (ADOPT_NSTK == ADOPT_CSTK) {
        ADOPT_CSTK = (ADOPT_CSTK == 0) ? 16 : ADOPT_CSTK*2;
        ADOPT_STK = realloc(ADOPT_STK, ADOPT_CSTK * sizeof(int));
    }
    ADOPT_STK[ADOPT_NSTK++] = v.u.o->clin;
}

// `root`: the value itself is adopted, as by `pin x = v`, and it is an
// error if it is held elsewhere; else it is only held, and then it may
// be only referenced. Leaves the values that are not adopted marked:
// `adopt_end` unmarks them, after the pin.
static int adopt_ok (AVal v, int d, int root) {
    ADOPT_N = 0;
    int ok = adopt_rec(v, d, root);
    int n = ADOPT_N;
    if (ok && n>0) {
        // held elsewhere: more references than those from the others
        for (int i=0; i<n; i++) {
            AdoptV* x = &ADOPT_VIS[i];
            int own = (i == 0) ? 1 : 0;
            // a named value: its names count too, and its ceiling
            // decides (`adopt_rec`)
            if (adopt_data(x->o) && x->o->ceil==0 && x->o->rc-x->in-own>0) {
                if (i==0 && root) {
#ifndef ATM_PIN_UNCHECKED
                    ADOPT_BAD = x->o;
                    ADOPT_SHARED = 1;
                    ok = 0;
                    break;
#else
                    continue;
#endif
                }
                adopt_out(a_obj(x->o));
            }
        }
        // and what they hold, with them
        while (ok && ADOPT_NSTK>0) {
            int j = ADOPT_STK[--ADOPT_NSTK];
            adopt_kids(a_obj(ADOPT_VIS[j].o), adopt_out);
        }
        ADOPT_NSTK = 0;
    }
    ADOPT_NEXT = 0;
    for (int i=0; i<n; i++) {
        AdoptV* x = &ADOPT_VIS[i];
        x->o->clin = x->clin;
        if (ok && x->ext) {
            x->o->pinned = ADOPT_OUT;
            if (ADOPT_NEXT == ADOPT_CEXT) {
                ADOPT_CEXT = (ADOPT_CEXT == 0) ? 16 : ADOPT_CEXT*2;
                ADOPT_EXT = realloc(ADOPT_EXT, ADOPT_CEXT * sizeof(AObj*));
            }
            ADOPT_EXT[ADOPT_NEXT++] = x->o;
        } else {
            x->o->pinned = 0;
        }
    }
    return ok;
}

static void adopt_end (void) {
    for (int i=0; i<ADOPT_NEXT; i++) {
        if (ADOPT_EXT[i]->pinned == ADOPT_OUT) {
            ADOPT_EXT[i]->pinned = 0;
        }
    }
    ADOPT_NEXT = 0;
}

static int adopt_rec (AVal v, int d, int top) {
    if (v.type<A_TAB || v.type==A_BOX) {
        return 1;
    }
    AObj* o = v.u.o;
    // already visited (a cycle, or a value held twice)
    if (o->pinned == ADOPT_SEEN) {
        ADOPT_VIS[o->clin].in++;
        return 1;
    }
    // pinned
    if (o->pinned) {
        return 1;
    }
    // a named value that is held by a holder inside the block of its
    // name is only referenced
    if (!top && o->ceil!=0 && d>(int)o->ceil) {
        return 1;
    }
    if (o->ceil!=0 && d>o->ceil) {
        ADOPT_BAD = o;
        return 0;
    }
    if (ADOPT_N == ADOPT_CAP) {
        ADOPT_CAP = (ADOPT_CAP == 0) ? 16 : ADOPT_CAP*2;
        ADOPT_VIS = realloc(ADOPT_VIS, ADOPT_CAP * sizeof(AdoptV));
    }
    // the first one is the value that goes to its owner
    ADOPT_VIS[ADOPT_N] = (AdoptV) { o, o->clin, (ADOPT_N==0) ? 0 : 1, 0 };
    o->clin   = ADOPT_N++;
    o->pinned = ADOPT_SEEN;
    switch (v.type) {
        case A_TAB: {
            uint32_t pos = 0;
            AVal k, x;
            while (a_tab_next(A_TAB_OF(v), &pos, &k, &x)) {
                if (!adopt_rec(k,d,0) || !adopt_rec(x,d,0)) {
                    return 0;
                }
            }
            return 1;
        }
        case A_FUNC:
        case A_TASK: {
            AFunc* f = A_FUNC_OF(v);
            for (int i=0; i<f->ncap; i++) {
                if (!adopt_rec(f->caps[i], d, 0)) {
                    return 0;
                }
            }
            return 1;
        }
        case A_XTASK:
            return adopt_rec(A_TASK_OF(v)->pub, d, 0) &&
                   adopt_rec(A_TASK_OF(v)->ret, d, 0);
        case A_NAT: {
            ANat* n = A_NAT_OF(v);
            for (int i=0; i<n->nholds; i++) {
                if (!adopt_rec(n->holds[i], d, 0)) {
                    return 0;
                }
            }
            return 1;
        }
        default:
            return 1;
    }
}

// "invalid pin : table named at f:10"
static AStatus adopt_err (const char* what) {
    AObj* o = ADOPT_BAD;
    AVal v = a_obj(o);
    const char* type = (v.type == A_XTASK) ? "task" :
                       (v.type == A_TASKS) ? "pool" : a_typename(v);
    if (ADOPT_SHARED) {
        ADOPT_SHARED = 0;
        return a_error("invalid %s : shared %s", what, type);
    }
    uint32_t lin = o->clin;
    return a_error("invalid %s : %s named at %s:%d", what, type,
        atm_files[A_LIN_FILE(lin)], (int)A_LIN_LINE(lin));
}

AStatus a_hold (AObj* h, AVal v) {
#ifdef ATM_PIN_OFF
    (void) h; (void) v; return A_OK;
#endif
#ifndef ATM_PIN_UNCHECKED
    int p = a_pdepth(v);
    if (p > (int)h->depth) {
        // An unpinned holder that only its name refers to may receive a
        // value of an inner block: its floor rises there, up to its
        // ceiling. With another reference, a value that holds it took
        // the old floor at its birth, and could leave with it.
        if (!h->pinned && h->rc==1 && (h->ceil==0 || p<=(int)h->ceil)) {
            h->depth = p;
            h->lin   = v.u.o->lin;
        } else {
            return a_err_pinned(v);
        }
    }
#endif
    if (h->pinned) {
        if (!adopt_ok(v, h->depth, 0)) {
            return adopt_err("pin");
        }
        pin_kid(v, h->depth);
        adopt_end();
    } else if (h->ceil != 0) {
        // the names of the holder see it too
        a_ceil(v, h->ceil, h->clin);
    }
    return A_OK;
}

void a_func_scope (AVal f) {
    AFunc* fn = A_FUNC_OF(f);
    int d = 0;
    uint32_t lin = 0;
    for (int i=0; i<fn->ncap; i++) {
        int p = a_pdepth(fn->caps[i]);
        if (p > d) {
            d = p;
            lin = fn->caps[i].u.o->lin;
        }
    }
    // unpinned, with its floor at the deepest block of its captures
    fn->hdr.depth = d;
    fn->hdr.lin   = lin;
}

int a_pdepth (AVal v) {
    if (v.type < A_TAB) {
        return 0;
    }
    // the block of a pinned value, or the floor of an unpinned one
    int d = (int) v.u.o->depth;
    if (v.type == A_XTASK) {
        ATask* t = A_TASK_OF(v);
        // a terminated task holds nothing and needs no owner
        if (t->node.dead || t->status==T_DONE) {
            return 0;
        }
        // a task in a pool follows the pin of the pool
        if (t->pool != NULL) {
            d = node_adepth(&t->pool->node);
        } else if (!v.u.o->pinned) {
            // An unpinned task has its floor from birth: the outermost
            // block of the trail that spawned it, which aborts it at
            // its end if nobody pins it.
            ANode* up = t->node.up;
            d = 0;
            if (up!=NULL && up->hdr.type==A_XTASK) {
                d = trail_bdepth((ATask*)up, t->node.trail);
            }
        }
        if (t->mindepth > d) {
            d = t->mindepth;
        }
    }
    return d;
}

AStatus a_pin (AVal* slot) {
    AVal v = *slot;
    if (v.type < A_TAB) {
        return A_OK;
    }
    // taking the running task or an ancestor would abort upwards
    if ((v.type==A_XTASK || v.type==A_TASKS) && is_up((ANode*)v.u.o)) {
        return a_error("invalid assignment : unexpected parent task");
    }
    if (v.u.o->pinned) {
        return a_err_owned(v);
    }
    // an unpinned value is pinned at its floor or deeper, and at its
    // ceiling or outer
#ifndef ATM_PIN_UNCHECKED
    if (a_pdepth(v) > a_depth) {
        return a_err_pinned(v);
    }
#endif
#ifndef ATM_PIN_OFF
    if (!adopt_ok(v, a_depth, 1)) {
        return adopt_err("pin");
    }
#endif
    pin_deep(v, a_depth, 1);
    adopt_end();
    return A_OK;
}

// Transfer of `v` out of the block being left, to the block `d` that
// encloses it. What `v` holds from the block leaves with it: data
// (tables, closures with their captured variables, natives), tasks,
// and pools. What leaves becomes unpinned in the outer block, which may
// pin it again.
//
// A value belongs to the block being left if its depth is greater
// than `d`: it is pinned there, or it is unpinned with its floor there.
// It cannot be deeper, since the blocks inside have ended.

#define XFER UINT32_MAX     // depth of a value being transferred

// Something held by what leaves cannot leave: a task with arguments
// pinned in the block, or a task of a pool that stays.
static int XFER_OK;
static AVal XFER_BAD;

// Pools of the block with a task that leaves: they must leave too.
static ATasks** XFER_PS = NULL;
static int      XFER_NPS = 0;

static int xfer_mark (AVal v, int d);

static int xfer_mark_task (ATask* t, int d) {
    int m1 = xfer_mark(t->pub, d);
    int m2 = xfer_mark(t->ret, d);
    return (m1 > m2) ? m1 : m2;
}

// Marks what leaves. Returns the deepest block among the values that
// stay but are still held by what leaves.
static int xfer_mark (AVal v, int d) {
    if (v.type < A_TAB) {
        return 0;
    }
    AObj* o = v.u.o;
    if (o->depth == XFER) {
        return 0;
    }
    if (v.type == A_XTASK) {
        ATask* t = A_TASK_OF(v);
        int live = (!t->node.dead && t->status!=T_DONE);
        if (t->pool != NULL) {
            // it stays with its pool, which must not end before what
            // leaves
            AObj* p = &t->pool->node.hdr;
            if ((int)p->depth <= d) {
                return p->depth;
            }
            if (live && p->depth!=XFER) {
                XFER_PS = realloc(XFER_PS, (XFER_NPS+1) * sizeof(ATasks*));
                XFER_PS[XFER_NPS++] = t->pool;
            }
            return 0;
        }
        if (o->pinned && (int)o->depth<=d) {
            return o->depth;
        }
        // a pinned value never leaves its block
        if (live && o->pinned) {
            XFER_OK  = 0;
            XFER_BAD = v;
            return 0;
        }
        // unpinned, or terminated
        if (live && t->mindepth>d) {
            XFER_OK  = 0;
            XFER_BAD = v;
            return 0;
        }
        // it remains attached to the trail that spawned it
        int m = 0;
        ANode* up = t->node.up;
        if (live && up!=NULL && up->hdr.type==A_XTASK) {
            m = trail_bdepth((ATask*)up, t->node.trail);
            if (m > d) {
                m = 0;
            }
        }
        o->depth = XFER;
        int mt = xfer_mark_task(t, d);
        return (mt > m) ? mt : m;
    }
    if ((int)o->depth <= d) {
        // it stays: pinned outside, or unpinned with a floor outside
        return o->depth;
    }
    // a pinned value never leaves its block, except a captured
    // variable, with the closure that leaves
    if (o->pinned && v.type!=A_BOX) {
        XFER_OK  = 0;
        XFER_BAD = v;
        return 0;
    }
    o->depth = XFER;
    int m = 0;
    if (v.type == A_TASKS) {
        for (ATask* t=A_TASKS_OF(v)->all; t!=NULL; t=t->znext) {
            if (!t->node.dead && t->status!=T_DONE && t->mindepth>d) {
                XFER_OK  = 0;
                XFER_BAD = a_obj(&t->node.hdr);
            }
            int mt = xfer_mark_task(t, d);
            m = (mt > m) ? mt : m;
        }
    } else if (v.type == A_TAB) {
        uint32_t pos = 0;
        AVal k, x;
        while (a_tab_next(A_TAB_OF(v), &pos, &k, &x)) {
            int mk = xfer_mark(k, d);
            int mx = xfer_mark(x, d);
            m = (mk > m) ? mk : m;
            m = (mx > m) ? mx : m;
        }
    } else if (v.type==A_FUNC || v.type==A_TASK) {
        AFunc* f = A_FUNC_OF(v);
        for (int i=0; i<f->ncap; i++) {
            int mc = xfer_mark(f->caps[i], d);
            m = (mc > m) ? mc : m;
        }
    } else if (v.type == A_BOX) {
        // a captured variable leaves with its closure
        m = xfer_mark(A_BOX_OF(v)->v, d);
    } else if (v.type == A_NAT) {
        ANat* n = A_NAT_OF(v);
        for (int i=0; i<n->nholds; i++) {
            int mh = xfer_mark(n->holds[i], d);
            m = (mh > m) ? mh : m;
        }
    }
    return m;
}

// What was marked either leaves (`ok`), unpinned and with its floor at
// the block `m` of what it still holds, or goes back to the block.
static void xfer_set (AVal v, int ok, int m, int d) {
    if (v.type<A_TAB || v.u.o->depth!=XFER) {
        return;
    }
    AObj* o = v.u.o;
    if (v.type==A_XTASK || v.type==A_TASKS) {
        // the destination must pin it again
        if (ok) {
            o->pinned = 0;
            o->depth  = 0;
        } else {
            o->depth = o->pinned ? d+1 : 0;
        }
        if (v.type == A_XTASK) {
            xfer_set(A_TASK_OF(v)->pub, ok, m, d);
            xfer_set(A_TASK_OF(v)->ret, ok, m, d);
        } else {
            for (ATask* t=A_TASKS_OF(v)->all; t!=NULL; t=t->znext) {
                xfer_set(t->pub, ok, m, d);
                xfer_set(t->ret, ok, m, d);
            }
        }
        return;
    }
    if (ok) {
        // a captured variable keeps belonging to a block: the floor
        o->pinned = (v.type == A_BOX);
        o->depth  = m;
    } else {
        o->depth = d+1;
    }
    if (v.type == A_BOX) {
        xfer_set(A_BOX_OF(v)->v, ok, m, d);
    } else if (v.type == A_TAB) {
        uint32_t pos = 0;
        AVal k, x;
        while (a_tab_next(A_TAB_OF(v), &pos, &k, &x)) {
            xfer_set(k, ok, m, d);
            xfer_set(x, ok, m, d);
        }
    } else if (v.type==A_FUNC || v.type==A_TASK) {
        AFunc* f = A_FUNC_OF(v);
        for (int i=0; i<f->ncap; i++) {
            xfer_set(f->caps[i], ok, m, d);
        }
    } else if (v.type == A_NAT) {
        ANat* n = A_NAT_OF(v);
        for (int i=0; i<n->nholds; i++) {
            xfer_set(n->holds[i], ok, m, d);
        }
    }
}

// Aborts a task or pool at the end of its block. The abort runs
// defers: the pending exit is set aside meanwhile.
static AStatus blk_abort (ANode* n, AStatus st) {
    APay pay;
    a_pay_save(&pay);
    AStatus st2 = node_abort(n);
    if (st2 == A_OK) {
        a_pay_restore(&pay);
    } else {
        a_pay_drop(&pay);
        st = st2;
    }
    return st;
}

AStatus a_blk_xfer (AStatus st, AVal* vs, int n) {
    int d = a_depth - 1;
    if (st != A_OK) {
        vs = a_pay;
        n  = a_pay_n;
    }
    // all or nothing
    XFER_OK  = 1;
    XFER_BAD = a_nil();
    XFER_NPS = 0;
    int m = 0;
    for (int i=0; i<n; i++) {
        int mi = xfer_mark(vs[i], d);
        m = (mi > m) ? mi : m;
    }
    for (int i=0; i<XFER_NPS; i++) {
        if (XFER_PS[i]->node.hdr.depth != XFER) {
            XFER_OK  = 0;
            XFER_BAD = a_obj(&XFER_PS[i]->node.hdr);
        }
    }
    for (int i=0; i<n; i++) {
        xfer_set(vs[i], XFER_OK, m, d);
    }
    if (!XFER_OK) {
        st = a_err_pinned(XFER_BAD);
    }
    return st;
}

// DROP

int a_ndrop = 0;
static ANode** DROPS = NULL;
static int     CDROPS = 0;

// The tree has the only reference left to the node.
static int node_dropped (ANode* n) {
    if (n->up==NULL || n->dead) {
        return 0;
    }
    // a declaration owns it, or its pool does
    if (n->hdr.pinned==1 || n->hdr.pinned==3) {
        return 0;
    }
    return 1;
}

// RETIRE

// The values that left the place that owned them and are still alive:
// the list holds one reference to each. The key is the place: the slot
// of a `pin`, or the table or task whose field or `pub` it left. They
// are finalized when the place is: at the end of its block.
typedef struct {
    AObj*       o;
    int         d;
    const void* key;
    int         prev, next;     // the chain of the same key, or free
} ARet;
static ARet* RETS  = NULL;
static int   CRETS = 0;
static int   FRETS = -1;        // free entries
int a_nrets = 0;                // entries in use
#define NRETS a_nrets

// A map from a pointer to an entry (open addressing; empty: NULL): one
// by place (the first entry of its chain), one by value.
typedef struct { const void* p; int e; } PSlot;
typedef struct { PSlot* v; int c; int n; } PMap;
#define PMAP_DEL ((const void*)1)
static PMap RETS_K = { NULL, 0, 0 };
static PMap RETS_O = { NULL, 0, 0 };

static uint32_t pmap_hash (const void* p) {
    uintptr_t x = (uintptr_t) p;
    x ^= x >> 17;
    x *= 0x9E3779B1u;
    return (uint32_t) (x ^ (x >> 15));
}

static int pmap_slot (PMap* m, const void* p) {
    uint32_t k = m->c - 1;
    uint32_t i = pmap_hash(p) & k;
    int del = -1;
    while (m->v[i].p != NULL) {
        if (m->v[i].p == p) {
            return i;
        }
        if (m->v[i].p==PMAP_DEL && del<0) {
            del = i;
        }
        i = (i + 1) & k;
    }
    return (del >= 0) ? del : (int)i;
}

// The entry of `p`, or -1.
static int pmap_get (PMap* m, const void* p) {
    if (m->c == 0) {
        return -1;
    }
    int i = pmap_slot(m, p);
    return (m->v[i].p == p) ? m->v[i].e : -1;
}

static void pmap_put (PMap* m, const void* p, int e) {
    if ((m->n+1)*2 > m->c) {
        PSlot* old = m->v;
        int    n   = m->c;
        m->c = (n == 0) ? 64 : n*2;
        m->v = calloc(m->c, sizeof(PSlot));
        m->n = 0;
        for (int i=0; i<n; i++) {
            if (old[i].p!=NULL && old[i].p!=PMAP_DEL) {
                m->v[pmap_slot(m, old[i].p)] = old[i];
                m->n++;
            }
        }
        free(old);
    }
    int i = pmap_slot(m, p);
    if (m->v[i].p == NULL) {
        m->n++;
    }
    m->v[i].p = p;
    m->v[i].e = e;
}

static void pmap_del (PMap* m, const void* p) {
    if (m->c == 0) {
        return;
    }
    int i = pmap_slot(m, p);
    if (m->v[i].p == p) {
        m->v[i].p = PMAP_DEL;
    }
}

static void pmap_free (PMap* m) {
    free(m->v);
    m->v = NULL;
    m->c = 0;
    m->n = 0;
}

// Takes the entry `i` out of its chain and of the maps, and frees it.
static ARet ret_take (int i) {
    ARet r = RETS[i];
    if (r.prev >= 0) {
        RETS[r.prev].next = r.next;
    } else if (r.next >= 0) {
        pmap_put(&RETS_K, r.key, r.next);
    } else {
        pmap_del(&RETS_K, r.key);
    }
    if (r.next >= 0) {
        RETS[r.next].prev = r.prev;
    }
    pmap_del(&RETS_O, r.o);
    RETS[i].o    = NULL;
    RETS[i].next = FRETS;
    FRETS = i;
    NRETS--;
    return r;
}

static void ret_release (AObj* o);

static void ret_add (AObj* o, int d, const void* key) {
    if (FRETS < 0) {
        int n = CRETS;
        CRETS = (n == 0) ? 8 : n*2;
        RETS  = realloc(RETS, CRETS * sizeof(ARet));
        for (int i=CRETS-1; i>=n; i--) {
            RETS[i].o    = NULL;
            RETS[i].next = FRETS;
            FRETS = i;
        }
    }
    int i = FRETS;
    FRETS = RETS[i].next;
    int h = pmap_get(&RETS_K, key);
    RETS[i] = (ARet) { o, d, key, -1, h };
    if (h >= 0) {
        RETS[h].prev = i;
    }
    pmap_put(&RETS_K, key, i);
    pmap_put(&RETS_O, o, i);
    NRETS++;
}

void a_retire (AVal old, int d, const void* key) {
    if (old.type < A_TAB) {
        a_dec(old);
        return;
    }
    AObj* o = old.u.o;
    if (old.type==A_XTASK || old.type==A_TASKS) {
        // as a task held by pinned data: it is aborted when its last
        // other reference goes, or at the end of the block
        ANode* n = (ANode*) o;
        if (o->pinned != 3) {
            o->pinned = 2;
            o->depth  = d;
            if (n->up!=NULL && n->up->hdr.type==A_XTASK) {
                ATask* up = (ATask*) n->up;
                if (up->trs!=NULL && up->trs[n->trail].conf<d) {
                    up->trs[n->trail].conf = d;
                }
            }
        }
        a_dec(old);
        return;
    }
    if (o->pinned == 4) {
        // already retired: the list holds it
        a_dec(old);
        return;
    }
    o->pinned = 4;
    o->depth  = d;
    ret_add(o, d, key);
    // nobody else refers to it: at once
    if (o->rc == 1) {
        ret_release(o);
    }
}

// The values pinned with a retired one, and that only it refers to, are
// retired with it, so that a cycle among them is broken when the place
// ends at the latest. One that something else refers to (another holder
// of the block) is left alone.
static void ret_children (AVal v, int d, const void* key) {
    #define RET_KID(x) \
        if ((x).type>=A_TAB && (x).type!=A_XTASK && (x).type!=A_TASKS && \
            (x).u.o->pinned==2 && (int)(x).u.o->depth==d && \
            (x).u.o->rc==1) { \
            a_inc(x); a_retire(x, d, key); \
        }
    if (v.type == A_TAB) {
        uint32_t pos = 0;
        AVal k, x;
        while (a_tab_next(A_TAB_OF(v), &pos, &k, &x)) {
            RET_KID(k);
            RET_KID(x);
        }
    } else if (v.type==A_FUNC || v.type==A_TASK) {
        AFunc* f = A_FUNC_OF(v);
        for (int i=0; i<f->ncap; i++) {
            RET_KID(f->caps[i]);
        }
    } else if (v.type == A_NAT) {
        ANat* n = A_NAT_OF(v);
        for (int i=0; i<n->nholds; i++) {
            RET_KID(n->holds[i]);
        }
    }
    #undef RET_KID
}

static const void* ret_remove (AObj* o) {
    int i = pmap_get(&RETS_O, o);
    if (i < 0) {
        return NULL;
    }
    return ret_take(i).key;
}

// The list has the last reference: the value goes now.
static void ret_release (AObj* o) {
    AVal v = a_obj(o);
    int d = o->depth;
    const void* key = ret_remove(o);
    o->pinned = 0;
    o->depth  = 0;
    ret_children(v, d, key);
    a_dec(v);
}

static void ret_end_one (ARet r) {
    AVal v = a_obj(r.o);
    if (r.o->pinned==4 && (int)r.o->depth==r.d) {
        r.o->pinned = 2;
        clr_deep(v, r.d);
    }
    a_dec(v);
}

// The place `key` ends (NULL: all of them): what left it and is still
// alive, in practice a cycle, is finalized as a pin of its block.
void a_ret_end (const void* key) {
    if (key == NULL) {
        // all of them: the list may change at each one
        while (NRETS > 0) {
            for (int i=0; i<CRETS; i++) {
                if (RETS[i].o != NULL) {
                    ret_end_one(ret_take(i));
                    break;
                }
            }
        }
        return;
    }
    int i;
    while ((i = pmap_get(&RETS_K, key)) >= 0) {
        ret_end_one(ret_take(i));
    }
}

// The holder `o` is freed before its block ends: what left it and is
// still referred to is no longer owned by a place, and stays confined
// to its block by its floor.
void a_ret_forget (AObj* o) {
    int i;
    while ((i = pmap_get(&RETS_K, o)) >= 0) {
        ARet r = ret_take(i);
        if (r.o->pinned == 4) {
            r.o->pinned = 0;
        }
        a_dec(a_obj(r.o));
    }
}

AStatus a_pin_set (AVal* slot, AVal v, int d) {
    if (v.type >= A_TAB) {
        // the same value again
        if (slot->type==v.type && slot->u.o==v.u.o) {
            return A_OK;
        }
        if (v.u.o->pinned) {
            return a_err_owned(v);
        }
        if ((v.type==A_XTASK || v.type==A_TASKS) && is_up((ANode*)v.u.o)) {
            return a_error("invalid assignment : unexpected parent task");
        }
#ifndef ATM_PIN_UNCHECKED
        if (a_pdepth(v) > d) {
            return a_err_pinned(v);
        }
#endif
#ifndef ATM_PIN_OFF
        if (!adopt_ok(v, d, 1)) {
            return adopt_err("pin");
        }
#endif
        pin_deep(v, d, 1);
        adopt_end();
    }
    AVal old = *slot;
    a_inc(v);
    *slot = v;
    if (old.type>=A_TAB && old.u.o->pinned==1 && (int)old.u.o->depth==d) {
        a_retire(old, d, slot);
    } else {
        a_dec(old);
    }
    return A_OK;
}

void a_dropped (AObj* o) {
    if (o->type!=A_XTASK && o->type!=A_TASKS) {
        // a retired value: the list has the last reference
        ret_release(o);
        return;
    }
    ANode* n = (ANode*) o;
    if (n->dropq || !node_dropped(n)) {
        return;
    }
    if (a_ndrop == CDROPS) {
        CDROPS = (CDROPS == 0) ? 8 : CDROPS*2;
        DROPS  = realloc(DROPS, CDROPS * sizeof(ANode*));
    }
    n->dropq = 1;
    n->hdr.rc++;
    DROPS[a_ndrop++] = n;
}

AStatus a_drop_flush (AStatus st) {
    while (a_ndrop > 0) {
        ANode* n = DROPS[--a_ndrop];
        n->dropq = 0;
        // somebody may have taken it meanwhile
        if (n->hdr.rc==2 && node_dropped(n)) {
            st = blk_abort(n, st);
            if (n->hdr.type==A_TASKS && !n->dead) {
                node_died(n);
            }
            st = chk_aborted(st);
        }
        // the reference of the list: it does not count as a drop
        if (--n->hdr.rc == 0) {
            a_free(&n->hdr);
        }
    }
    return st;
}

static void a_drop_quit (void) {
    free(DROPS);
    DROPS  = NULL;
    CDROPS = 0;
    a_ndrop = 0;
}

AStatus a_chk_unpinned (AVal v) {
    if (v.type==A_XTASK || v.type==A_TASKS) {
        // the running task and its ancestors outlive the current
        // scope, and a terminated task needs no owner
        if (is_up((ANode*)v.u.o)) {
            return A_OK;
        }
        if (v.type==A_XTASK && A_TASK_OF(v)->status==T_DONE) {
            return A_OK;
        }
    }
    return a_error("invalid assignment : expected pinned value");
}

void a_own (AVal v) {
    pin_deep(v, 0, 1);
}

// An event that is emitted is shared by the tasks that receive it,
// which may keep it after the emit: no block can own it. An unpinned one
// becomes owned by the runtime, at its floor, and is released when
// nobody refers to it.
static void evt_own (AVal evt) {
    if (evt.type>=A_TAB && evt.type!=A_XTASK && evt.type!=A_TASKS) {
        pin_deep(evt, a_pdepth(evt), 1);
    }
}

// A value held by the one that is being released, and pinned with it.
static int clr_held (AVal x, uint32_t d) {
    return x.type>=A_TAB && x.u.o->pinned==2 && x.u.o->depth==d;
}

static void clr_deep (AVal v, uint32_t d);

// What a task that ended holds: its `pub` and its final value.
static void clr_task (ATask* t, uint32_t d) {
    if (NRETS > 0) {
        a_ret_end(&t->node.hdr);
    }
    if (clr_held(t->pub, d)) {
        clr_deep(t->pub, d);
    }
    if (clr_held(t->ret, d)) {
        clr_deep(t->ret, d);
    }
    a_clr(&t->pub);
    a_clr(&t->ret);
}

static void clr_deep (AVal v, uint32_t d) {
    AObj* o = v.u.o;
    if (NRETS > 0) {
        a_ret_end(o);
    }
    // each value once: a task or pool stays as finalized (3), not to be
    // pinned again, and so does data (A_FIN), which may no longer be
    // used by what still refers to it
    if (v.type==A_XTASK || v.type==A_TASKS) {
        o->pinned = 3;
    } else {
        o->pinned = A_FIN;
        o->depth  = 0;
    }
    if (a_nouts > 0) {
        a_out_ended(o);
    }
    switch (v.type) {
        case A_TAB: {
            uint32_t pos = 0;
            AVal k, x;
            while (a_tab_next(A_TAB_OF(v), &pos, &k, &x)) {
                if (clr_held(k, d)) {
                    clr_deep(k, d);
                }
                if (clr_held(x, d)) {
                    clr_deep(x, d);
                }
            }
            a_tab_clear(A_TAB_OF(v));
            break;
        }
        case A_FUNC:
        case A_TASK: {
            AFunc* f = A_FUNC_OF(v);
            for (int i=0; i<f->ncap; i++) {
                if (clr_held(f->caps[i], d)) {
                    clr_deep(f->caps[i], d);
                }
            }
            a_clrs(f->caps, f->ncap);
            break;
        }
        case A_NAT: {
            // what it holds is released after it
            ANat* n = A_NAT_OF(v);
            int   k = n->nholds;
            AVal* hs = malloc((k+1) * sizeof(AVal));
            for (int i=0; i<k; i++) {
                hs[i] = n->holds[i];
                a_inc(hs[i]);
            }
            a_nat_close(v);
            for (int i=0; i<k; i++) {
                if (clr_held(hs[i], d)) {
                    clr_deep(hs[i], d);
                }
                a_dec(hs[i]);
            }
            free(hs);
            break;
        }
        case A_XTASK: {
            // only after it ended
            ATask* t = A_TASK_OF(v);
            if (t->trs == NULL) {
                clr_task(t, d);
            }
            break;
        }
        case A_TASKS: {
            ATask* t = A_TASKS_OF(v)->all;
            while (t != NULL) {
                // the release may free the next ones: one at a time
                AVal ref = a_obj(&t->node.hdr);
                a_inc(ref);
                if (t->trs == NULL) {
                    clr_task(t, d);
                }
                ATask* nxt = (t->pool != NULL) ? t->znext : NULL;
                a_dec(ref);
                t = nxt;
            }
            break;
        }
    }
}

void a_pin_clr (AVal* slot) {
    if (NRETS > 0) {
        a_ret_end(slot);
    }
    AVal v = *slot;
    // only what is still pinned in this very block
    if (v.type<A_TAB || v.u.o->pinned!=1 || v.u.o->depth!=(uint32_t)a_depth) {
        return;
    }
    clr_deep(v, a_depth);
}

// A child exits to a trail that is running. The trail is below in the
// C stack, in a call that led to the exit: it takes the exit when
// control returns to it, and then leaves the block it is in. So what
// that block started stops at once, as when a block waits to end:
// suspended tasks no longer react, and running ones unwind when
// control returns to them, before it returns to the trail.
static void exit_park (ATask* t, ATrail* tr, AStatus st) {
    if (tr->pend) {
        // only the first exit counts
        a_pay_clear();
        return;
    }
    ATrailX* x = trail_x(tr);
    tr->pend = 1;
    x->pst = st;
    a_pay_save(&x->ppay);

    // depth of the block the trail is in: kept by the resume that its
    // call led to
    int depth = -1;
    for (RFrame* rf=RSTK; rf!=NULL; rf=rf->up) {
        if (rf->from == tr) {
            depth = rf->depth;
            break;
        }
    }
    if (depth < 0) {
        return;
    }
    const ATrailD* ds = t->desc->trails;
    for (int j=ds[tr->idx].kid; j>=0; j=ds[j].next) {
        ATrail* c = &t->trs[j];
        if (c->status!=TR_OFF && trail_bdepth(t,j)-1>=depth) {
            c->doomed = 1;
            trail_stop(t, c);
        }
    }
    for (int i=0; i<t->node.ndns && tr->nkids>0; i++) {
        ANode* n = t->node.dns[i];
        if (!n->dead && n->trail==tr->idx && node_adepth(n)>=depth) {
            n->doomed = 1;
            node_stop(n);
        }
    }
}

// A trail has not ended after its abort: it is running below in the C
// stack, or it waits for one that is.
static int trail_late (ATrail* tr) {
    return tr->status==TR_RUNNING || (tr->status!=TR_OFF && tr->waiting);
}

// A trail in a subtree that has not ended after the abort of the
// subtree.
static ATrail* late_of (ANode* n) {
    if (n->hdr.type == A_XTASK) {
        ATask* t = (ATask*) n;
        if (n->dead || t->trs==NULL) {
            return NULL;
        }
        return trail_late(&t->trs[0]) ? &t->trs[0] : NULL;
    }
    for (int i=0; i<n->ndns; i++) {
        ATrail* tr = late_of(n->dns[i]);
        if (tr != NULL) {
            return tr;
        }
    }
    return NULL;
}

// Marks the trails in a subtree that are running, to unwind when
// control returns to them.
static void trail_stop (ATask* t, ATrail* tr) {
    if (tr->status == TR_OFF) {
        return;
    }
    if (tr->status==TR_RUNNING && !tr->aborted) {
        tr->aborted = 1;
    }
    KIt it;
    kit_init(&it, t, tr);
    ATrail* c = NULL;
    ANode*  n = NULL;
    int kind;
    while ((kind = kit_next(&it,&c,&n)) != 0) {
        if (kind == 1) {
            trail_stop(t, c);
        } else {
            node_stop(n);
        }
    }
}

static void node_stop (ANode* n) {
    if (n->dead) {
        return;
    }
    if (n->hdr.type == A_XTASK) {
        ATask* t = (ATask*) n;
        if (t->trs != NULL) {
            trail_stop(t, &t->trs[0]);
        }
        return;
    }
    for (int i=0; i<n->ndns; i++) {
        node_stop(n->dns[i]);
    }
}

void a_pin_doom (AVal* slot) {
    AVal v = *slot;
    if ((v.type!=A_XTASK && v.type!=A_TASKS) ||
        !v.u.o->pinned || v.u.o->depth!=(uint32_t)a_depth)
    {
        return;
    }
    ANode* n = (ANode*) v.u.o;
    n->doomed = 1;
    node_stop(n);
}

void a_trail_doom (ATask* t, int i) {
    ATrail* tr = &t->trs[i];
    if (tr->status == TR_OFF) {
        return;
    }
    tr->doomed = 1;
    trail_stop(t, tr);
}

// The trail suspends the exit of a block until `late` ends. It keeps
// the pending exit meanwhile.
static void wait_for (ATrail* me, ATrail* late, AStatus st, int base) {
    trail_x(late)->waiter = me;
    ATrailX* x = trail_x(me);
    if (me->pend) {
        // an exit of a child arrived meanwhile: it replaces this one
        me->pend = 0;
        a_pay_clear();
        a_pay_restore(&x->ppay);
        x->ppay.v   = NULL;
        x->ppay.n   = 0;
        x->ppay.cap = 0;
        st = x->pst;
    }
    me->waiting = 1;
    x->wst = st;
    a_pay_save(&x->wpay);
    x->wn   = a_top - base;
    x->wstk = malloc((x->wn + 1) * sizeof(AVal));
    memcpy(x->wstk, a_stk+base, x->wn * sizeof(AVal));
    a_top = base;
}

int a_conf_end (ATrail* me, AStatus* st, int base) {
    ATask*  t  = a_cur;
    ATrail* tr = a_trl;
    if (t==NULL || tr->conf<a_depth) {
        return 0;
    }
    int any = 0;
    for (int i=0; i<t->node.ndns; i++) {
        ANode* c = t->node.dns[i];
        if (c->dead || c->trail!=tr->idx || c->hdr.pinned==1) {
            continue;
        }
        if (c->hdr.pinned == 2) {
            // held by data that is pinned in this block
            if (c->hdr.depth != (uint32_t)a_depth) {
                continue;
            }
        } else {
            // unpinned, and holding values pinned in this block
            if (c->hdr.type != A_XTASK) {
                continue;
            }
            ATask* x = (ATask*) c;
            if (x->status==T_DONE || x->mindepth<a_depth) {
                continue;
            }
        }
        // the node may leave the tree when it dies
        AVal ref = a_obj(&c->hdr);
        a_inc(ref);
        *st = blk_abort(c, *st);
        any = 1;
        ATrail* late = late_of(c);
        if (late==NULL && c->hdr.type==A_TASKS && !c->dead) {
            node_died(c);
        }
        int dead = c->dead;
        a_dec(ref);
        if (late != NULL) {
            // running below in the C stack: the block waits for it,
            // if it can; otherwise it unwinds when control returns
            if (me != NULL) {
                wait_for(me, late, *st, base);
                return 1;
            }
        } else if (dead) {
            // the abort may rearrange the tree: scan again
            i = -1;
        }
    }
    tr->conf = a_depth - 1;
    if (any) {
        *st = chk_aborted(*st);
    }
    return 0;
}

int a_orph_end (ATrail* me, AStatus* st, int base) {
    ATask* t = A_TRAIL_TASK(me);
    for (int i=0; i<t->node.ndns; i++) {
        ANode* n = t->node.dns[i];
        if (n->dead || n->trail!=me->idx) {
            continue;
        }
        // the node may leave the tree when it dies
        AVal ref = a_obj(&n->hdr);
        a_inc(ref);
        *st = blk_abort(n, *st);
        ATrail* late = late_of(n);
        if (late != NULL) {
            a_dec(ref);
            wait_for(me, late, *st, base);
            return 1;
        }
        if (n->hdr.type==A_TASKS && !n->dead) {
            node_died(n);
        }
        int dead = n->dead;
        a_dec(ref);
        if (dead) {
            // the abort may rearrange the tree: scan again
            i = -1;
        }
    }
    *st = chk_aborted(*st);
    return 0;
}

void a_orph_doom (ATrail* me) {
    ATask* t = A_TRAIL_TASK(me);
    for (int i=0; i<t->node.ndns; i++) {
        ANode* n = t->node.dns[i];
        if (!n->dead && !n->hdr.pinned && n->trail==me->idx) {
            n->doomed = 1;
            node_stop(n);
        }
    }
}

int a_pin_end (ATrail* me, AVal* slot, AStatus* st, int base) {
    AVal v = *slot;
    // only what is still pinned in this very block
    if (v.type<A_STR || !v.u.o->pinned || v.u.o->depth!=(uint32_t)a_depth) {
        return 0;
    }

    // data is released later, by `a_pin_clr`
    if (v.type!=A_XTASK && v.type!=A_TASKS) {
        return 0;
    }
    ANode* n = (ANode*) v.u.o;
    *st = blk_abort(n, *st);

    // the block waits for what could not be aborted yet
    ATrail* late = (me != NULL) ? late_of(n) : NULL;
    if (late != NULL) {
        wait_for(me, late, *st, base);
        return 1;
    }

    // a pool leaves the tree with its block
    if (n->hdr.type==A_TASKS && !n->dead) {
        node_died(n);
    }
    *st = chk_aborted(*st);
    return 0;
}

int a_trail_end (ATrail* me, int i, AStatus* st, int base) {
    ATask*  t  = A_TRAIL_TASK(me);
    ATrail* tr = &t->trs[i];
    if (tr->status == TR_OFF) {
        return 0;
    }
    // the abort runs defers: the pending exit is set aside meanwhile
    APay pay;
    a_pay_save(&pay);
    AStatus st2 = trail_abort(t, tr);
    if (st2 == A_OK) {
        a_pay_restore(&pay);
    } else {
        a_pay_drop(&pay);
        *st = st2;
    }
    // the block waits for what could not be aborted yet
    if (trail_late(tr)) {
        wait_for(me, tr, *st, base);
        return 1;
    }
    *st = chk_aborted(*st);
    return 0;
}

AStatus a_pin_resumed (ATrail* tr) {
    ATrailX* x = tr->x;
    tr->waiting = 0;
    for (int i=0; i<x->wn; i++) {
        a_push(x->wstk[i]);
    }
    free(x->wstk);
    x->wstk = NULL;
    x->wn   = 0;
    a_pay_restore(&x->wpay);
    x->wpay.v   = NULL;
    x->wpay.n   = 0;
    x->wpay.cap = 0;
    return x->wst;
}

///////////////////////////////////////////////////////////////////////////////
// MAIN
///////////////////////////////////////////////////////////////////////////////

AStatus a_main (const AProto* p) {
    node_init(&ROOT.node, A_TASKS);
    ROOT.max = -1;
    TIME = 1;
    CLK_MIN  = INFINITY;
    CLK_LATE = 0;

    // the program arguments are the `...` of the main chunk
    AVal proto = a_task(p, 0);
    int base = a_top;
    for (int i=1; i<ARGC; i++) {
        a_push(a_strz(ARGV[i]));
    }
    MAIN = task_new(A_FUNC_OF(proto), &ROOT.node, base);
    MAIN->depth = 1;
    // owned by the runtime, outside all blocks
    MAIN->node.hdr.pinned = 1;
    a_inc(a_obj(&MAIN->node.hdr));
    a_dec(proto);
    return trail_resume(MAIN, &MAIN->trs[0], R_START);
}

///////////////////////////////////////////////////////////////////////////////
// DRIVER
///////////////////////////////////////////////////////////////////////////////

void a_task_init (int argc, char** argv) {
    ARGC = argc;
    ARGV = argv;
    MAIN = NULL;
    a_cur = NULL;
    a_trl = NULL;
}

void a_task_quit (void) {
    a_drop_flush(A_OK);
    if (MAIN != NULL) {
        // what it holds may refer to it
        clr_task(MAIN, 0);
        a_ret_end(NULL);
        a_dec(a_obj(&MAIN->node.hdr));
        MAIN = NULL;
        node_sweep(&ROOT.node);
        free(ROOT.node.dns);
        ROOT.node.dns = NULL;
    }
    a_drop_flush(A_OK);
    a_drop_quit();
    free(ADOPT_VIS);
    ADOPT_VIS = NULL;
    ADOPT_CAP = 0;
    free(RETS);
    RETS  = NULL;
    FRETS = -1;
    pmap_free(&RETS_K);
    pmap_free(&RETS_O);
    a_nrets = 0;
    CRETS = 0;
}

int atm_alive (void) {
    return MAIN!=NULL && MAIN->status!=T_DONE;
}

AStatus atm_stop (void) {
    if (!atm_alive()) {
        return A_OK;
    }
    return node_abort(&MAIN->node);
}

AStatus atm_emit (AVal evt) {
    if (atm_step_failed()) {
        return A_ERROR;
    }
    if (a_isnum(evt)) {
        return atm_step_chk(emit_tick(a_dbl(evt)));
    }
    evt_own(evt);
    return atm_step_chk(emit_top(&ROOT.node, evt));
}

// The task that holds a node, beyond its pool, and the trail of that
// task which spawned it. NULL for the main chunk.
static ATask* node_holder (ANode* n, int* i) {
    ANode* x = n;
    do {
        *i = x->trail;
        x  = x->up;
    } while (x!=NULL && x->hdr.type!=A_XTASK);
    return (ATask*) x;
}

static AStatus emit_to1 (AVal to, AVal evt);

AStatus atm_emit_to (AVal to, AVal evt) {
    if (atm_step_failed()) {
        return A_ERROR;
    }
    return atm_step_chk(emit_to1(to, evt));
}

static AStatus emit_to1 (AVal to, AVal evt) {
    if (to.type!=A_XTASK && to.type!=A_TASKS) {
        return a_error("invalid emit : expected task");
    }
    if (a_isnum(evt)) {
        return a_error("invalid emit : expected global");
    }
    ANode* n = (ANode*) to.u.o;
    evt_own(evt);

    // Who holds the target, and who holds them, up to the main chunk,
    // each with the trail that spawned the one below. The chain is
    // taken before the emit: a task that ends leaves the tree, and the
    // emit may end any of them (the target ends, which awakes its
    // parent, which raises).
    typedef struct { ATask* t; int i; } Hold;
    Hold  fix[8];           // deeper trees go to the heap
    int   nh  = 0;
    int   cap = 8;
    Hold* hs  = fix;
    for (ANode* x=n; ; ) {
        int i = 0;
        ATask* t = node_holder(x, &i);
        if (t == NULL) {
            break;
        }
        if (nh == cap) {
            cap *= 2;
            if (hs == fix) {
                hs = malloc(cap * sizeof(Hold));
                memcpy(hs, fix, sizeof(fix));
            } else {
                hs = realloc(hs, cap * sizeof(Hold));
            }
        }
        hs[nh].t = t;
        hs[nh].i = i;
        nh++;
        a_inc(a_obj(&t->node.hdr));
        x = &t->node;
    }

    a_inc(to);
    AStatus st = emit_top(n, evt);
    a_dec(to);

    // An exit that leaves the target did not pass through who holds
    // it, since the emit started at the target. It is delivered as a
    // broadcast does: raised in the trail that spawned the target, and
    // upwards while nobody handles it. A holder that the emit ended is
    // skipped.
    for (int k=0; k<nh && st!=A_OK; k++) {
        ATask* t = hs[k].t;
        int    i = hs[k].i;
        while (t->trs != NULL) {
            ATrail* tr = &t->trs[i];
            if (tr->waiting) {
                // leaving a block: replaces the pending exit
                wait_set(tr, st);
                st = A_OK;
            } else if (tr->status == TR_SUSPENDED) {
                // raised at its await
                EXIT = st;
                st = trail_resume(t, tr, R_EXIT);
            } else if (tr->status == TR_RUNNING) {
                // it is on the C stack
                exit_park(t, tr, st);
                st = A_OK;
            }
            if (st==A_OK || t->trs==NULL || i==0) {
                break;
            }
            i = t->desc->trails[i].up;
        }
    }
    for (int k=0; k<nh; k++) {
        a_dec(a_obj(&hs[k].t->node.hdr));
    }
    if (hs != fix) {
        free(hs);
    }
    return st;
}

AStatus atm_step (uint32_t ms) {
    return emit_tick(ms * 1000.0);
}

double atm_next (void) {
    return a_clock_next();
}
