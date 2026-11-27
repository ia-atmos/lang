// Atmos compiler: C code generation.
//
// A construct is compiled against a destination: nowhere (effects
// only), a slot (one value), or the value stack (all its values, with
// the count in a C variable).
//
// A simple expression is compiled to an operand: either a borrowed C
// expression (constant, immutable variable), or an owned temporary
// slot `L[n]`. A temporary is released or moved by whoever consumes
// it.
//
// Invariant: when an expression starts to evaluate, its function has
// no pending values on the stack. Arguments are evaluated into slots
// and only then pushed, so an exit can always unwind to `base`.
//
// Failures and non-local exits set `st` and jump to the cleanup label
// of the enclosing block, which releases its slots and propagates the
// status. The values of an exit travel in the payload.

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atmc.h"

///////////////////////////////////////////////////////////////////////////////
// OUTPUT
///////////////////////////////////////////////////////////////////////////////

typedef struct Buf {
    char*  s;
    size_t n;
    size_t cap;
} Buf;

static void buf_add (Buf* b, const char* s) {
    size_t n = strlen(s);
    if (b->n + n + 1 > b->cap) {
        b->cap = (b->n + n + 1) * 2;
        b->s = realloc(b->s, b->cap);
    }
    memcpy(b->s + b->n, s, n + 1);
    b->n += n;
}

// One C function being generated.
typedef struct Unit {
    Buf body;
    // function being compiled, and its number of parameters
    Proto* proto;
    int npar;
    // task: resumable, with the slots in its frame
    int task;
    // transparent block: shares the slots of the unit of its task
    struct Unit* root;
    // trail of the unit: 0 for the body of the task
    int trail;
    // unit of a task: its trails, with their C functions
    struct Trail* trs;
    int ntrs;
    // number of resumption points
    int npcs;
    // number of slots in `L`
    int nslots;
    // the slots that are variables (and not temporaries), for the end
    // of their block
    char* isname;
    int   cname;
    // number of count variables `n<i>`
    int ncnts;
    // number of cleanup labels
    int nlabels;
    // cleanup label of the enclosing block
    int cleanup;
    // extra indentation of the generated lines
    int ind;
    // source line of the statement being compiled
    int lin;
    // nesting level of the enclosing block: the body is level 0, at
    // the depth `D` of the unit
    int level;
    // finalizers of the enclosing block
    struct Defer* defs;
    int ndefs;
    // constructor that initializes a `pin`: pinned before it is filled
    Expr* born;
    // table that will hold the spawn in `born`, if it goes to a field
    const char* bornh;
    // inside the body of a defer, which never suspends
    int indefer;
    // the value being assigned is known to be no object (#32): no test
    // of a name or of a store
    int scalar;
    // the block being generated declares a captured variable
    int box;
    // the next block is the outermost of a task: it ends its orphans
    int orph;
} Unit;

// Static description of a trail: its C function, parent, and the
// nesting level of its outermost block.
typedef struct Trail {
    const char* fn;
    int up;
    int level;
} Trail;

// Finalizer of a block: a defer (flag slot and body), a pin (the
// storage of the pinned task or pool), or a trail that the block
// started.
typedef struct Defer {
    int   flag;
    Expr* e;
    const char* pin;
    int   trail;
    // pin written by the user: its value may leave the block
    int   user;
    // the tasks of the unit that nobody pinned: they end with its
    // outermost block, after everything else
    int   orph;
    // set :out: the place goes back to nil, if it still refers to the
    // value. 1: a table (slots `ot`, `ok`); 2: a variable (`odst`).
    // The value is in the slot `ov`.
    // 3: `enter`/`leave` of the context in trail `ot`: it ends with the
    // defers of the block, before its pins are finalized
    int   out;
    int   ot, ok, ov;
    const char* odst;
} Defer;

// Appends one line at column zero (labels).
static void out0 (Unit* u, const char* fmt, ...) {
    char line[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    buf_add(&u->body, line);
    buf_add(&u->body, "\n");
}

// Appends one line to the body of the unit.
static void out (Unit* u, const char* fmt, ...) {
    char line[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    for (int i=0; i<=u->ind; i++) {
        buf_add(&u->body, "    ");
    }
    buf_add(&u->body, line);
    buf_add(&u->body, "\n");
}

// Emits a runtime call that may fail.
static void out_try (Unit* u, const char* fmt, ...) {
    char call[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(call, sizeof(call), fmt, ap);
    va_end(ap);
    out(u, "if ((st = %s) != A_OK) goto CL_%d;", call, u->cleanup);
}

static char* quote (const char* s, int len);

// A line with its file, as the runtime keeps it in `a_lin`.
static int lin_of (Tk tk) {
    return (tk.file << 20) | tk.lin;
}

// Quoted name of the file of a token.
static const char* file_q (Tk tk) {
    const char* f = FILES[tk.file];
    return quote(f, strlen(f));
}

// Slot counter of a unit.
static int* nslots (Unit* u) {
    return (u->root != NULL) ? &u->root->nslots : &u->nslots;
}

static int slot_new (Unit* u) {
    return (*nslots(u))++;
}

static void slot_name (Unit* u, int s) {
    Unit* o = (u->root != NULL) ? u->root : u;
    if (s >= o->cname) {
        int n = (s+1 > 2*o->cname) ? s+1 : 2*o->cname;
        o->isname = realloc(o->isname, n);
        memset(o->isname + o->cname, 0, n - o->cname);
        o->cname = n;
    }
    o->isname[s] = 1;
}

static int slot_isname (Unit* u, int s) {
    Unit* o = (u->root != NULL) ? u->root : u;
    return s < o->cname && o->isname[s];
}

///////////////////////////////////////////////////////////////////////////////
// CONSTANTS
///////////////////////////////////////////////////////////////////////////////

// String constants of the program, created once at startup as
// `atm_K[i]`. Every name that the compiler declares at file scope has
// the prefix `atm_`, away from the names of a `:prelude`.

typedef struct Const {
    const char* s;
    int         len;
} Const;

// What the coder collects for the C unit of each module: constants,
// declarations and definitions of functions, and what it uses of the
// other units (their top-level variables, and the modules it starts).
typedef struct COut {
    Const* ks;
    int    kn;
    Buf    decls;
    Buf    funcs;
    Var**  exts;
    int    nexts;
    int*   mods;
    int    nmods;
} COut;

static COut OUTS[FILES_MAX];
// module being compiled
static int  CUR = 0;

#define KS    (OUTS[CUR].ks)
#define KN    (OUTS[CUR].kn)
#define DECLS (OUTS[CUR].decls)
#define FUNCS (OUTS[CUR].funcs)

static int konst (const char* s, int len) {
    for (int i=0; i<KN; i++) {
        if (KS[i].len==len && !memcmp(KS[i].s,s,len)) {
            return i;
        }
    }
    KS = realloc(KS, (KN+1) * sizeof(Const));
    KS[KN].s   = s;
    KS[KN].len = len;
    return KN++;
}

// Decodes the escapes of a single-line string literal, as in Lua.
static char* decode (Tk tk, int* len) {
    char* ret = xalloc(tk.len*4 + 1);
    int n = 0;
    for (int i=0; i<tk.len; i++) {
        int c = (unsigned char) tk.str[i];
        if (c != '\\') {
            ret[n++] = c;
            continue;
        }
        c = (unsigned char) tk.str[++i];
        switch (c) {
            case 'a':  ret[n++] = '\a'; break;
            case 'b':  ret[n++] = '\b'; break;
            case 'f':  ret[n++] = '\f'; break;
            case 'n':  ret[n++] = '\n'; break;
            case 'r':  ret[n++] = '\r'; break;
            case 't':  ret[n++] = '\t'; break;
            case 'v':  ret[n++] = '\v'; break;
            case '\\': ret[n++] = '\\'; break;
            case '"':  ret[n++] = '"';  break;
            case '\'': ret[n++] = '\''; break;
            case 'x': {
                // \xHH
                int v = 0;
                for (int j=0; j<2; j++) {
                    int d = (unsigned char) tk.str[++i];
                    if (d>='0' && d<='9') {
                        v = v*16 + (d - '0');
                    } else if (d>='a' && d<='f') {
                        v = v*16 + (d - 'a' + 10);
                    } else if (d>='A' && d<='F') {
                        v = v*16 + (d - 'A' + 10);
                    } else {
                        err(tk, "invalid escape sequence");
                    }
                }
                ret[n++] = v;
                break;
            }
            case 'z':
                // \z skips the following spaces
                while (tk.str[i+1]==' ' || tk.str[i+1]=='\t') {
                    i++;
                }
                break;
            default:
                if (c>='0' && c<='9') {
                    // \ddd
                    int v = c - '0';
                    for (int j=0; j<2; j++) {
                        int d = (unsigned char) tk.str[i+1];
                        if (d<'0' || d>'9') {
                            break;
                        }
                        v = v*10 + (d - '0');
                        i++;
                    }
                    if (v > 255) {
                        err(tk, "invalid escape sequence");
                    }
                    ret[n++] = v;
                } else {
                    err(tk, "invalid escape sequence");
                }
                break;
        }
    }
    ret[n] = '\0';
    *len = n;
    return ret;
}

// C literal for arbitrary bytes.
static char* quote (const char* s, int len) {
    char* ret = xalloc(len*4 + 3);
    int n = 0;
    ret[n++] = '"';
    for (int i=0; i<len; i++) {
        int c = (unsigned char) s[i];
        if (c>=32 && c<127 && c!='"' && c!='\\' && c!='?') {
            ret[n++] = c;
        } else {
            n += sprintf(&ret[n], "\\%03o", c);
        }
    }
    ret[n++] = '"';
    ret[n] = '\0';
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// OPERANDS
///////////////////////////////////////////////////////////////////////////////

typedef struct Opnd {
    // owned temporary slot, or -1
    int tmp;
    // C expression of the value
    const char* c;
} Opnd;

static Opnd opnd_tmp (Unit* u) {
    Opnd o;
    o.tmp = slot_new(u);
    o.c   = xfmt("L[%d]", o.tmp);
    return o;
}

static Opnd opnd_c (const char* c) {
    Opnd o;
    o.tmp = -1;
    o.c   = c;
    return o;
}

// Releases a consumed operand.
static void opnd_rel (Unit* u, Opnd o) {
    if (o.tmp >= 0) {
        out(u, "a_clr(&L[%d]);", o.tmp);
    }
}

// Stores an operand into a slot, consuming it.
static void opnd_mov (Unit* u, Opnd o, const char* dst) {
    if (o.tmp >= 0) {
        out(u, "a_mov(&%s, &L[%d]);", dst, o.tmp);
    } else {
        out(u, "a_cpy(&%s, %s);", dst, o.c);
    }
}

// Pushes an operand onto the value stack.
static void opnd_push (Unit* u, Opnd o) {
    if (o.tmp >= 0) {
        out(u, "a_pushm(&L[%d]);", o.tmp);
    } else {
        out(u, "a_pushc(%s);", o.c);
    }
}

///////////////////////////////////////////////////////////////////////////////
// DESTINATIONS
///////////////////////////////////////////////////////////////////////////////

typedef enum {
    D_NONE, D_SLOT, D_MULTI
} DKind;

typedef struct Dst {
    DKind kind;
    // D_SLOT: C expression of the slot
    const char* slot;
    // D_MULTI: C variable that receives the number of values
    const char* n;
} Dst;

static Dst dst_none (void) {
    Dst d = { D_NONE, NULL, NULL };
    return d;
}

static Dst dst_slot (const char* slot) {
    Dst d = { D_SLOT, slot, NULL };
    return d;
}

static Dst dst_multi (Unit* u) {
    Dst d = { D_MULTI, NULL, xfmt("n%d", u->ncnts++) };
    return d;
}

// Delivers an operand, consuming it.
static void store (Unit* u, Opnd o, Dst d) {
    switch (d.kind) {
        case D_NONE:
            opnd_rel(u, o);
            break;
        case D_SLOT:
            opnd_mov(u, o, d.slot);
            break;
        case D_MULTI:
            opnd_push(u, o);
            out(u, "%s = 1;", d.n);
            break;
    }
}

// Delivers no value: nil for a slot, nothing for the stack.
static void store_void (Unit* u, Dst d) {
    switch (d.kind) {
        case D_NONE:
            break;
        case D_SLOT:
            out(u, "a_clr(&%s);", d.slot);
            break;
        case D_MULTI:
            out(u, "%s = 0;", d.n);
            break;
    }
}

// Delivers the payload of an exit.
static void store_pay (Unit* u, Dst d) {
    switch (d.kind) {
        case D_NONE:
            out(u, "a_pay_clear();");
            break;
        case D_SLOT:
            out(u, "a_pay_take(&%s, 0);", d.slot);
            out(u, "a_pay_clear();");
            break;
        case D_MULTI:
            out(u, "%s = a_pay_push();", d.n);
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// EXPRESSIONS
///////////////////////////////////////////////////////////////////////////////

static void gen       (Unit* u, Expr* e, Dst d);
static Opnd gen_expr  (Unit* u, Expr* e);
static void gen_block (Unit* u, List* es, Dst d);

// The C name of a top-level variable in a program with modules.
static const char* glb_name (Var* v) {
    if (v->nth > 0) {
        return xfmt("atm_G_%s_%s_%d", MODS[v->mod].name, v->tk.str, v->nth);
    }
    return xfmt("atm_G_%s_%s", MODS[v->mod].name, v->tk.str);
}

// C expression of the raw storage of a variable: a static variable,
// a slot of its function, or a capture of the current function.
// For a boxed variable, this is the box.
static const char* var_raw (Unit* u, Var* v) {
    if (v->csym != NULL) {
        return v->csym;
    } else if (v->global) {
        if (NMODS == 1) {
            return xfmt("atm_G%d_%s", v->id, v->tk.str);
        }
        // a program with modules: a name that does not change with
        // the other modules, declared in the units that use it
        if (v->mod != CUR) {
            COut* o = &OUTS[CUR];
            int has = 0;
            for (int i=0; i<o->nexts; i++) {
                has = has || (o->exts[i] == v);
            }
            if (!has) {
                o->exts = realloc(o->exts, (o->nexts+1) * sizeof(Var*));
                o->exts[o->nexts++] = v;
            }
        }
        return glb_name(v);
    } else if (v == u->proto->self) {
        // the function that the variable names
        return "SELF";
    } else if (v->proto == u->proto) {
        if (v->slot < 0) {
            v->slot  = slot_new(u);
            v->level = (u->level > 0) ? u->level : 0;
            slot_name(u, v->slot);
        }
        return xfmt("L[%d]", v->slot);
    } else {
        for (int i=0; i<u->proto->ncaps; i++) {
            if (u->proto->caps[i] == v) {
                return xfmt("self->caps[%d]", i);
            }
        }
        err(v->tk, "internal error : capture not found");
        return NULL;
    }
}

// C expression of the value of a variable.
static const char* var_c (Unit* u, Var* v) {
    const char* raw = var_raw(u, v);
    return v->boxed ? xfmt("A_BOX_OF(%s)->v", raw) : raw;
}

// Appends the expressions in `src` to `dst`, splicing nested lists:
// f((a,b),c) is f(a,b,c).
static void flatten (List* dst, List* src) {
    for (int i=0; i<src->n; i++) {
        Expr* e = src->v[i];
        if (e->tag == E_LIST) {
            flatten(dst, &e->u.list.es);
        } else {
            list_add(dst, e);
        }
    }
}

// Expressions that may yield a number of values other than one.
static int is_multi (Expr* e) {
    switch (e->tag) {
        case E_CALL:
        case E_DOTS:
        case E_LIST:
        case E_BLOCK:
        case E_DO:
        case E_IFS:
        case E_LOOP:
        case E_CATCH:
        case E_WHERE:
        case E_AWAIT:
        case E_PAR:
            return 1;
        default:
            return 0;
    }
}

// Pushes the values of a list of expressions: one of each, except the
// last, which gives all of its values. Sets the count in `n`.
static void gen_args (Unit* u, List* es, const char* n) {
    List args = { 0, NULL };
    flatten(&args, es);
    int k = args.n;

    int multi = (k > 0) && is_multi(args.v[k-1]);
    int fix = multi ? k-1 : k;
    Opnd* xs = xalloc((k+1) * sizeof(Opnd));
    int nx = 0;
    for (int i=0; i<fix; i++) {
        xs[nx++] = gen_expr(u, args.v[i]);
    }

    if (!multi) {
        for (int i=0; i<nx; i++) {
            opnd_push(u, xs[i]);
        }
        out(u, "%s = %d;", n, nx);
        return;
    }

    // the last expression may suspend or fail: the others wait in
    // slots and are inserted below its values
    Dst d = { D_MULTI, NULL, n };
    gen(u, args.v[k-1], d);
    if (nx > 0) {
        out(u, "a_open(%s, %d);", n, nx);
        for (int i=0; i<nx; i++) {
            if (xs[i].tmp >= 0) {
                out(u, "a_putm(a_top-%s-%d, &L[%d]);", n, nx-i, xs[i].tmp);
            } else {
                out(u, "a_putc(a_top-%s-%d, %s);", n, nx-i, xs[i].c);
            }
        }
        out(u, "%s += %d;", n, nx);
    }
}

// A number literal is an integer if it has neither a fraction nor an
// exponent. A decimal one that does not fit becomes a float; a
// hexadecimal one wraps.
static const char* gen_num (const char* str) {
    int hex = (str[0]=='0' && (str[1]=='x' || str[1]=='X'));
    if (strpbrk(str, hex ? ".pP" : ".eE") == NULL) {
        unsigned long long v = strtoull(str, NULL, 0);
        unsigned long long max = OPT_INT32 ?
            2147483647ULL : 9223372036854775807ULL;
        if (hex || (strlen(str)<=19 && v<=max)) {
            return xfmt("a_int((AInt)%lluULL)", v);
        }
    }
    return xfmt("a_num(%.17g)", strtod(str, NULL));
}

// An immutable variable is borrowed in place. A mutable one is
// copied, since it may change before the value is consumed.
static Opnd gen_acc (Unit* u, Expr* e) {
    Var* v = e->var;
    if (v->kind != V_VAR) {
        return opnd_c(var_c(u, v));
    } else {
        Opnd d = opnd_tmp(u);
        out(u, "a_cpy(&%s, %s);", d.c, var_c(u,v));
        return d;
    }
}

// The expression without its parentheses.
static Expr* unparens (Expr* e) {
    while (e->tag == E_PARENS) {
        e = e->u.parens.e;
    }
    return e;
}

// A mutable variable.
static int is_var (Expr* e) {
    e = unparens(e);
    return e->tag==E_ACC && e->var->kind==V_VAR;
}

// Whether the evaluation of `e` runs no code of the program, so that
// it assigns no variable: literals, names, indexes, and operators over
// them (which may raise, but not assign).
static int is_quiet (Expr* e) {
    switch (e->tag) {
        case E_NIL: case E_BOOL: case E_NUM: case E_CLK:
        case E_STR: case E_TAG: case E_ACC:
            return 1;
        case E_PARENS:
            return is_quiet(e->u.parens.e);
        case E_UNO:
            return is_quiet(e->u.uno.e);
        case E_BIN:
            return is_quiet(e->u.bin.e1) && is_quiet(e->u.bin.e2);
        case E_INDEX:
            return is_quiet(e->u.index.t) &&
                (e->u.index.kind!=IDX_EXPR || is_quiet(e->u.index.idx));
        default:
            return 0;
    }
}

// An operand that is consumed at once, with no code of the program
// in between: a mutable variable is borrowed in place, with no copy.
static Opnd gen_now (Unit* u, Expr* e) {
    if (is_var(e)) {
        return opnd_c(var_c(u, unparens(e)->var));
    }
    return gen_expr(u, e);
}

static Opnd gen_uno (Unit* u, Expr* e) {
    Opnd x = gen_now(u, e->u.uno.e);
    Opnd d = opnd_tmp(u);
    if (!strcmp(e->tk.str, "-")) {
        out_try(u, "a_neg(&%s, %s)", d.c, x.c);
    } else if (!strcmp(e->tk.str, "#")) {
        out_try(u, "a_len(&%s, %s)", d.c, x.c);
    } else if (!strcmp(e->tk.str, "~")) {
        out_try(u, "a_bnot(&%s, %s)", d.c, x.c);
    } else {
        out(u, "a_not(&%s, %s);", d.c, x.c);
    }
    opnd_rel(u, x);
    return d;
}

// a && b, a || b: the second operand is only evaluated if needed, and
// the result is the value of the deciding operand.
static Opnd gen_logic (Unit* u, Expr* e) {
    int and = !strcmp(e->tk.str, "&&");
    Opnd d = opnd_tmp(u);
    opnd_mov(u, gen_expr(u,e->u.bin.e1), d.c);
    out(u, "if (%sa_truthy(%s)) {", and ? "" : "!", d.c);
    u->ind++;
    opnd_mov(u, gen_expr(u,e->u.bin.e2), d.c);
    u->ind--;
    out(u, "}");
    return d;
}

// Applies a binary operator to two operands, consuming them. The
// result goes to the place `to`, or to a temporary if it is NULL.
static Opnd gen_binop_to (Unit* u, Tk tk, const char* op, Opnd x, Opnd y,
                          const char* to)
{
    // operator, runtime function, swapped operands
    static const char* OPS[][3] = {
        { "+",  "a_add",  "" }, { "-",  "a_sub", ""  },
        { "*",  "a_mul",  "" }, { "/",  "a_div", ""  },
        { "//", "a_idiv", "" }, { "%",  "a_mod", ""  },
        { "++", "a_cat",  "" },
        { "<",  "a_lt",   "" }, { "<=", "a_le",  ""  },
        { ">",  "a_lt",  "x" }, { ">=", "a_le",  "x" },
        { NULL, NULL, NULL }
    };
    // operator, predicate, negated
    static const char* PREDS[][3] = {
        { "==",  "a_eq",  ""  }, { "!=",  "a_eq",  "!" },
        { "??",  "a_is",  ""  }, { "!?",  "a_is",  "!" },
        { NULL, NULL, NULL }
    };
    // operator, bitwise operation
    static const char* BITS[][2] = {
        { "&",  "A_BAND" }, { "|",  "A_BOR" }, { "^", "A_BXOR" },
        { "<<", "A_SHL"  }, { ">>", "A_SHR" },
        { NULL, NULL }
    };

    Opnd d = (to == NULL) ? opnd_tmp(u) : opnd_c(to);
    int done = 0;
    for (int i=0; PREDS[i][0]!=NULL; i++) {
        if (!strcmp(PREDS[i][0], op)) {
            out(u, "a_set(&%s, a_bool(%s%s(%s, %s)));",
                d.c, PREDS[i][2], PREDS[i][1], x.c, y.c);
            done = 1;
        }
    }
    for (int i=0; OPS[i][0]!=NULL; i++) {
        if (!strcmp(OPS[i][0], op)) {
            int swap = (OPS[i][2][0] != '\0');
            out_try(u, "%s(&%s, %s, %s)",
                OPS[i][1], d.c, swap ? y.c : x.c, swap ? x.c : y.c);
            done = 1;
        }
    }
    for (int i=0; BITS[i][0]!=NULL; i++) {
        if (!strcmp(BITS[i][0], op)) {
            out_try(u, "a_bit(%s, &%s, %s, %s)", BITS[i][1], d.c, x.c, y.c);
            done = 1;
        }
    }
    if (!done) {
        err(tk, "operator not implemented");
    }
    opnd_rel(u, x);
    opnd_rel(u, y);
    return d;
}

static Opnd gen_binop (Unit* u, Tk tk, const char* op, Opnd x, Opnd y) {
    return gen_binop_to(u, tk, op, x, y, NULL);
}

static Opnd gen_bin_to (Unit* u, Expr* e, const char* to) {
    const char* op = e->tk.str;
    // the second operand is consumed at once; the first, if the
    // second runs no code of the program
    Opnd x = is_quiet(e->u.bin.e2) ?
        gen_now(u, e->u.bin.e1) : gen_expr(u, e->u.bin.e1);
    Opnd y = gen_now(u, e->u.bin.e2);
    return gen_binop_to(u, e->tk, op, x, y, to);
}

static Opnd gen_bin (Unit* u, Expr* e) {
    const char* op = e->tk.str;
    if (!strcmp(op,"&&") || !strcmp(op,"||")) {
        return gen_logic(u, e);
    }
    return gen_bin_to(u, e, NULL);
}

static int op_scalar (const char* op);

// The value of an operator that never gives an object (a number or a
// boolean) goes straight to the variable in `to`, with no temporary:
// the operator reads its operands before it writes, writes only if it
// does not raise, and there is nothing to name or to check. Returns
// whether `e` is such an operator, and was compiled.
static int gen_scalar_to (Unit* u, Expr* e, const char* to) {
    e = unparens(e);
    if (e->tag==E_BIN && op_scalar(e->tk.str)) {
        gen_bin_to(u, e, to);
        return 1;
    }
    return 0;
}

// A spawn that is the value of a field of the table `holder` is pinned
// from birth with the table, if the table is pinned.
static void gen_born_in (Unit* u, Expr* val, const char* holder) {
    if (val->tag==E_SPAWN && val->u.spawn.pool==NULL) {
        u->born  = val;
        u->bornh = holder;
    }
}

// [ e, id=e, @(k)=e ]: positional entries are numbered from 0, and
// the last entry, if positional, gives all of its values.
static Opnd gen_table (Unit* u, Expr* e) {
    Opnd d = opnd_tmp(u);
    out(u, "a_set(&%s, a_tab());", d.c);
    if (u->born == e) {
        // pinned from birth, so it may hold other pinned values
        u->born = NULL;
        out_try(u, "a_pin(&%s)", d.c);
    }
    int n = 0;
    int last = e->u.table.vs.n - 1;
    for (int i=0; i<=last; i++) {
        Expr* key = e->u.table.ks.v[i];
        Expr* val = e->u.table.vs.v[i];
        if (key==NULL && i==last && is_multi(val)) {
            Dst m = dst_multi(u);
            gen(u, val, m);
            out_try(u, "a_tab_fill(%s, %d, %s)", d.c, n, m.n);
        } else if (key == NULL) {
            gen_born_in(u, val, d.c);
            Opnd v = gen_expr(u, val);
            out(u, "a_born(%s, %s);", d.c, v.c);
            out_try(u, "a_setindex(%s, a_int(%d), %s)", d.c, n++, v.c);
            opnd_rel(u, v);
        } else {
            Opnd k = gen_expr(u, key);
            gen_born_in(u, val, d.c);
            Opnd v = gen_expr(u, val);
            out(u, "a_born(%s, %s);", d.c, k.c);
            out(u, "a_born(%s, %s);", d.c, v.c);
            out_try(u, "a_setindex(%s, %s, %s)", d.c, k.c, v.c);
            opnd_rel(u, k);
            opnd_rel(u, v);
        }
    }
    return d;
}

// :X [...]: tags the table in place.
static Opnd gen_tagged (Unit* u, Expr* e) {
    Opnd t = gen_expr(u, e->u.tagged.e);
    out_try(u, "a_tag(%s, atm_K[%d])", t.c, konst(e->tk.str+1,e->tk.len-1));
    return t;
}

// t@(e)  t.x  t@#  t@+
static Opnd gen_index (Unit* u, Expr* e) {
    Opnd t = gen_expr(u, e->u.index.t);
    Opnd d = opnd_tmp(u);
    switch (e->u.index.kind) {
        case IDX_EXPR: {
            Opnd k = gen_expr(u, e->u.index.idx);
            out_try(u, "a_index(&%s, %s, %s)", d.c, t.c, k.c);
            opnd_rel(u, k);
            break;
        }
        case IDX_LAST:
            out_try(u, "a_index_last(&%s, %s)", d.c, t.c);
            break;
        case IDX_NEXT:
            // the next position is always empty
            out_try(u, "a_len(&%s, %s)", d.c, t.c);
            out(u, "a_clr(&%s);", d.c);
            break;
    }
    opnd_rel(u, t);
    return d;
}

///////////////////////////////////////////////////////////////////////////////
// BLOCKS
///////////////////////////////////////////////////////////////////////////////

// Sequence of expressions in a new block, whose value is the value of
// the last expression.
//
// A block may stop an exit: its payload is delivered to a destination
// and control goes to an end label.
//  - break: the body of a loop
//  - skip: the body of a loop, which goes to its next step
//  - return: the body of a lambda expanded in place
//  - error: the body of a catch, which only diverts to its label
//  - escape: a `do :X` block, if the tag matches constant `esck`
//  - join: a `par :any`, identified by slot `joink`
typedef struct Hnd {
    int brk;
    Dst brkd;
    int skp;
    int ret;
    Dst retd;
    int err;
    int esc;
    int esck;
    Dst escd;
    int join;
    int joink;
    Dst joind;
} Hnd;

static Hnd hnd_none (void) {
    Hnd h;
    memset(&h, 0, sizeof(h));
    return h;
}

// An open block: its cleanup label, first slot, and what it suspends
// of the enclosing block.
typedef struct Blk {
    int up;
    int lo;
    int cl;
    Defer* defs;
    int ndefs;
    int box;
} Blk;

static Blk blk_open (Unit* u) {
    Blk b;
    b.up = u->cleanup;
    b.lo = *nslots(u);
    b.cl = ++u->nlabels;
    b.defs  = u->defs;
    b.ndefs = u->ndefs;
    b.box   = u->box;
    u->box  = 0;
    u->cleanup = b.cl;
    u->defs  = NULL;
    u->ndefs = 0;
    if (u->orph) {
        // the first finalizer: the last one to execute
        u->orph = 0;
        u->defs = realloc(u->defs, sizeof(Defer));
        u->defs[0].flag = -1;
        u->defs[0].e    = NULL;
        u->defs[0].pin  = NULL;
        u->defs[0].trail = -1;
        u->defs[0].user = 0;
        u->defs[0].orph = 1;
        u->defs[0].out  = 0;
        u->ndefs = 1;
    }
    u->level++;
    out(u, "{");
    u->ind++;
    out(u, "a_depth = D + %d;", u->level);
    return b;
}

// The address of the slot of a pin, for its end. A boxed one (a `pin
// var` that a closure captures) has no box yet if an exit left the block
// before its declaration: the end then sees nil.
static const char* pin_ref (const char* slot) {
    const char* pre = "A_BOX_OF(";
    size_t n = strlen(pre);
    size_t m = strlen(slot);
    if (strncmp(slot, pre, n)==0 && m>n+4 && !strcmp(slot+m-4, ")->v")) {
        return xfmt("a_box_ref(&%.*s)", (int)(m-n-4), slot+n);
    }
    return xfmt("&%s", slot);
}

// Closes a block: runs its finalizers, releases its slots, and then
// handles or propagates a pending exit. The block gives its values
// to `d` when it ends normally.
static void blk_close (Unit* u, Blk b, Dst d, Hnd h) {
    u->ind--;
    out0(u, "CL_%d: A_UNUSED;", b.cl);
    u->ind++;

    // an exit may arrive from a deeper block
    out(u, "a_depth = D + %d;", u->level);

    // pins are aborted and defers that were reached run, in reverse
    // order, whatever the exit. The pending exit is set aside
    // meanwhile, and replaced if a defer exits itself.
    Defer* mine = u->defs;
    int   nmine = u->ndefs;

    // what leaves a block with pins is unpinned before they end; so
    // is a captured variable of the block, with a closure that leaves
    int user = 0;
    for (int i=0; i<nmine; i++) {
        user = user || (mine[i].pin!=NULL && mine[i].user);
    }
    int box = u->box;
    u->box = b.box;
    if (user || box) {
        switch (d.kind) {
            case D_NONE:
                out(u, "st = a_blk_xfer(st, NULL, 0);");
                break;
            case D_SLOT:
                out(u, "st = a_blk_xfer(st, &%s, 1);", d.slot);
                break;
            case D_MULTI:
                out(u, "st = a_blk_xfer(st, a_stk+a_top-%s, %s);", d.n, d.n);
                break;
        }
    }

    if (user && u->task && !u->indefer) {
        // tasks that hold the pins of the block end before them, with
        // the same wait as below
        int pc = ++u->npcs;
        out0(u, "LW_%d: ;", pc);
        out(u, "if (a_conf_end(tr, &st, base)) {");
        for (int j=0; j<nmine; j++) {
            if (mine[j].trail >= 0) {
                out(u, "    a_trail_doom(t, %d);", mine[j].trail);
            } else if (mine[j].pin != NULL) {
                out(u, "    a_pin_doom(%s);", pin_ref(mine[j].pin));
            } else if (mine[j].orph) {
                out(u, "    a_orph_doom(tr);");
            }
        }
        out(u, "    tr->pc = %d;", pc);
        out(u, "    return A_AWAIT;");
        out0(u, "AW_%d:", pc);
        out(u, "    base = a_top;");
        out(u, "    a_lin = %d;", u->lin);
        out(u, "    a_depth = D + %d;", u->level);
        out(u, "    st = a_pin_resumed(tr);");
        if (d.kind == D_MULTI) {
            out(u, "    %s = a_top - base;", d.n);
        }
        out(u, "    goto LW_%d;", pc);
        out(u, "}");
    } else if (user) {
        out(u, "a_conf_end(NULL, &st, 0);");
    }

    for (int i=nmine-1; i>=0; i--) {
        // A task of the block that is running only unwinds when control
        // returns to it: the exit waits for that, so that the
        // finalizers still run from the inside out. Trails and orphans
        // only exist in tasks, outside defers.
        const char* end = NULL;
        if (mine[i].trail >= 0) {
            end = xfmt("a_trail_end(tr, %d, &st, base)", mine[i].trail);
        } else if (mine[i].orph) {
            // the tasks that nobody pinned
            end = "a_orph_end(tr, &st, base)";
        } else if (mine[i].pin!=NULL && u->task && !u->indefer) {
            end = xfmt("a_pin_end(tr, %s, &st, base)", pin_ref(mine[i].pin));
        }
        if (end != NULL) {
            int pc = ++u->npcs;
            out0(u, "LW_%d: ;", pc);
            out(u, "if (%s) {", end);
            // the other tasks of the block stop meanwhile
            for (int j=0; j<i; j++) {
                if (mine[j].trail >= 0) {
                    out(u, "    a_trail_doom(t, %d);", mine[j].trail);
                } else if (mine[j].pin != NULL) {
                    out(u, "    a_pin_doom(%s);", pin_ref(mine[j].pin));
                } else if (mine[j].orph) {
                    out(u, "    a_orph_doom(tr);");
                }
            }
            out(u, "    tr->pc = %d;", pc);
            out(u, "    return A_AWAIT;");
            out0(u, "AW_%d:", pc);
            out(u, "    base = a_top;");
            out(u, "    a_lin = %d;", u->lin);
            out(u, "    a_depth = D + %d;", u->level);
            out(u, "    st = a_pin_resumed(tr);");
            if (d.kind == D_MULTI) {
                out(u, "    %s = a_top - base;", d.n);
            }
            out(u, "    goto LW_%d;", pc);
            out(u, "}");
            continue;
        }
        if (mine[i].pin != NULL) {
            out(u, "a_pin_end(NULL, %s, &st, 0);", pin_ref(mine[i].pin));
            continue;
        }
        if (mine[i].out == 1) {
            out(u, "if (a_truthy(L[%d])) a_out_tab_end(L[%d], L[%d], L[%d]);",
                mine[i].flag, mine[i].ot, mine[i].ok, mine[i].ov);
            continue;
        } else if (mine[i].out == 2) {
            out(u, "if (a_truthy(L[%d])) a_out_var_end(&%s, L[%d]);",
                mine[i].flag, mine[i].odst, mine[i].ov);
            continue;
        } else if (mine[i].out == 3) {
            out(u, "if (a_truthy(L[%d])) {", mine[i].flag);
            out(u, "    AStatus st_c = a_ctx_end(t, %d);", mine[i].ot);
            out(u, "    if (st == A_OK) st = st_c;");
            out(u, "}");
            continue;
        }
        int id = ++u->nlabels;
        out(u, "if (a_truthy(L[%d])) {", mine[i].flag);
        u->ind++;
        out(u, "AStatus st_%d = st;", id);
        out(u, "APay pay_%d;", id);
        out(u, "a_pay_save(&pay_%d);", id);
        out(u, "st = A_OK;");
        out(u, "{");
        u->ind++;
        // values of the block may be pending on the stack
        out(u, "int base = a_top;");
        u->cleanup = id;
        u->indefer++;
        gen_block(u, &mine[i].e->u.defer.blk->u.block.es, dst_none());
        u->indefer--;
        u->ind--;
        out0(u, "CL_%d: A_UNUSED;", id);
        out(u, "}");
        out(u, "if (st == A_OK) {");
        out(u, "    st = st_%d;", id);
        out(u, "    a_pay_restore(&pay_%d);", id);
        out(u, "} else {");
        out(u, "    a_pay_drop(&pay_%d);", id);
        out(u, "}");
        u->ind--;
        out(u, "}");
    }
    // pinned data is released after all code of the block has run
    for (int i=nmine-1; i>=0; i--) {
        if (mine[i].pin != NULL) {
            out(u, "a_pin_clr(%s);", pin_ref(mine[i].pin));
        }
    }
    u->cleanup = b.cl;
    u->defs  = b.defs;
    u->ndefs = b.ndefs;

    if (*nslots(u) > b.lo) {
        // only the variables of the block may give a ceiling: its
        // temporaries are empty when it ends
        int n = *nslots(u);
        for (int i=b.lo; i<n; ) {
            if (!slot_isname(u, i)) {
                i++;
                continue;
            }
            int j = i;
            while (j<n && slot_isname(u, j)) {
                j++;
            }
            out(u, "a_unname(&L[%d], %d, D + %d);", i, j-i, u->level);
            i = j;
        }
        out(u, "a_clrs(&L[%d], %d);", b.lo, *nslots(u)-b.lo);
    }
    // the parameters of a function or task end with its body
    if (u->level==0 && u->npar>0 && u->root==NULL) {
        out(u, "a_unname(&L[0], %d, D);", u->npar);
    }
    u->level--;
    out(u, "a_depth = D + %d;", u->level);
    // so is one that the block dropped
    out(u, "if (a_ndrop) st = a_drop_flush(st);");
    if (h.brk > 0) {
        out(u, "if (st == A_BREAK) {");
        u->ind++;
        out(u, "st = A_OK;");
        out(u, "a_settop(base);");
        store_pay(u, h.brkd);
        out(u, "goto LE_%d;", h.brk);
        u->ind--;
        out(u, "}");
    }
    if (h.skp > 0) {
        out(u, "if (st == A_SKIP) {");
        u->ind++;
        out(u, "st = A_OK;");
        out(u, "a_settop(base);");
        out(u, "goto LE_%d;", h.skp);
        u->ind--;
        out(u, "}");
    }
    if (h.esc > 0) {
        out(u, "if (st==A_ESCAPE && a_escape_is(atm_K[%d])) {", h.esck);
        u->ind++;
        out(u, "st = A_OK;");
        out(u, "a_settop(base);");
        out(u, "a_escape_vals();");
        store_pay(u, h.escd);
        out(u, "goto LE_%d;", h.esc);
        u->ind--;
        out(u, "}");
    }
    if (h.ret > 0) {
        out(u, "if (st == A_RETURN) {");
        u->ind++;
        out(u, "st = A_OK;");
        out(u, "a_settop(base);");
        store_pay(u, h.retd);
        out(u, "goto LE_%d;", h.ret);
        u->ind--;
        out(u, "}");
    }
    if (h.join > 0) {
        out(u, "if (st==A_JOIN && a_join_key==&L[%d]) {", h.joink);
        u->ind++;
        out(u, "st = A_OK;");
        out(u, "a_settop(base);");
        store_pay(u, h.joind);
        out(u, "goto LE_%d;", h.join);
        u->ind--;
        out(u, "}");
    }
    out(u, "if (st != A_OK) {");
    out(u, "    a_settop(base);");
    if (h.err > 0) {
        out(u, "    if (st == A_ERROR) goto LC_%d;", h.err);
    }
    out(u, "    goto CL_%d;", b.up);
    out(u, "}");
    u->ind--;
    out(u, "}");

    u->cleanup = b.up;
}

static void gen_block_x (Unit* u, List* es, Dst d, Hnd h) {
    Blk b = blk_open(u);
    for (int i=0; i<es->n; i++) {
        // C diagnostics and debuggers point to the source line
        Tk tk = es->v[i]->tk;
        out0(u, "#line %d %s", tk.lin, file_q(tk));
        out(u, "a_lin = %d;", lin_of(tk));
        u->lin = lin_of(tk);
        gen(u, es->v[i], (i == es->n-1) ? d : dst_none());
        if (i < es->n-1) {
            // an unpinned task that the statement dropped is aborted
            out(u, "if (a_ndrop && (st = a_drop_flush(A_OK)) != A_OK) "
                   "goto CL_%d;", u->cleanup);
        }
    }
    if (es->n == 0) {
        store_void(u, d);
    }
    blk_close(u, b, d, h);
}

static void gen_block (Unit* u, List* es, Dst d) {
    gen_block_x(u, es, d, hnd_none());
}

// do :X {...}: stops a matching escape, whose values become the
// values of the block.
static void gen_do (Unit* u, Expr* e, Dst d) {
    List* es = &e->u.xdo.blk->u.block.es;
    if (!e->u.xdo.esc) {
        gen_block(u, es, d);
        return;
    }
    Hnd h = hnd_none();
    h.esc  = ++u->nlabels;
    h.esck = konst(e->tk.str+1, e->tk.len-1);
    h.escd = d;
    gen_block_x(u, es, d, h);
    out0(u, "LE_%d: ;", h.esc);
}

// defer {...}: registers the body to run when the block exits.
static void gen_defer (Unit* u, Expr* e) {
    int flag = slot_new(u);
    u->defs = realloc(u->defs, (u->ndefs+1) * sizeof(Defer));
    u->defs[u->ndefs].flag = flag;
    u->defs[u->ndefs].e    = e;
    u->defs[u->ndefs].pin  = NULL;
    u->defs[u->ndefs].trail = -1;
    u->defs[u->ndefs].user = 0;
    u->defs[u->ndefs].orph = 0;
    u->defs[u->ndefs].out  = 0;
    u->ndefs++;
    out(u, "a_set(&L[%d], a_bool(1));", flag);
}

// The enclosing block aborts the task or pool in `slot` on exit.
static void gen_pin_end (Unit* u, const char* slot, int user) {
    u->defs = realloc(u->defs, (u->ndefs+1) * sizeof(Defer));
    u->defs[u->ndefs].flag = -1;
    u->defs[u->ndefs].e    = NULL;
    u->defs[u->ndefs].pin  = slot;
    u->defs[u->ndefs].trail = -1;
    u->defs[u->ndefs].user = user;
    u->defs[u->ndefs].orph = 0;
    u->defs[u->ndefs].out  = 0;
    u->ndefs++;
}

// The enclosing block aborts the trail on exit.
static void gen_trail_end (Unit* u, int trail) {
    u->defs = realloc(u->defs, (u->ndefs+1) * sizeof(Defer));
    u->defs[u->ndefs].flag = -1;
    u->defs[u->ndefs].e    = NULL;
    u->defs[u->ndefs].pin  = NULL;
    u->defs[u->ndefs].trail = trail;
    u->defs[u->ndefs].user = 0;
    u->defs[u->ndefs].orph = 0;
    u->defs[u->ndefs].out  = 0;
    u->ndefs++;
}

// The value in `slot` becomes owned by the enclosing block, which
// aborts it on exit, if it is a task or a pool.
static void gen_pin (Unit* u, const char* slot) {
    out_try(u, "a_pin(&%s)", slot);
    gen_pin_end(u, slot, 1);
}

// e where { x = e1 ; y = e2 }
static void gen_where (Unit* u, Expr* e, Dst d) {
    List es = { 0, NULL };
    for (int i=0; i<e->u.where.dcls.n; i++) {
        list_add(&es, e->u.where.dcls.v[i]);
    }
    list_add(&es, e->u.where.e);
    gen_block(u, &es, d);
}

///////////////////////////////////////////////////////////////////////////////
// DECLARATIONS AND ASSIGNMENTS
///////////////////////////////////////////////////////////////////////////////

// A captured mutable variable gets an unpinned box every time its
// declaration executes.
static void gen_boxes (Unit* u, Expr* e) {
    for (int i=0; i<e->u.dcl.n; i++) {
        if (e->vars[i]->boxed) {
            out(u, "a_set(&%s, a_box());", var_raw(u,e->vars[i]));
            u->box = 1;
        }
    }
}

// The depth of the block of a variable, as a C expression.
static const char* var_depth (Unit* u, Var* x) {
    if (x->boxed) {
        return xfmt("(int)%s.u.o->depth", var_raw(u,x));
    } else if (x->global) {
        return "1";
    } else {
        return xfmt("D + %d", (x->level < 0) ? 0 : x->level);
    }
}

// An operator whose result is never an object (a number or a boolean).
static int op_scalar (const char* op) {
    static const char* OPS[] = {
        "+", "-", "*", "/", "//", "%", "<", "<=", ">", ">=", "==", "!=",
        "??", "!?", "&", "|", "^", "<<", ">>", NULL
    };
    for (int i=0; OPS[i]!=NULL; i++) {
        if (!strcmp(OPS[i], op)) {
            return 1;
        }
    }
    return 0;
}

// An expression whose value is never an object.
static int expr_scalar (Expr* e) {
    switch (e->tag) {
        case E_NIL:
        case E_BOOL:
        case E_NUM:
        case E_STR:
        case E_TAG:
        case E_CLK:
            return 1;
        case E_PARENS:
            return expr_scalar(e->u.parens.e);
        case E_BIN:
            return op_scalar(e->tk.str);
        case E_UNO:
            return !strcmp(e->tk.str,"-") || !strcmp(e->tk.str,"#") ||
                   !strcmp(e->tk.str,"~") || !strcmp(e->tk.str,"!") ||
                   !strcmp(e->tk.str,"not");
        default:
            return 0;
    }
}

// After a declaration: a pin takes ownership of a task or pool; a val
// or var only accepts one that is pinned elsewhere.
static void gen_dcl_chk (Unit* u, Expr* e) {
    for (int i=0; i<e->u.dcl.n; i++) {
        const char* v = var_c(u, e->vars[i]);
        if (e->vars[i]->kind == V_PIN) {
            gen_pin(u, v);
        } else if (u->scalar) {
            // no object: nothing to name or to check
        } else {
            // a name: it refers to a value that has an owner, or holds
            // an unpinned one, which gets its ceiling here
            out(u, "a_name(%s, %s);", v, var_depth(u, e->vars[i]));
            // a captured variable belongs to the block that declares it
            if (e->vars[i]->boxed) {
                out_try(u, "a_chk_store(%s, (int)%s.u.o->depth)", v,
                    var_raw(u,e->vars[i]));
            }
        }
    }
}

// val x = e  |  var x, y = e  |  pin x = e
static void gen_dcl (Unit* u, Expr* e) {
    int k = e->u.dcl.n;
    const char** vs = xalloc(k * sizeof(char*));
    if (e->u.dcl.set == NULL) {
        for (int i=0; i<k; i++) {
            const char* v = var_c(u, e->vars[i]);
            // pin var x: what it gets later ends with the block
            if (e->vars[i]->kind == V_PIN) {
                gen_pin_end(u, v, 1);
            }
        }
    } else if (k == 1) {
        // pin x = [...]  |  pin x = :T [...]
        Expr* tab = e->u.dcl.set;
        if (tab->tag == E_TAGGED) {
            tab = tab->u.tagged.e;
        }
        int born = (e->vars[0]->kind==V_PIN && tab->tag==E_TABLE);
        // pin x = spawn T(...)
        if (e->vars[0]->kind==V_PIN && tab->tag==E_SPAWN &&
            tab->u.spawn.pool==NULL)
        {
            born = 1;
        }
        if (born) {
            u->born = tab;
        }
        if (!born && e->vars[0]->kind!=V_PIN && !e->vars[0]->boxed &&
            gen_scalar_to(u, e->u.dcl.set, var_c(u,e->vars[0])))
        {
            return;
        }
        Opnd o = gen_expr(u, e->u.dcl.set);
        // the initialization may span lines: an error of the
        // declaration is in its own line
        out(u, "a_lin = %d;", lin_of(e->tk));
        gen_boxes(u, e);
        opnd_mov(u, o, var_c(u,e->vars[0]));
        if (born) {
            gen_pin_end(u, var_c(u,e->vars[0]), 1);
        } else {
            u->scalar = expr_scalar(e->u.dcl.set);
            gen_dcl_chk(u, e);
            u->scalar = 0;
        }
        return;
    } else {
        Dst m = dst_multi(u);
        gen(u, e->u.dcl.set, m);
        out(u, "a_lin = %d;", lin_of(e->tk));
        out(u, "a_adj(%s, %d);", m.n, k);
        gen_boxes(u, e);
        for (int i=0; i<k; i++) {
            vs[i] = var_c(u, e->vars[i]);
        }
        for (int i=k-1; i>=0; i--) {
            out(u, "a_pop(&%s);", vs[i]);
        }
        gen_dcl_chk(u, e);
        return;
    }
    gen_boxes(u, e);
}

// A destination of a set, with its table and key already evaluated.
typedef struct Loc {
    Expr* e;
    Opnd  t;
    Opnd  k;
} Loc;

static Loc gen_loc (Unit* u, Expr* dst) {
    Loc l;
    l.e = dst;
    l.t = opnd_c(NULL);
    l.k = opnd_c(NULL);
    if (dst->tag == E_INDEX) {
        l.t = gen_expr(u, dst->u.index.t);
        if (dst->u.index.kind == IDX_EXPR) {
            l.k = gen_expr(u, dst->u.index.idx);
        }
    } else if (dst->tag!=E_ACC && dst->tag!=E_PUB) {
        err(dst->tk, "assignment : not implemented");
    }
    return l;
}

// Assigns an operand to a destination, consuming both.
static void gen_assign_x (Unit* u, Loc l, Opnd v) {
    if (l.e->tag==E_ACC && l.e->var->kind==V_PIN) {
        // set x = v, with `x` a pin: it owns the new value, in the
        // block of the variable, and releases the old one
        Var* x = l.e->var;
        const char* dst = var_c(u, x);
        if (x->boxed) {
            out_try(u, "a_pin_set(&%s, %s, (int)%s.u.o->depth)", dst, v.c,
                var_raw(u,x));
        } else if (x->global) {
            out_try(u, "a_pin_set(&%s, %s, 1)", dst, v.c);
        } else {
            out_try(u, "a_pin_set(&%s, %s, D + %d)", dst, v.c, x->level);
        }
        opnd_rel(u, v);
        return;
    }
    if (l.e->tag==E_ACC && u->scalar) {
        // no object: nothing to name or to check
        opnd_mov(u, v, var_c(u, l.e->var));
        return;
    }
    if (l.e->tag == E_ACC) {
        // a name: it refers to a value that has an owner, or holds a
        // unpinned one, which gets its ceiling in the block of the name
        out(u, "a_name(%s, %s);", v.c, var_depth(u, l.e->var));
        // a pinned value cannot go to a variable that outlives its
        // pin block: a captured variable (which belongs to the block
        // that declares it), a top-level variable, or an outer block
        Var* x = l.e->var;
        const char* dst = var_c(u, x);
        if (x->boxed) {
            out_try(u, "a_chk_store(%s, (int)%s.u.o->depth)", v.c,
                var_raw(u,x));
        } else if (x->global) {
            out_try(u, "a_chk_store(%s, 1)", v.c);
        } else if (x->level < u->level) {
            out_try(u, "a_chk_store(%s, D + %d)", v.c, x->level);
        }
        opnd_mov(u, v, dst);
        return;
    }
    if (l.e->tag == E_PUB) {
        out_try(u, "a_pub_set(NULL, %s)", v.c);
        opnd_rel(u, v);
        return;
    }
    switch (l.e->u.index.kind) {
        case IDX_EXPR:
            out_try(u, "a_setindex(%s, %s, %s)", l.t.c, l.k.c, v.c);
            opnd_rel(u, l.k);
            break;
        case IDX_LAST:
            out_try(u, "a_setindex_last(%s, %s)", l.t.c, v.c);
            break;
        case IDX_NEXT:
            out_try(u, "a_setindex_next(%s, %s)", l.t.c, v.c);
            break;
    }
    opnd_rel(u, v);
    opnd_rel(u, l.t);
}

static void gen_assign (Unit* u, Loc l, Opnd v) {
    gen_assign_x(u, l, v);
}

// Reads the current value of a destination into a temporary.
static Opnd gen_read (Unit* u, Loc l) {
    Opnd d = opnd_tmp(u);
    if (l.e->tag == E_ACC) {
        out(u, "a_cpy(&%s, %s);", d.c, var_c(u,l.e->var));
    } else if (l.e->tag == E_PUB) {
        out(u, "{");
        out(u, "    AVal* pub;");
        out(u, "    if ((st = a_pub(&pub)) != A_OK) goto CL_%d;", u->cleanup);
        out(u, "    a_cpy(&%s, *pub);", d.c);
        out(u, "}");
    } else {
        switch (l.e->u.index.kind) {
            case IDX_EXPR:
                out_try(u, "a_index(&%s, %s, %s)", d.c, l.t.c, l.k.c);
                break;
            case IDX_LAST:
                out_try(u, "a_index_last(&%s, %s)", d.c, l.t.c);
                break;
            case IDX_NEXT:
                // the next position is always empty
                break;
        }
    }
    return d;
}

// set x += e: the destination is evaluated once.
// `&&=` and `||=` only evaluate and assign if needed.
static int is_place (Expr* dst);

static void gen_set_op (Unit* u, Expr* e) {
    const char* op = e->u.set.op;
    Expr* dst = e->u.set.dsts.v[0];
    if (is_place(dst) && op_scalar(op) && is_quiet(e->u.set.src)) {
        // set x += e: computed in place, since `e` cannot assign `x`
        const char* x = var_c(u, dst->var);
        Opnd v = gen_now(u, e->u.set.src);
        gen_binop_to(u, e->tk, op, opnd_c(x), v, x);
        return;
    }
    Loc l = gen_loc(u, dst);
    Opnd cur = gen_read(u, l);

    if (!strcmp(op,"&&") || !strcmp(op,"||")) {
        out(u, "if (%sa_truthy(%s)) {", (op[0] == '&') ? "" : "!", cur.c);
        u->ind++;
        opnd_rel(u, cur);
        gen_assign(u, l, gen_expr(u,e->u.set.src));
        u->ind--;
        out(u, "} else {");
        u->ind++;
        opnd_rel(u, cur);
        opnd_rel(u, l.k);
        opnd_rel(u, l.t);
        u->ind--;
        out(u, "}");
        return;
    }

    Opnd v = gen_expr(u, e->u.set.src);
    if (!strcmp(op,"++")) {
        // a table is extended in place; any other value is rebound
        out(u, "if (%s.type == A_TAB) {", cur.c);
        u->ind++;
        out(u, "a_lin = %d;", lin_of(e->tk));
        out_try(u, "a_cat_set(%s, %s)", cur.c, v.c);
        opnd_rel(u, cur);
        opnd_rel(u, v);
        opnd_rel(u, l.k);
        opnd_rel(u, l.t);
        u->ind--;
        out(u, "} else {");
        u->ind++;
        if (e->u.set.fixed) {
            // a name that cannot be reassigned
            out(u, "a_lin = %d;", lin_of(e->tk));
            out_try(u, "a_error(\"invalid set : expected table or mutable "
                "variable : got %%s\", a_typename(%s))", cur.c);
            u->ind--;
            out(u, "}");
            return;
        }
        gen_assign(u, l, gen_binop(u, e->tk, op, cur, v));
        u->ind--;
        out(u, "}");
        return;
    }
    Opnd r = gen_binop(u, e->tk, op, cur, v);
    u->scalar = op_scalar(op);
    gen_assign(u, l, r);
    u->scalar = 0;
}

// A variable that an operator may write in place: not a pin, which
// owns its value.
static int is_place (Expr* dst) {
    return dst->tag==E_ACC && dst->var->kind!=V_PIN;
}

// set x = e  |  set t@k, y = e
// Tables and keys are evaluated before the values.
// set :out x = e: the place refers to the value, with no check of an
// escape and no adoption, until the enclosing block ends, when it goes
// back to nil if it still refers to it. The value outlives the block
// (it belongs to this block or to an outer one), so the place never
// refers to a finalized value; a name that reads the place and
// outlives the block refers to a finalized value, which may not be
// used.
static void gen_set_out (Unit* u, Expr* e) {
    Expr* dst = e->u.set.dsts.v[0];
    if (dst->tag==E_INDEX && dst->u.index.kind==IDX_LAST) {
        err(dst->tk, "invalid set :out : expected field or index");
    }
    if (dst->tag==E_ACC && dst->var->kind==V_PIN) {
        err(dst->tk, "invalid set :out : unexpected pin");
    }
    Loc l = gen_loc(u, dst);
    if (dst->tag!=E_INDEX && dst->tag!=E_ACC && dst->tag!=E_PUB) {
        err(dst->tk, "invalid set :out : expected variable, field, or index");
    }
    Opnd v = gen_expr(u, e->u.set.src);
    out(u, "a_lin = %d;", lin_of(e->tk));
    Defer d;
    memset(&d, 0, sizeof(d));
    d.flag  = slot_new(u);
    d.trail = -1;
    d.ov    = slot_new(u);
    out(u, "a_cpy(&L[%d], %s);", d.ov, v.c);
    if (dst->tag == E_INDEX) {
        d.out = 1;
        d.ot  = slot_new(u);
        d.ok  = slot_new(u);
        out(u, "a_cpy(&L[%d], %s);", d.ot, l.t.c);
        if (dst->u.index.kind == IDX_NEXT) {
            // t@+: the index that is next now
            out_try(u, "a_out_next(&L[%d], L[%d])", d.ok, d.ot);
        } else {
            out(u, "a_cpy(&L[%d], %s);", d.ok, l.k.c);
        }
        out_try(u, "a_out_tab(L[%d], L[%d], L[%d])", d.ot, d.ok, d.ov);
        // until the value ends, if it has an end of its own; else, until
        // this block ends
        out(u, "a_set(&L[%d], a_bool(!a_out_reg_tab(L[%d], L[%d], L[%d])));",
            d.flag, d.ot, d.ok, d.ov);
    } else if (dst->tag == E_PUB) {
        // the place is in the running task, which the entry holds
        d.out  = 2;
        d.ot   = slot_new(u);
        d.odst = xfmt("A_TASK_OF(L[%d])->pub", d.ot);
        out_try(u, "a_out_pub(&L[%d], L[%d], &L[%d])", d.ot, d.ov, d.flag);
    } else {
        Var* x = dst->var;
        d.out  = 2;
        d.odst = var_c(u, x);
        out(u, "a_name(L[%d], %s);", d.ov, var_depth(u, x));
        out(u, "a_cpy(&%s, L[%d]);", d.odst, d.ov);
        out(u, "a_set(&L[%d], a_bool(!a_out_reg_var(&%s, %s, L[%d], %s)));",
            d.flag, d.odst, x->boxed ? var_raw(u,x) : "a_nil()", d.ov,
            var_depth(u, x));
    }
    u->defs = realloc(u->defs, (u->ndefs+1) * sizeof(Defer));
    u->defs[u->ndefs++] = d;
    opnd_rel(u, v);
    opnd_rel(u, l.k);
    opnd_rel(u, l.t);
}

static void gen_set (Unit* u, Expr* e) {
    if (e->u.set.out) {
        gen_set_out(u, e);
        return;
    }
    if (e->u.set.op != NULL) {
        gen_set_op(u, e);
        return;
    }
    int k = e->u.set.dsts.n;
    if (k==1 && is_place(e->u.set.dsts.v[0]) &&
        gen_scalar_to(u, e->u.set.src, var_c(u,e->u.set.dsts.v[0]->var)))
    {
        return;
    }
    Loc* ls = xalloc(k * sizeof(Loc));
    for (int i=0; i<k; i++) {
        ls[i] = gen_loc(u, e->u.set.dsts.v[i]);
    }
    if (k == 1) {
        // set t.x = spawn T(...)
        if (ls[0].e->tag == E_INDEX) {
            gen_born_in(u, e->u.set.src, ls[0].t.c);
        }
        Opnd v = gen_expr(u, e->u.set.src);
        out(u, "a_lin = %d;", lin_of(e->tk));
        u->scalar = expr_scalar(e->u.set.src);
        gen_assign_x(u, ls[0], v);
        u->scalar = 0;
    } else {
        Dst m = dst_multi(u);
        gen(u, e->u.set.src, m);
        out(u, "a_lin = %d;", lin_of(e->tk));
        out(u, "a_adj(%s, %d);", m.n, k);
        // all values are taken before the first assignment
        Opnd* vs = xalloc(k * sizeof(Opnd));
        for (int i=k-1; i>=0; i--) {
            vs[i] = opnd_tmp(u);
            out(u, "a_pop(&%s);", vs[i].c);
        }
        for (int i=0; i<k; i++) {
            gen_assign(u, ls[i], vs[i]);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// CONDITIONALS
///////////////////////////////////////////////////////////////////////////////

// Lambda expanded in place: binds its first parameter to `arg` and
// evaluates its body as a block.
static void gen_inline (Unit* u, Expr* p, Opnd arg, Dst d) {
    if (p->u.proto.n > 0) {
        opnd_mov(u, arg, var_c(u,p->vars[0]));
    } else {
        opnd_rel(u, arg);
    }
    // a return inside the lambda gives its value
    int end = ++u->nlabels;
    Hnd h = hnd_none();
    h.ret  = end;
    h.retd = d;
    gen_block_x(u, &p->u.proto.blk->u.block.es, d, h);
    out0(u, "LE_%d: ;", end);
}

// if, ifs, match: a chain of tests. The body of the first case that
// holds gives the value, and receives the tested value if it is a
// lambda.
static void gen_ifs (Unit* u, Expr* e, Dst d) {
    Opnd h = opnd_c(NULL);
    if (e->u.ifs.head != NULL) {
        h = gen_expr(u, e->u.ifs.head);
    }

    for (int i=0; i<e->u.ifs.n; i++) {
        Case* cs = &e->u.ifs.cases[i];
        Opnd c;
        if (h.c == NULL) {
            // ifs: the condition itself
            c = (cs->cnd == NULL) ? opnd_c("a_bool(1)") :
                                    gen_expr(u, cs->cnd);
        } else if (cs->test) {
            // match: lambda applied to the head
            Opnd arg = opnd_tmp(u);
            out(u, "a_cpy(&%s, %s);", arg.c, h.c);
            c = opnd_tmp(u);
            gen_inline(u, cs->cnd, arg, dst_slot(c.c));
        } else {
            // match: the head, if it is the case value (or else)
            c = opnd_tmp(u);
            const char* v = xfmt("(a_truthy(%s) ? %s : a_bool(1))", h.c, h.c);
            if (cs->cnd == NULL) {
                out(u, "a_cpy(&%s, %s);", c.c, v);
            } else {
                Opnd x = gen_expr(u, cs->cnd);
                out(u, "if (a_is(%s, %s)) {", h.c, x.c);
                out(u, "    a_cpy(&%s, %s);", c.c, v);
                out(u, "}");
                opnd_rel(u, x);
            }
        }

        out(u, "if (a_truthy(%s)) {", c.c);
        u->ind++;
        if (cs->body->tag==E_PROTO && cs->body->u.proto.dots) {
            // a variadic lambda is called with the tested value
            Opnd f = gen_expr(u, cs->body);
            out(u, "{");
            u->ind++;
            out(u, "int b = a_top;");
            out(u, "a_pushc(%s);", c.c);
            out_try(u, "a_call(%s, b)", f.c);
            switch (d.kind) {
                case D_NONE:
                    out(u, "a_settop(b);");
                    break;
                case D_SLOT:
                    out(u, "a_ret1(&%s, b);", d.slot);
                    break;
                case D_MULTI:
                    out(u, "%s = a_top - b;", d.n);
                    break;
            }
            u->ind--;
            out(u, "}");
            opnd_rel(u, f);
            opnd_rel(u, c);
        } else if (cs->body->tag == E_PROTO) {
            gen_inline(u, cs->body, c, d);
        } else {
            opnd_rel(u, c);
            gen_block(u, &cs->body->u.block.es, d);
        }
        u->ind--;
        out(u, "} else {");
        u->ind++;
        opnd_rel(u, c);
    }
    // no case holds
    store_void(u, d);
    for (int i=0; i<e->u.ifs.n; i++) {
        u->ind--;
        out(u, "}");
    }

    if (h.c != NULL) {
        opnd_rel(u, h);
    }
}

///////////////////////////////////////////////////////////////////////////////
// LOOPS
///////////////////////////////////////////////////////////////////////////////

// loop {...}  |  loop i {...}  |  loop k,v in e {...}
// Its value is nothing, or the values of a break.
static void gen_loop (Unit* u, Expr* e, Dst d) {
    int iter = (e->u.loop.n > 0) || (e->u.loop.itr != NULL);

    // iterator, limit of a range, cursor
    Opnd it  = opnd_c(NULL);
    Opnd lim = opnd_c(NULL);
    Opnd stp = opnd_c(NULL);
    Opnd pos = opnd_c(NULL);
    const char* k = NULL;
    const char* v = NULL;
    if (iter) {
        it  = opnd_tmp(u);
        lim = opnd_tmp(u);
        stp = opnd_tmp(u);
        pos = opnd_tmp(u);
        if (e->u.loop.itr != NULL) {
            // loop i in (a,b) gives two values; (a,b,s), three
            Dst m = dst_multi(u);
            gen(u, e->u.loop.itr, m);
            out(u, "a_adj(%s, 3);", m.n);
            out(u, "a_pop(&%s);", stp.c);
            out(u, "a_pop(&%s);", lim.c);
            out(u, "a_pop(&%s);", it.c);
            // `loop i` counts with no end: `loop i in nil` is not its
            // other spelling (atmos-iter#6b)
            out_try(u, "((%s.type == A_NIL) ? a_error(\"invalid loop : unexpected nil\") : A_OK)", it.c);
        }
        k = (e->u.loop.n > 0) ? var_c(u,e->vars[0]) : opnd_tmp(u).c;
        v = (e->u.loop.n > 1) ? var_c(u,e->vars[1]) : opnd_tmp(u).c;
    }

    int end = ++u->nlabels;
    int nxt = ++u->nlabels;
    out(u, "while (1) {");
    u->ind++;
    if (iter) {
        out_try(u, "a_next(%s, %s, %s, &%s, &%s, &%s)", it.c, lim.c, stp.c, pos.c, k, v);
        out(u, "if (%s.type == A_NIL) break;", k);
        for (int i=0; i<e->u.loop.n && i<2; i++) {
            out(u, "a_name(%s, %s);", var_c(u,e->vars[i]),
                var_depth(u,e->vars[i]));
        }
    }
    Hnd h = hnd_none();
    h.brk  = end;
    h.brkd = d;
    h.skp  = nxt;
    gen_block_x(u, &e->u.loop.blk->u.block.es, dst_none(), h);
    // a skip arrives here, at the end of the step
    out0(u, "LE_%d: ;", nxt);
    u->ind--;
    out(u, "}");
    // the loop ends without a break
    store_void(u, d);
    out0(u, "LE_%d: ;", end);

    if (iter) {
        opnd_rel(u, it);
        opnd_rel(u, lim);
        opnd_rel(u, pos);
    }
}

// break(...)  until(c,...)  while(c,...)
static void gen_break (Unit* u, Expr* e) {
    Special sp = e->u.call.sp;
    List args = { 0, NULL };
    flatten(&args, &e->u.call.es);

    // condition, then the values of the break
    Opnd c = opnd_c(NULL);
    List vals = args;
    if (sp != SP_BREAK) {
        if (args.n == 0) {
            err(e->tk, "expected condition");
        }
        c = gen_expr(u, args.v[0]);
        vals.n--;
        vals.v++;
    }
    Dst m = dst_multi(u);
    gen_args(u, &vals, m.n);

    if (sp == SP_UNTIL) {
        // until(c, v, ...) breaks with `v, ...` if `v` holds, else `c`
        out(u, "if (a_truthy(%s)) {", c.c);
        out(u, "    if (%s>0 && a_truthy(a_stk[a_top-%s])) {", m.n, m.n);
        out(u, "        a_pay_pull(%s);", m.n);
        out(u, "    } else {");
        out(u, "        a_settop(a_top - %s);", m.n);
        out(u, "        a_pay_clear();");
        out(u, "        a_pay_addc(%s);", c.c);
        out(u, "    }");
    } else if (sp == SP_WHILE) {
        out(u, "if (!a_truthy(%s)) {", c.c);
        out(u, "    a_pay_pull(%s);", m.n);
    } else {
        out(u, "{");
        out(u, "    a_pay_pull(%s);", m.n);
    }
    out(u, "    st = A_BREAK;");
    out(u, "    goto CL_%d;", u->cleanup);
    out(u, "}");
    if (sp != SP_BREAK) {
        out(u, "a_settop(a_top - %s);", m.n);
        opnd_rel(u, c);
    }
}

///////////////////////////////////////////////////////////////////////////////
// EXCEPTIONS
///////////////////////////////////////////////////////////////////////////////

// catch e {...}: `true` plus the values of the body, or `false` plus
// the values of a matching throw.
static void gen_catch (Unit* u, Expr* e, Dst d) {
    Opnd c = gen_expr(u, e->u.xcatch.cnd);
    int lc  = ++u->nlabels;
    int end = ++u->nlabels;

    Dst m = (d.kind == D_MULTI) ? dst_multi(u) : dst_none();
    Hnd h = hnd_none();
    h.err = lc;
    gen_block_x(u, &e->u.xcatch.blk->u.block.es, m, h);

    // no error
    switch (d.kind) {
        case D_NONE:
            break;
        case D_SLOT:
            out(u, "a_set(&%s, a_bool(1));", d.slot);
            break;
        case D_MULTI:
            out(u, "a_open(%s, 1);", m.n);
            out(u, "a_putc(a_top-%s-1, a_bool(1));", m.n);
            out(u, "%s = %s + 1;", d.n, m.n);
            break;
    }
    out(u, "goto LE_%d;", end);

    // error: caught here, or propagated
    out0(u, "LC_%d:", lc);
    out(u, "{");
    out(u, "    int ok;");
    out(u, "    if ((st = a_catch(%s, &ok)) != A_OK) goto CL_%d;",
        c.c, u->cleanup);
    out(u, "    if (!ok) {");
    out(u, "        st = A_ERROR;");
    out(u, "        goto CL_%d;", u->cleanup);
    out(u, "    }");
    out(u, "}");
    switch (d.kind) {
        case D_NONE:
            out(u, "a_pay_clear();");
            break;
        case D_SLOT:
            out(u, "a_set(&%s, a_bool(0));", d.slot);
            out(u, "a_pay_clear();");
            break;
        case D_MULTI:
            out(u, "a_push(a_bool(0));");
            out(u, "%s = 1 + a_pay_push();", d.n);
            break;
    }
    out0(u, "LE_%d: ;", end);
    opnd_rel(u, c);
}

///////////////////////////////////////////////////////////////////////////////
// CALLS
///////////////////////////////////////////////////////////////////////////////

static void gen_call (Unit* u, Expr* e, Dst d) {
    switch (e->u.call.sp) {
        case SP_NONE:
            break;
        case SP_DO: {
            // do(...): evaluates and discards
            Dst m = dst_multi(u);
            gen_args(u, &e->u.call.es, m.n);
            out(u, "a_settop(a_top - %s);", m.n);
            store_void(u, d);
            return;
        }
        case SP_BREAK:
        case SP_UNTIL:
        case SP_WHILE:
            gen_break(u, e);
            store_void(u, d);
            return;
        case SP_SKIP:
            // skip: the step ends, with no values
            out(u, "a_pay_clear();");
            out(u, "st = A_SKIP;");
            out(u, "goto CL_%d;", u->cleanup);
            store_void(u, d);
            return;
        case SP_RETURN: {
            // the values travel in the payload up to the function
            Dst m = dst_multi(u);
            gen_args(u, &e->u.call.es, m.n);
            out(u, "a_pay_pull(%s);", m.n);
            out(u, "st = A_RETURN;");
            out(u, "goto CL_%d;", u->cleanup);
            store_void(u, d);
            return;
        }
        case SP_ABORT: {
            List args = { 0, NULL };
            flatten(&args, &e->u.call.es);
            if (args.n != 1) {
                err(e->tk, "invalid abort : expected single argument");
            }
            Opnd x = gen_expr(u, args.v[0]);
            out(u, "a_lin = %d;", lin_of(e->tk));
            out_try(u, "a_abort(%s)", x.c);
            opnd_rel(u, x);
            store_void(u, d);
            return;
        }
        case SP_ESCAPE: {
            Dst m = dst_multi(u);
            gen_args(u, &e->u.call.es, m.n);
            out(u, "a_pay_pull(%s);", m.n);
            out(u, "st = a_escape();");
            out(u, "goto CL_%d;", u->cleanup);
            store_void(u, d);
            return;
        }
        case SP_THROW: {
            Dst m = dst_multi(u);
            gen_args(u, &e->u.call.es, m.n);
            out(u, "a_pay_pull(%s);", m.n);
            out(u, "st = a_throw();");
            out(u, "goto CL_%d;", u->cleanup);
            store_void(u, d);
            return;
        }
        default:
            err(e->tk, "statement not implemented");
    }

    Opnd f;
    Dst  m = dst_multi(u);
    f = gen_expr(u, e->u.call.f);
    gen_args(u, &e->u.call.es, m.n);

    out(u, "{");
    u->ind++;
    out(u, "int b = a_top - %s;", m.n);
    out(u, "a_lin = %d;", lin_of(e->tk));
    out_try(u, "a_call(%s, b)", f.c);
    switch (d.kind) {
        case D_NONE:
            out(u, "a_settop(b);");
            break;
        case D_SLOT:
            out(u, "a_ret1(&%s, b);", d.slot);
            break;
        case D_MULTI:
            out(u, "%s = a_top - b;", d.n);
            break;
    }
    u->ind--;
    out(u, "}");
    opnd_rel(u, f);
}

// (e1, e2, ...): all values of the list, or only the first one.
static void gen_list (Unit* u, Expr* e, Dst d) {
    Dst m = (d.kind == D_MULTI) ? d : dst_multi(u);
    gen_args(u, &e->u.list.es, m.n);
    switch (d.kind) {
        case D_NONE:
            out(u, "a_settop(a_top - %s);", m.n);
            break;
        case D_SLOT:
            out(u, "a_ret1(&%s, a_top - %s);", d.slot, m.n);
            break;
        case D_MULTI:
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

// Declarations and definitions of the generated C functions: `DECLS`
// and `FUNCS`, of the module being compiled.

// Appends the common prologue of a function, its body, and its
// cleanup label.
//
// A function keeps its slots on the C stack. A task keeps them in its
// frame, and starts by jumping to its resumption point.
static void unit_emit (Unit* u, Buf* c) {
    int n = (u->nslots > 0) ? u->nslots : 1;
    // depth of the body block
    if (u->task) {
        buf_add(c, "    AVal* L = t->L;\n");
        buf_add(c, "    AFunc* self A_UNUSED = t->proto;\n");
        buf_add(c, "    int D = t->depth;\n");
    } else {
        buf_add(c, xfmt("    AVal L[%d];\n", n));
        buf_add(c, xfmt("    a_nils(L, %d);\n", n));
        buf_add(c, "    int D = a_depth + 1;\n");
    }
    if (u->proto->self != NULL) {
        buf_add(c, "    AVal SELF = a_obj(&self->hdr);\n");
        buf_add(c, "    (void) SELF;\n");
    }
    buf_add(c, "    AStatus st = A_OK;\n");
    buf_add(c, "    int base = a_top;\n");
    for (int i=0; i<u->ncnts; i++) {
        buf_add(c, xfmt("    int n%d A_UNUSED = 0;\n", i));
    }
    if (u->task) {
        buf_add(c, "    switch (tr->pc) {\n");
        for (int i=1; i<=u->npcs; i++) {
            buf_add(c, xfmt("        case %d: goto AW_%d;\n", i, i));
        }
        buf_add(c, "    }\n");
    }
    buf_add(c, u->body.s ? u->body.s : "");
    buf_add(c, "    goto CL_0;\n");
    buf_add(c, "CL_0:\n");
    if (!u->task) {
        buf_add(c, xfmt("    a_clrs(L, %d);\n", n));
    }
}

// Adds a trail to the task of a unit. Returns its position.
static int trail_new (Unit* u, const char* fn, int up, int level) {
    Unit* r = (u->root != NULL) ? u->root : u;
    r->trs = realloc(r->trs, (r->ntrs+1) * sizeof(Trail));
    r->trs[r->ntrs].fn    = fn;
    r->trs[r->ntrs].up    = up;
    r->trs[r->ntrs].level = level;
    return r->ntrs++;
}

// Compiles a task prototype (or the main chunk) to a resume function
// and its static description. Returns the name of the description.
//
// Parameters take the first slots of the frame, then comes a table
// with the extra arguments, if variadic. The value of the body, or of
// a return, becomes the result of the task.
static const char* gen_task (Proto* p, Var** pars, int npar, List* body,
                             int lin, int flin)
{
    Unit u;
    memset(&u, 0, sizeof(u));
    u.proto = p;
    u.npar  = npar;
    u.task  = 1;
    u.level = -1;
    for (int i=0; i<npar; i++) {
        pars[i]->slot  = u.nslots++;
        pars[i]->level = 0;
    }
    if (p->dots) {
        u.nslots++;
    }
    u.orph = 1;
    const char* name = xfmt("atm_T%d", p->lid);
    trail_new(&u, name, -1, 0);
    // the parameters are names of the body of the task
    if (npar > 0) {
        out(&u, "a_lin = %d;", flin);
    }
    for (int i=0; i<npar; i++) {
        out(&u, "a_name_arg(L[%d], D);", pars[i]->slot);
    }
    // the value of the main chunk goes nowhere
    gen_block(&u, body, (p==PROTO_MAIN || p->module) ? dst_none() :
        dst_slot("t->ret"));

    const char* sig = xfmt(
        "static AStatus %s (ATask* t, ATrail* tr, AKind k)", name);
    buf_add(&DECLS, xfmt("%s;\n", sig));

    Buf* c = &FUNCS;
    buf_add(c, xfmt("// line %d\n%s {\n", lin, sig));
    unit_emit(&u, c);
    buf_add(c, "    if (st == A_RETURN) {\n");
    buf_add(c, "        st = A_OK;\n");
    buf_add(c, "        a_pay_take(&t->ret, 0);\n");
    buf_add(c, "        a_pay_clear();\n");
    buf_add(c, "    }\n");
    buf_add(c, "    return st;\n");
    buf_add(c, "}\n\n");
    // the trails, with their tree: siblings in source order
    buf_add(c, xfmt("static const ATrailD atm_P%d_T[] = {\n", p->lid));
    for (int i=0; i<u.ntrs; i++) {
        int kid  = -1;
        int next = -1;
        for (int j=u.ntrs-1; j>i; j--) {
            if (u.trs[j].up == i) {
                kid = j;
            }
            if (u.trs[j].up == u.trs[i].up) {
                next = j;
            }
        }
        buf_add(c, xfmt("    { %s, %d, %d, %d, %d },\n", u.trs[i].fn,
            u.trs[i].up, kid, (i == 0) ? -1 : next, u.trs[i].level));
    }
    buf_add(c, "};\n");
    // the body of a module is started by another unit
    const char* desc = p->module ? xfmt("atm_M_%s", MODS[p->mod].name) :
        xfmt("atm_P%d", p->lid);
    buf_add(c, xfmt(
        "%sconst AProto %s = { %d, %d, %d, %d, atm_P%d_T };\n\n",
        p->module ? "" : "static ", desc,
        (u.nslots > 0) ? u.nslots : 1, npar, p->dots, u.ntrs, p->lid));
    return desc;
}

// Compiles a prototype to a C function. Returns its name.
//
// The values of the body, or of a return, are left on the stack.
static const char* gen_func (Expr* e) {
    Unit u;
    memset(&u, 0, sizeof(u));
    u.proto = e->proto;
    u.npar  = e->u.proto.n;
    u.level = -1;

    // parameters are immutable copies of the arguments, names of the
    // body of the function
    if (e->u.proto.n > 0) {
        out(&u, "a_lin = %d;", lin_of(e->tk));
    }
    for (int i=0; i<e->u.proto.n; i++) {
        out(&u, "if (argc > %d) a_cpy(&%s, argv[%d]);",
            i, var_c(&u,e->vars[i]), i);
        out(&u, "a_name(%s, D);", var_c(&u,e->vars[i]));
    }
    gen_block(&u, &e->u.proto.blk->u.block.es, dst_multi(&u));

    const char* name = xfmt("atm_F%d", e->proto->lid);
    const char* sig = xfmt(
        "static AStatus %s (AFunc* self, int argc, AVal* argv)", name);
    buf_add(&DECLS, xfmt("%s;\n", sig));

    Buf* c = &FUNCS;
    buf_add(c, xfmt("// line %d\n%s {\n", e->tk.lin, sig));
    unit_emit(&u, c);
    buf_add(c, "    if (st == A_RETURN) {\n");
    buf_add(c, "        st = A_OK;\n");
    buf_add(c, "        a_pay_push();\n");
    buf_add(c, "    }\n");
    buf_add(c, "    return st;\n");
    buf_add(c, "}\n\n");
    return name;
}

// func (...) {...}: a closure with copies of the captured variables
// (or their boxes).
static Opnd gen_proto (Unit* u, Expr* e) {
    Proto* p = e->proto;
    Opnd d = opnd_tmp(u);
    if (e->u.proto.task) {
        const char* name = gen_task(p, e->vars, e->u.proto.n,
            &e->u.proto.blk->u.block.es, e->tk.lin, lin_of(e->tk));
        out(u, "a_set(&%s, a_task(&%s, %d));", d.c, name, p->ncaps);
    } else {
        const char* name = gen_func(e);
        out(u, "a_set(&%s, a_func(%s, %d));", d.c, name, p->ncaps);
    }
    for (int i=0; i<p->ncaps; i++) {
        out(u, "a_cpy(&A_FUNC_OF(%s)->caps[%d], %s);",
            d.c, i, var_raw(u,p->caps[i]));
    }
    if (p->ncaps > 0) {
        out(u, "a_func_scope(%s);", d.c);
    }
    return d;
}

// Native declarations, at file scope, in order. Every unit has those
// of a `:prelude.h`, as a header; only the unit of its module has those
// of a `:prelude.c`, as what a program defines once (a variable, a
// function that is not `static`).
typedef struct Pre {
    int         mod;
    int         once;
    const char* c;
} Pre;
static Pre* PRES  = NULL;
static int  NPRES = 0;

// `C code`: statements, declarations, or an expression converted
// according to its tag. `$x` is the variable `x`, as an `AVal`.
static void gen_nat (Unit* u, Expr* e, Dst d) {
    const char* s;
    NatKind kind = nat_kind(e->tk.str, &s);

    Buf code = { NULL, 0, 0 };
    buf_add(&code, "");
    int n = 0;
    while (*s != '\0') {
        int len = (s[0] == '$') ? nat_id(s+1) : 0;
        if (len > 0) {
            buf_add(&code, var_c(u, e->vars[n++]));
            s += 1 + len;
        } else {
            char c[2] = { *s++, '\0' };
            buf_add(&code, c);
        }
    }
    const char* line = xfmt("#line %d %s\n", e->tk.lin, file_q(e->tk));

    if (kind==NAT_PRE || kind==NAT_ONCE) {
        PRES = realloc(PRES, (NPRES+1) * sizeof(Pre));
        PRES[NPRES].mod  = CUR;
        PRES[NPRES].once = (kind == NAT_ONCE);
        PRES[NPRES].c    = xfmt("%s%s\n", line, code.s);
        NPRES++;
        store_void(u, d);
        return;
    }
    if (kind == NAT_STMT) {
        buf_add(&u->body, line);
        buf_add(&u->body, code.s);
        buf_add(&u->body, "\n;\n");
        // the code may fail: `st = a_error(...)`
        out(u, "if (st != A_OK) goto CL_%d;", u->cleanup);
        store_void(u, d);
        return;
    }

    static const char* CONV[] = {
        NULL, NULL,
        "a_int((AInt)(", "a_num((double)(", "a_bool(0 != (",
        "a_strn((", "((", "a_number(("
    };
    Opnd t = opnd_tmp(u);
    buf_add(&u->body, line);
    buf_add(&u->body, xfmt("a_set(&%s, %s", t.c, CONV[kind]));
    buf_add(&u->body, code.s);
    buf_add(&u->body, ")));\n");
    out(u, "if (st != A_OK) goto CL_%d;", u->cleanup);
    store(u, t, d);
}

// `...`: the extra arguments of the function. A task keeps them in a
// table in its frame.
static void gen_dots (Unit* u, Dst d) {
    if (u->task) {
        switch (d.kind) {
            case D_NONE:
                break;
            case D_SLOT:
                out(u, "a_cpy(&%s, a_tab_get(A_TAB_OF(L[%d]), a_int(0)));",
                    d.slot, u->npar);
                break;
            case D_MULTI:
                out(u, "%s = a_args_unpack(L[%d]);", d.n, u->npar);
                break;
        }
        return;
    }
    switch (d.kind) {
        case D_NONE:
            break;
        case D_SLOT:
            out(u, "a_cpy(&%s, (argc > %d) ? argv[%d] : a_nil());",
                d.slot, u->npar, u->npar);
            break;
        case D_MULTI:
            out(u, "%s = a_dots(argc, argv, %d);", d.n, u->npar);
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// TASKS
///////////////////////////////////////////////////////////////////////////////

// spawn T(...)  |  spawn @ts T(...)
// As a statement, the task is pinned to the enclosing block.
// Otherwise it flows out, and the receiver must pin it.
static Opnd* gen_filters (Unit* u, Patt** fs, int nf, int* n);

// The constants of a context value: its tag and its two keys.
static const char* ctx_keys (void) {
    return xfmt("atm_K[%d], atm_K[%d], atm_K[%d]", konst("Context",7),
        konst("enter",5), konst("leave",5));
}

// in [<P>] X, ...: binds what the next spawn or pool creates.
static void gen_bind (Unit* u, Expr* ctx, Opnd x, Patt* cpat) {
    if (ctx == NULL) {
        return;
    }
    if (cpat != NULL) {
        int n;
        Opnd* os = gen_filters(u, &cpat, 1, &n);
        out(u, "    st = a_ctx_bind(%s, %s, fs, 1);", x.c, ctx_keys());
        out(u, "}");
        out(u, "if (st != A_OK) goto CL_%d;", u->cleanup);
        for (int i=0; i<n; i++) {
            opnd_rel(u, os[i]);
        }
    } else {
        out_try(u, "a_ctx_bind(%s, %s, NULL, 0)", x.c, ctx_keys());
    }
}

static void gen_spawn (Unit* u, Expr* e, Dst d) {
    Expr* ctx = e->u.spawn.ctx;
    Opnd cx = (ctx == NULL) ? opnd_c(NULL) : gen_expr(u, ctx);
    Opnd f = gen_expr(u, e->u.spawn.f);
    Opnd pool = opnd_c("a_nil()");
    if (e->u.spawn.pool != NULL) {
        pool = gen_expr(u, e->u.spawn.pool);
    }
    Dst m = dst_multi(u);
    gen_args(u, &e->u.spawn.args, m.n);

    Opnd t = opnd_tmp(u);
    out(u, "a_lin = %d;", lin_of(e->tk));
    // a statement, or the value of a `pin`: the block of the spawn
    // pins the task from birth
    int stmt = (d.kind==D_NONE && e->u.spawn.pool==NULL);
    int born = (u->born == e);
    const char* holder = born ? u->bornh : NULL;
    if (born) {
        u->born  = NULL;
        u->bornh = NULL;
    }
    if (born && holder!=NULL) {
        // the value of a field: pinned from birth with its table
        gen_bind(u, ctx, cx, e->u.spawn.cpat);
        out_try(u, "a_spawn_in(&%s, %s, a_top-%s, %s)", t.c, f.c, m.n,
            holder);
    } else if (stmt || born) {
        gen_bind(u, ctx, cx, e->u.spawn.cpat);
        out_try(u, "a_spawn_pin(&%s, %s, a_top-%s)", t.c, f.c, m.n);
    } else {
        if (e->u.spawn.pool != NULL) {
            // `spawn @p`: the pool must exist (a nil `p` is an error)
            out_try(u, "a_spawn_pool(%s)", pool.c);
        }
        gen_bind(u, ctx, cx, e->u.spawn.cpat);
        out_try(u, "a_spawn(&%s, %s, %s, a_top-%s)", t.c, f.c, pool.c, m.n);
    }
    if (ctx != NULL) {
        opnd_rel(u, cx);
    }
    opnd_rel(u, f);
    opnd_rel(u, pool);

    if (stmt) {
        gen_pin_end(u, t.c, 1);
    } else {
        store(u, t, d);
    }
}

// Compiles a transparent block as trail `id` of its task: a resume
// function that shares the slots of the task.
//
// As a branch of a `par :any`, its values are raised as a join for
// the par identified by slot `key`. As a branch of a `watching`
// (`flag` is 0 or 1, otherwise -1), the boolean `flag` goes before the
// values. Otherwise, its first value is the result of the block, which
// also goes to `ret`, if given. Any other exit crosses to the parent
// unchanged.
static void gen_tra (Unit* up, int id, List* body, ParMode mode, int key,
                     const char* ret, int lin, DKind want, int flag)
{
    Unit u;
    memset(&u, 0, sizeof(u));
    u.proto = up->proto;
    u.npar  = up->npar;
    u.task  = 1;
    u.root  = (up->root != NULL) ? up->root : up;
    u.trail = id;
    // a transparent block is a nested block of its parent
    u.level = up->level;
    u.orph  = 1;

    if (mode == PAR_ANY) {
        // only the values that the destination of the par takes: the
        // others are not given to it, and need not leave the block
        if (want == D_MULTI) {
            Dst m = dst_multi(&u);
            gen_block(&u, body, m);
            if (flag >= 0) {
                out(&u, "a_open(%s, 1);", m.n);
                out(&u, "a_putc(a_top-%s-1, a_bool(%d));", m.n, flag);
                out(&u, "a_pay_pull(%s + 1);", m.n);
            } else {
                out(&u, "a_pay_pull(%s);", m.n);
            }
        } else if (want==D_SLOT && flag>=0) {
            // the single value taken is the boolean
            gen_block(&u, body, dst_none());
            out(&u, "a_push(a_bool(%d));", flag);
            out(&u, "a_pay_pull(1);");
        } else if (want == D_SLOT) {
            Opnd r = opnd_tmp(&u);
            gen_block(&u, body, dst_slot(r.c));
            out(&u, "a_pushc(%s);", r.c);
            opnd_rel(&u, r);
            out(&u, "a_pay_pull(1);");
        } else {
            gen_block(&u, body, dst_none());
            out(&u, "a_pay_pull(0);");
        }
        out(&u, "a_join_key = &L[%d];", key);
        out(&u, "st = A_JOIN;");
    } else {
        // its first value travels in the payload, to the event of its
        // termination
        Opnd r = opnd_tmp(&u);
        gen_block(&u, body, dst_slot(r.c));
        if (ret != NULL) {
            out(&u, "a_cpy(&%s, %s);", ret, r.c);
        }
        out(&u, "a_pay_clear();");
        out(&u, "a_pay_addc(%s);", r.c);
        opnd_rel(&u, r);
    }

    const char* name = u.root->trs[id].fn;
    const char* sig = xfmt(
        "static AStatus %s (ATask* t, ATrail* tr, AKind k)", name);
    buf_add(&DECLS, xfmt("%s;\n", sig));

    Buf* c = &FUNCS;
    buf_add(c, xfmt("// line %d\n%s {\n", lin, sig));
    unit_emit(&u, c);
    buf_add(c, "    return st;\n");
    buf_add(c, "}\n\n");
}

// A new trail for a transparent block that starts in the current
// block, which must abort it on exit.
static int gen_trail (Unit* u) {
    Unit* r = (u->root != NULL) ? u->root : u;
    const char* fn = xfmt("atm_T%d_%d", u->proto->lid, r->ntrs);
    return trail_new(u, fn, u->trail, u->level + 1);
}

// Starts a transparent block as the trail `id`.
static void gen_spawn_tra (Unit* u, int id, Expr* blk, ParMode mode, int key,
                           const char* ret, DKind want, int flag)
{
    gen_tra(u, id, &blk->u.block.es, mode, key, ret, blk->tk.lin, want, flag);
    out(u, "a_lin = %d;", lin_of(blk->tk));
    out_try(u, "a_trail_spawn(t, %d)", id);
}

static void gen_suspend (Unit* u, Dst d);

// par {...} with {...}: never rejoins
// par :any: rejoins when a branch terminates, with its values
// par :all: rejoins when all terminate, with the first value of each
static void gen_par (Unit* u, Expr* e, Dst d) {
    int k = e->u.par.blks.n;
    ParMode mode = e->u.par.mode;
    Hnd h = hnd_none();

    // the results outlive the block of the branches
    Opnd* rs = xalloc(k * sizeof(Opnd));
    if (mode == PAR_ALL) {
        for (int i=0; i<k; i++) {
            rs[i] = opnd_tmp(u);
        }
    }
    if (mode == PAR_ANY) {
        h.join  = ++u->nlabels;
        h.joink = slot_new(u);
        h.joind = d;
    }

    Blk b = blk_open(u);
    // the branches take consecutive trails; on exit, they are aborted
    // in spawn order
    int lo = 0;
    for (int i=0; i<k; i++) {
        int id = gen_trail(u);
        lo = (i == 0) ? id : lo;
    }
    for (int i=k-1; i>=0; i--) {
        gen_trail_end(u, lo+i);
    }
    for (int i=0; i<k; i++) {
        gen_spawn_tra(u, lo+i, e->u.par.blks.v[i], mode, h.joink,
            (mode == PAR_ALL) ? rs[i].c : NULL, d.kind,
            e->u.par.watch ? (i != 0) : -1);
    }
    if (mode == PAR_ALL) {
        out(u, "st = a_await_trails(tr, %d, %d);", lo, k);
    } else {
        out(u, "st = a_await_val(tr, a_bool(0));");
    }
    gen_suspend(u, dst_none());
    blk_close(u, b, dst_none(), h);

    if (mode == PAR_ALL) {
        switch (d.kind) {
            case D_NONE:
                break;
            case D_SLOT:
                out(u, "a_cpy(&%s, %s);", d.slot, rs[0].c);
                break;
            case D_MULTI:
                for (int i=0; i<k; i++) {
                    out(u, "a_pushc(%s);", rs[i].c);
                }
                out(u, "%s = %d;", d.n, k);
                break;
        }
        for (int i=0; i<k; i++) {
            opnd_rel(u, rs[i]);
        }
    } else {
        store_void(u, d);
    }
    if (mode == PAR_ANY) {
        out0(u, "LE_%d: ;", h.join);
    }
}

// Suspends the task until it is resumed, then continues with the
// status of the resumption. The results arrive in the payload.
static void gen_suspend (Unit* u, Dst d) {
    int pc = ++u->npcs;
    out(u, "if (st == A_AWAIT) {");
    out(u, "    tr->pc = %d;", pc);
    out(u, "    return A_AWAIT;");
    out0(u, "AW_%d:", pc);
    out(u, "    base = a_top;");
    out(u, "    a_lin = %d;", u->lin);
    out(u, "    a_depth = D + %d;", u->level);
    out(u, "    st = a_resumed(tr, k);");
    out(u, "}");
    out(u, "if (st != A_OK) goto CL_%d;", u->cleanup);
    store_pay(u, d);
}

// A pattern is eager: its values, task prototypes, arguments, and
// predicates are evaluated once, in order, before the await starts.
static void patt_eval (Unit* u, Patt* p, Opnd* os, int* n) {
    switch (p->kind) {
        case PT_VAL:
            os[(*n)++] = gen_expr(u, p->e);
            break;
        case PT_SPAWN: {
            os[(*n)++] = gen_expr(u, p->e);
            // the arguments wait in a table
            Opnd t = opnd_tmp(u);
            out(u, "a_set(&%s, a_tab());", t.c);
            Dst m = dst_multi(u);
            gen_args(u, &p->args, m.n);
            out(u, "a_args_pack(%s, %s);", t.c, m.n);
            os[(*n)++] = t;
            break;
        }
        case PT_ANY:
        case PT_ALL:
            os[(*n)++] = gen_expr(u, p->e);
            break;
        case PT_UNTIL:
        case PT_WHILE:
            if (p->a != NULL) {
                patt_eval(u, p->a, os, n);
            }
            os[(*n)++] = gen_expr(u, p->e);
            break;
        case PT_NOT:
            patt_eval(u, p->a, os, n);
            break;
        case PT_AND:
        case PT_OR:
            patt_eval(u, p->a, os, n);
            patt_eval(u, p->b, os, n);
            break;
        case PT_CNT:
            os[(*n)++] = gen_expr(u, p->e);
            patt_eval(u, p->a, os, n);
            break;
    }
}

// Builds the runtime pattern from the evaluated operands, as one C
// variable per node. Returns the variable of the root.
static const char* patt_build (Unit* u, Patt* p, Opnd* os, int* n, int* k) {
    const char* a = NULL;
    const char* b = NULL;
    const char* c = NULL;
    switch (p->kind) {
        case PT_VAL:
            c = xfmt("a_pt_val(%s)", os[(*n)++].c);
            break;
        case PT_SPAWN:
            c = xfmt("a_pt_spawn(%s, %s)", os[*n].c, os[*n+1].c);
            *n += 2;
            break;
        case PT_ANY:
        case PT_ALL:
            c = xfmt("a_pt_pool(%d, %s)", p->kind==PT_ALL, os[(*n)++].c);
            break;
        case PT_UNTIL:
        case PT_WHILE:
            a = (p->a != NULL) ? patt_build(u, p->a, os, n, k) : "NULL";
            c = xfmt("a_pt_pred(%d, %s, %s)", p->kind==PT_UNTIL, a,
                os[(*n)++].c);
            break;
        case PT_NOT:
            a = patt_build(u, p->a, os, n, k);
            c = xfmt("a_pt_not(%s)", a);
            break;
        case PT_AND:
        case PT_OR:
            a = patt_build(u, p->a, os, n, k);
            b = patt_build(u, p->b, os, n, k);
            c = xfmt("a_pt_bin(%d, %s, %s)", p->kind==PT_AND, a, b);
            break;
        case PT_CNT: {
            const char* cnt = os[(*n)++].c;
            a = patt_build(u, p->a, os, n, k);
            c = xfmt("a_pt_cnt(%s, %s)", cnt, a);
            break;
        }
    }
    const char* v = xfmt("p%d", (*k)++);
    out(u, "    APatt* %s = %s;", v, c);
    return v;
}

static int patt_size (Patt* p) {
    if (p == NULL) {
        return 0;
    }
    return 2 + patt_size(p->a) + patt_size(p->b);
}

// Establishes an await on a pattern and suspends.
static void gen_await_patt (Unit* u, Patt* p, int lin, Dst d) {
    Opnd* os = xalloc(patt_size(p) * sizeof(Opnd));
    int n = 0;
    patt_eval(u, p, os, &n);

    out(u, "a_lin = %d;", lin);
    if (p->kind == PT_VAL) {
        // a single value needs no pattern
        out(u, "st = a_await_val(tr, %s);", os[0].c);
    } else if (p->kind==PT_CNT && p->a->kind==PT_VAL) {
        // nor does a single value that is counted
        out(u, "st = a_await_cnt(tr, %s, %s);", os[0].c, os[1].c);
    } else {
        out(u, "{");
        int i = 0;
        int k = 0;
        const char* v = patt_build(u, p, os, &i, &k);
        out(u, "    st = a_await(tr, %s);", v);
        out(u, "}");
    }
    for (int j=0; j<n; j++) {
        opnd_rel(u, os[j]);
    }
    gen_suspend(u, d);
}

// Evaluates and builds the filters of a mute, stage or solo into the C array
// `fs`, inside an open C block. Returns the operands to release.
static Opnd* gen_filters (Unit* u, Patt** fs, int nf, int* n) {
    int size = 0;
    for (int i=0; i<nf; i++) {
        size += patt_size(fs[i]);
    }
    Opnd* os = xalloc((size + 1) * sizeof(Opnd));
    *n = 0;
    for (int i=0; i<nf; i++) {
        patt_eval(u, fs[i], os, n);
    }
    out(u, "{");
    out(u, "    APatt* fs[%d];", (nf > 0) ? nf : 1);
    int j = 0;
    int k = 0;
    for (int i=0; i<nf; i++) {
        const char* v = patt_build(u, fs[i], os, &j, &k);
        out(u, "    fs[%d] = %s;", i, v);
    }
    return os;
}

// The pattern of a mute names the events silenced: what is muted
// reacts to the others, its filter is `!P`.
static Patt** mute_filt (Patt** fs, int nf) {
    if (nf == 0) {
        return fs;
    }
    Patt* p = xalloc(sizeof(Patt));
    p->kind = PT_NOT;
    p->tk   = fs[0]->tk;
    p->a    = fs[0];
    Patt** r = xalloc(sizeof(Patt*));
    r[0] = p;
    return r;
}

// mute [<P>] t(b)
static void gen_mute (Unit* u, Expr* e) {
    List args = { 0, NULL };
    flatten(&args, &e->u.mute.args);
    if (args.n != 1) {
        err(e->tk, "invalid mute : expected single argument");
    }
    Opnd t  = gen_expr(u, e->u.mute.t);
    Opnd on = gen_expr(u, args.v[0]);
    int n;
    Patt** fs = mute_filt(e->u.mute.fs, e->u.mute.nf);
    Opnd* os = gen_filters(u, fs, e->u.mute.nf, &n);
    out(u, "    a_lin = %d;", lin_of(e->tk));
    out(u, "    st = a_mute(%s, %s, fs, %d);", t.c, on.c, e->u.mute.nf);
    out(u, "}");
    out(u, "if (st != A_OK) goto CL_%d;", u->cleanup);
    for (int i=0; i<n; i++) {
        opnd_rel(u, os[i]);
    }
    opnd_rel(u, t);
    opnd_rel(u, on);
}

// mute [P] on :T {...}: the block runs as a transparent task that the
// events `:T [true]` and `:T [false]` mute and unmute.
static void gen_mute_on (Unit* u, Expr* e, Dst d) {
    Opnd r = opnd_tmp(u);
    Blk b = blk_open(u);
    int id = gen_trail(u);
    gen_trail_end(u, id);
    gen_spawn_tra(u, id, e->u.mute_on.blk, PAR_NONE, 0, r.c, D_NONE, -1);

    int n;
    Patt** fs = mute_filt(e->u.mute_on.fs, e->u.mute_on.nf);
    Opnd* os = gen_filters(u, fs, e->u.mute_on.nf, &n);
    out(u, "    a_trail_mute(t, %d, atm_K[%d], fs, %d);",
        id, konst(e->tk.str+1,e->tk.len-1), e->u.mute_on.nf);
    out(u, "}");
    for (int i=0; i<n; i++) {
        opnd_rel(u, os[i]);
    }

    out(u, "st = a_await_trails(tr, %d, 1);", id);
    gen_suspend(u, dst_none());
    blk_close(u, b, dst_none(), hnd_none());
    store(u, r, d);
}

// stage [P] {...}  |  solo [P] {...}: the block runs as a transparent
// task, registered before it starts. No pattern: every event.
static void gen_solo (Unit* u, Expr* e, Dst d) {
    Opnd r = opnd_tmp(u);
    Blk b = blk_open(u);
    int id = gen_trail(u);
    gen_trail_end(u, id);

    int n;
    Opnd* os = gen_filters(u, e->u.solo.fs, e->u.solo.nf, &n);
    out(u, "    a_lin = %d;", lin_of(e->tk));
    out(u, "    st = a_trail_solo(t, %d, %d, fs, %d);",
        id, e->u.solo.kind, e->u.solo.nf);
    out(u, "}");
    out(u, "if (st != A_OK) goto CL_%d;", u->cleanup);
    for (int i=0; i<n; i++) {
        opnd_rel(u, os[i]);
    }
    gen_spawn_tra(u, id, e->u.solo.blk, PAR_NONE, 0, r.c, D_NONE, -1);

    out(u, "st = a_await_trails(tr, %d, 1);", id);
    gen_suspend(u, dst_none());
    blk_close(u, b, dst_none(), hnd_none());
    store(u, r, d);
}

// context [<P>] {...}  |  in [<P>] X, do {...}: the block runs as a
// transparent task that is a context, before its first reaction.
static void gen_context (Unit* u, Expr* e, Dst d) {
    Expr* pair = e->u.context.pair;
    Opnd pr = (pair == NULL) ? opnd_c(NULL) : gen_expr(u, pair);
    Opnd r = opnd_tmp(u);
    Blk b = blk_open(u);
    int id = gen_trail(u);
    e->u.context.id = id;
    gen_trail_end(u, id);
    Expr* blk = e->u.context.blk;
    gen_tra(u, id, &blk->u.block.es, PAR_NONE, 0, r.c, blk->tk.lin, D_NONE, -1);
    out(u, "a_lin = %d;", lin_of(e->tk));
    if (pair != NULL) {
        out_try(u, "a_trail_context_val(t, %d, %s, %s)", id, pr.c, ctx_keys());
        opnd_rel(u, pr);
    }
    if (e->u.context.patt != NULL) {
        int n;
        Opnd* os = gen_filters(u, &e->u.context.patt, 1, &n);
        out(u, "    st = a_trail_context_filt(t, %d, fs, 1);", id);
        out(u, "}");
        out(u, "if (st != A_OK) goto CL_%d;", u->cleanup);
        for (int i=0; i<n; i++) {
            opnd_rel(u, os[i]);
        }
    }
    out(u, "a_lin = %d;", lin_of(blk->tk));
    out_try(u, "a_trail_spawn(t, %d)", id);
    out(u, "st = a_await_trails(tr, %d, 1);", id);
    gen_suspend(u, dst_none());
    blk_close(u, b, dst_none(), hnd_none());
    store(u, r, d);
}

// enter F  leave F: registered in the trail of the context; `enter`
// applies it from here. Both end with the defers of the block.
static void gen_ctxfn (Unit* u, Expr* e) {
    Expr* ctx = e->u.ctxfn.ctx;
    if (ctx->u.context.id < 0 || !u->task) {
        err(e->tk, "invalid %s : expected the block of a context",
            e->u.ctxfn.leave ? "leave" : "enter");
    }
    int id = ctx->u.context.id;
    Opnd f = gen_expr(u, e->u.ctxfn.f);
    out(u, "a_lin = %d;", lin_of(e->tk));
    out_try(u, "a_ctx_%s(t, %d, %s)", e->u.ctxfn.leave ? "leave" : "enter",
        id, f.c);
    opnd_rel(u, f);
    Defer d;
    memset(&d, 0, sizeof(d));
    d.flag  = slot_new(u);
    d.trail = -1;
    d.out   = 3;
    d.ot    = id;
    out(u, "a_set(&L[%d], a_bool(1));", d.flag);
    u->defs = realloc(u->defs, (u->ndefs+1) * sizeof(Defer));
    u->defs[u->ndefs++] = d;
}

// await P
static void gen_await (Unit* u, Expr* e, Dst d) {
    gen_await_patt(u, e->u.await.p, lin_of(e->tk), d);
}

// emit @to (e)
static void gen_emit (Unit* u, Expr* e) {
    // no target is the current task (level 0); a target that evaluates
    // to nil sends to no one
    Opnd to = opnd_c("a_int(0)");
    if (e->u.emit.to != NULL) {
        to = gen_expr(u, e->u.emit.to);
    }
    Opnd v = gen_expr(u, e->u.emit.e);
    out(u, "a_lin = %d;", lin_of(e->tk));
    out_try(u, "a_emit(%s, %s)", to.c, v.c);
    opnd_rel(u, to);
    opnd_rel(u, v);
}

///////////////////////////////////////////////////////////////////////////////

// Compiles a simple expression to an operand.
static Opnd gen_expr (Unit* u, Expr* e) {
    switch (e->tag) {
        case E_NIL:
            return opnd_c("a_nil()");
        case E_BOOL:
            return opnd_c(!strcmp(e->tk.str,"true") ?
                "a_bool(1)" : "a_bool(0)");
        case E_NUM:
            return opnd_c(gen_num(e->tk.str));
        case E_CLK: {
            // a clock is a number of units of time (microseconds or
            // milliseconds): an integer, which must fit the integer of
            // the build, or a float if it has a fraction of the unit
            double t = OPT_TIME_MS ? e->tk.clk/1000 : e->tk.clk;
            if (t >= (OPT_INT32 ? 2147483648.0 : 9223372036854775808.0)) {
                err(e->tk, "invalid clock : out of range");
            }
            if (t != floor(t)) {
                return opnd_c(xfmt("a_num(%.17g)", t));
            }
            return opnd_c(xfmt("a_int((AInt)%.0f)", t));
        }
        case E_STR: {
            // only quoted single-line literals have escapes
            int len = e->tk.len;
            const char* str = e->tk.str;
            if (e->tk.tag==TK_STR && !e->tk.multi) {
                str = decode(e->tk, &len);
            }
            return opnd_c(xfmt("atm_K[%d]", konst(str,len)));
        }
        case E_TAG:
            // a tag is the string after the colon
            return opnd_c(xfmt("atm_K[%d]", konst(e->tk.str+1,e->tk.len-1)));
        case E_ACC:
            return gen_acc(u, e);
        case E_INDEX:
            return gen_index(u, e);
        case E_TABLE:
            return gen_table(u, e);
        case E_TAGGED:
            return gen_tagged(u, e);
        case E_PARENS:
            return gen_expr(u, e->u.parens.e);
        case E_UNO:
            return gen_uno(u, e);
        case E_BIN:
            return gen_bin(u, e);
        case E_PROTO:
            return gen_proto(u, e);
        case E_ME: {
            Opnd t = opnd_tmp(u);
            out(u, "a_set(&%s, a_me());", t.c);
            return t;
        }
        case E_PUB: {
            Opnd t = opnd_tmp(u);
            out(u, "{");
            out(u, "    AVal* pub;");
            out(u, "    if ((st = a_pub(&pub)) != A_OK) goto CL_%d;",
                u->cleanup);
            out(u, "    a_cpy(&%s, *pub);", t.c);
            out(u, "}");
            return t;
        }
        case E_TASKS: {
            Opnd n = opnd_c("a_nil()");
            if (e->u.tasks.n != NULL) {
                n = gen_expr(u, e->u.tasks.n);
            }
            Opnd t = opnd_tmp(u);
            if (e->u.tasks.ctx != NULL) {
                Opnd cx = gen_expr(u, e->u.tasks.ctx);
                gen_bind(u, e->u.tasks.ctx, cx, e->u.tasks.cpat);
                opnd_rel(u, cx);
            }
            out_try(u, "a_tasks(&%s, %s)", t.c, n.c);
            opnd_rel(u, n);
            return t;
        }
        default: {
            // compound: evaluated into a temporary
            Opnd t = opnd_tmp(u);
            gen(u, e, dst_slot(t.c));
            return t;
        }
    }
}

// Compiles any expression against a destination.
static void gen (Unit* u, Expr* e, Dst d) {
    switch (e->tag) {
        case E_CALL:
            gen_call(u, e, d);
            break;
        case E_LIST:
            gen_list(u, e, d);
            break;
        case E_DCL:
            gen_dcl(u, e);
            store_void(u, d);
            break;
        case E_SET:
            gen_set(u, e);
            store_void(u, d);
            break;
        case E_BLOCK:
            gen_block(u, &e->u.block.es, d);
            break;
        case E_DO:
            gen_do(u, e, d);
            break;
        case E_IFS:
            gen_ifs(u, e, d);
            break;
        case E_LOOP:
            gen_loop(u, e, d);
            break;
        case E_WHERE:
            gen_where(u, e, d);
            break;
        case E_DOTS:
            gen_dots(u, d);
            break;
        case E_CATCH:
            gen_catch(u, e, d);
            break;
        case E_DEFER:
            gen_defer(u, e);
            store_void(u, d);
            break;
        case E_CTX_FN:
            gen_ctxfn(u, e);
            store_void(u, d);
            break;
        case E_SPAWN:
            gen_spawn(u, e, d);
            break;
        case E_AWAIT:
            gen_await(u, e, d);
            break;
        case E_EMIT:
            gen_emit(u, e);
            store_void(u, d);
            break;
        case E_IMPORT: {
            // the body of the module, in its unit
            int cur = CUR;
            CUR = e->u.import.mod;
            const char* name = gen_task(e->proto, NULL, 0,
                &e->u.import.blk->u.block.es, 1,
                lin_of(e->u.import.blk->tk));
            CUR = cur;
            COut* o = &OUTS[CUR];
            o->mods = realloc(o->mods, (o->nmods+1) * sizeof(int));
            o->mods[o->nmods++] = e->u.import.mod;
            // starts it, pinned to the enclosing block
            Opnd f = opnd_tmp(u);
            Opnd t = opnd_tmp(u);
            out(u, "a_lin = %d;", lin_of(e->tk));
            out(u, "a_set(&%s, a_task(&%s, 0));", f.c, name);
            out_try(u, "a_spawn_mod(&%s, %s, a_top)", t.c, f.c);
            opnd_rel(u, f);
            gen_pin_end(u, t.c, 1);
            store_void(u, d);
            break;
        }
        case E_SPAWN_BLK: {
            // ends with the enclosing block
            int id = gen_trail(u);
            gen_trail_end(u, id);
            gen_spawn_tra(u, id, e->u.sblk.blk, PAR_NONE, 0, NULL, D_NONE, -1);
            store_void(u, d);
            break;
        }
        case E_PAR:
            gen_par(u, e, d);
            break;
        case E_MUTE:
            gen_mute(u, e);
            store_void(u, d);
            break;
        case E_CONTEXT:
            gen_context(u, e, d);
            break;
        case E_MUTE_ON:
            gen_mute_on(u, e, d);
            break;
        case E_SOLO:
            gen_solo(u, e, d);
            break;
        case E_NAT:
            gen_nat(u, e, d);
            break;
        default:
            store(u, gen_expr(u,e), d);
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// MAIN
///////////////////////////////////////////////////////////////////////////////

// The C of the unit of a module. The program (0) also has the table
// of files, the start, and the end.
static char* unit_c (int mod, const char* main) {
    CUR = mod;
    int multi = (NMODS > 1);
    const char* name = MODS[mod].name;
    COut* o = &OUTS[mod];

    Unit u;
    memset(&u, 0, sizeof(u));
    u.proto = PROTO_MAIN;

    Var** globals;
    int nglobals = resolver_globals(mod, &globals);

    Buf c = { NULL, 0, 0 };
    buf_add(&c, "#include \"atmos.h\"\n\n");
    for (int i=0; i<NPRES; i++) {
        if (!PRES[i].once || PRES[i].mod==mod) {
            buf_add(&c, PRES[i].c);
        }
    }
    buf_add(&c, "\n");
    if (mod == 0) {
        buf_add(&c, "const char* atm_files[] = {\n");
        for (int i=0; i<NFILES; i++) {
            buf_add(&c, xfmt("    %s,\n", quote(FILES[i], strlen(FILES[i]))));
        }
        buf_add(&c, "};\n\n");
    }

    // top-level variables: its own, and those it uses of other units
    for (int i=0; i<nglobals; i++) {
        buf_add(&c, xfmt("%sAVal %s;\n", multi ? "" : "static ",
            var_raw(&u,globals[i])));
    }
    for (int i=0; i<o->nexts; i++) {
        buf_add(&c, xfmt("extern AVal %s;\n", glb_name(o->exts[i])));
    }
    for (int i=0; i<o->nmods; i++) {
        buf_add(&c, xfmt("extern const AProto atm_M_%s;\n",
            MODS[o->mods[i]].name));
    }
    buf_add(&c, "\n");

    // constants
    buf_add(&c, xfmt("static AVal atm_K[%d];\n\n", (KN > 0) ? KN : 1));
    if (multi) {
        buf_add(&c, xfmt("void atm_U_%s_init (void) {\n", name));
    } else {
        buf_add(&c, "static void atm_consts_init (void) {\n");
    }
    for (int i=0; i<KN; i++) {
        buf_add(&c, xfmt("    atm_K[%d] = a_str(%s, %d);\n",
            i, quote(KS[i].s,KS[i].len), KS[i].len));
    }
    buf_add(&c, "}\n\n");

    // functions and tasks
    buf_add(&c, DECLS.s ? DECLS.s : "");
    buf_add(&c, "\n");
    buf_add(&c, FUNCS.s ? FUNCS.s : "");

    // the end of the unit: its variables, then its constants
    if (multi) {
        buf_add(&c, xfmt("void atm_U_%s_free (int consts) {\n", name));
        buf_add(&c, "    if (consts) {\n");
        buf_add(&c, xfmt("        a_clrs(atm_K, %d);\n", KN));
        buf_add(&c, "        return;\n");
        buf_add(&c, "    }\n");
        for (int i=nglobals-1; i>=0; i--) {
            buf_add(&c, xfmt("    a_clr(&%s);\n", var_raw(&u,globals[i])));
        }
        buf_add(&c, "}\n\n");
    }
    if (mod != 0) {
        return c.s;
    }

    if (multi) {
        for (int i=1; i<NMODS; i++) {
            buf_add(&c, xfmt("void atm_U_%s_init (void);\n", MODS[i].name));
            buf_add(&c, xfmt("void atm_U_%s_free (int consts);\n",
                MODS[i].name));
        }
        buf_add(&c, "\n");
    }
    buf_add(&c, "AStatus atm_main (void) {\n");
    if (multi) {
        for (int i=0; i<NMODS; i++) {
            buf_add(&c, xfmt("    atm_U_%s_init();\n", MODS[i].name));
        }
    } else {
        buf_add(&c, "    atm_consts_init();\n");
    }
    buf_add(&c, xfmt("    return a_main(&%s);\n", main));
    buf_add(&c, "}\n\n");

    buf_add(&c, "void atm_free (void) {\n");
    if (multi) {
        for (int i=NMODS-1; i>=0; i--) {
            buf_add(&c, xfmt("    atm_U_%s_free(0);\n", MODS[i].name));
        }
    } else {
        for (int i=nglobals-1; i>=0; i--) {
            buf_add(&c, xfmt("    a_clr(&%s);\n", var_raw(&u,globals[i])));
        }
    }
    // the environments close their libraries after the variables that
    // hold their handles, and before the constants that their code uses
    buf_add(&c, "    atm_env_quit();\n");
    if (multi) {
        for (int i=NMODS-1; i>=0; i--) {
            buf_add(&c, xfmt("    atm_U_%s_free(1);\n", MODS[i].name));
        }
    } else {
        buf_add(&c, xfmt("    a_clrs(atm_K, %d);\n", KN));
    }
    buf_add(&c, "}\n");
    return c.s;
}

int coder_main (Expr* prog, CUnit** units) {
    // the main chunk is a task
    CUR = 0;
    const char* main = gen_task(PROTO_MAIN, NULL, 0, &prog->u.block.es, 1, 1);

    CUnit* us = xalloc(NMODS * sizeof(CUnit));
    for (int i=0; i<NMODS; i++) {
        us[i].name = MODS[i].name;
        us[i].c    = unit_c(i, main);
    }
    *units = us;
    return NMODS;
}
