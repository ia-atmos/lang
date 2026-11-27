// Standard library.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atmos.h"
#include "lib.h"

AVal atm_g_print;
AVal atm_g_type;
AVal atm_g_tostring;
AVal atm_g_tonumber;
AVal atm_g_assert;
AVal atm_g_next;
AVal atm_g_iter;
AVal atm_g_clause;
AVal atm_g_super;
AVal atm_g_X;
AVal atm_g_atmos;
AVal atm_g_math;

///////////////////////////////////////////////////////////////////////////////
// BASE
///////////////////////////////////////////////////////////////////////////////

static void print1 (AVal v) {
    AVal s = a_tostring(v);
    fwrite(A_STR_OF(s)->s, 1, A_STR_OF(s)->len, stdout);
    a_dec(s);
}

// print(...): shows the arguments separated by tabs.
static AStatus std_print (AFunc* self, int argc, AVal* argv) {
    (void) self;
    for (int i=0; i<argc; i++) {
        if (i > 0) {
            fputc('\t', stdout);
        }
        print1(argv[i]);
    }
    fputc('\n', stdout);
    return A_OK;
}

// type(v): the name of the type as a tag.
static AStatus std_type (AFunc* self, int argc, AVal* argv) {
    (void) self;
    a_push(a_strz(a_typename(arg(argc,argv,0))));
    return A_OK;
}

// tostring(v)
static AStatus std_tostring (AFunc* self, int argc, AVal* argv) {
    (void) self;
    a_push(a_tostring(arg(argc,argv,0)));
    return A_OK;
}

// tonumber(v): a number, or a string holding a whole number literal:
// an integer if it has neither a fraction nor an exponent.
static AStatus std_tonumber (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AVal v = arg(argc, argv, 0);
    if (a_isnum(v)) {
        a_push(v);
        return A_OK;
    }
    if (v.type != A_STR) {
        a_push(a_nil());
        return A_OK;
    }
    const char* s = A_STR_OF(v)->s;
    while (*s==' ' || *s=='\t' || *s=='\n') {
        s++;
    }
    const char* d = (*s=='-' || *s=='+') ? s+1 : s;
    int hex = (d[0]=='0' && (d[1]=='x' || d[1]=='X'));
    char* end;
    AVal ret;
    if (strpbrk(s, hex ? ".pP" : ".eEnN") == NULL) {
        // as a literal: a decimal that does not fit the integer is a
        // float, a hexadecimal wraps
        uint64_t u = strtoull(d, &end, hex ? 16 : 10);
        if (hex || u<=(uint64_t)A_INT_MAX) {
            ret = a_int((AInt)((*s == '-') ? 0 - u : u));
        } else {
            ret = a_num(A_STRTOR(s, &end));
        }
    } else {
        ret = a_num(A_STRTOR(s, &end));
    }
    int ok = (end != s) && (end != d);
    while (*end==' ' || *end=='\t' || *end=='\n') {
        end++;
    }
    ok = ok && (size_t)(end - A_STR_OF(v)->s) == A_STR_OF(v)->len;
    a_push(ok ? ret : a_nil());
    return A_OK;
}

// assert(v, msg): raises if `v` is falsy, else returns its arguments.
static AStatus std_assert (AFunc* self, int argc, AVal* argv) {
    (void) self;
    if (!a_truthy(arg(argc,argv,0))) {
        AVal msg = arg(argc, argv, 1);
        if (msg.type == A_NIL) {
            return a_error("assertion failed");
        }
        AVal s = a_tostring(msg);
        AStatus st = a_error("%s", A_STR_OF(s)->s);
        a_dec(s);
        return st;
    }
    for (int i=0; i<argc; i++) {
        a_pushc(argv[i]);
    }
    return A_OK;
}

// next(t, k): the key and value that follow key `k` in table `t`, or
// the first ones if `k` is nil. Gives nil after the last.
static AStatus std_next (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AVal t = arg(argc, argv, 0);
    if (t.type != A_TAB) {
        return a_error("invalid argument to 'next' : expected table : got %s",
            a_typename(t));
    }
    uint32_t pos;
    if (!a_tab_after(A_TAB_OF(t), arg(argc,argv,1), &pos)) {
        return a_error("invalid argument to 'next' : unknown key");
    }
    AVal k, v;
    if (a_tab_next(A_TAB_OF(t), &pos, &k, &v)) {
        a_pushc(k);
        a_pushc(v);
    } else {
        a_push(a_nil());
    }
    return A_OK;
}

// The collection of a function of `iter`: what `loop ... in` walks,
// but `nil`, which `a_next` walks. Its members are `v, k`: in a table or a pool, the value and
// its key; over a number `n`, `v` goes from `0` to `n-1` and `k` is
// `nil`; over two numbers `a, b`, from `a` up to `b` inclusive, as the
// range of a loop, and over three, `a, b, s`, by the step `s`; over a function, the first and second values of each
// call, until it gives `nil`.
typedef struct {
    AVal xs, lim, step;
    int  swap;
} Cur;

// The collection is argument 0, or the range of arguments 0 and 1 when
// both are numbers and the call has the `nrest` arguments that follow.
// Sets `*next` to the index of the first argument after it.
static AStatus cur_init (const char* name, int argc, AVal* argv, int nrest,
    Cur* c, int* next)
{
    c->xs  = arg(argc, argv, 0);
    c->lim  = a_nil();
    c->step = a_nil();
    *next   = 1;
    if (a_isnum(c->xs) && argc>=2+nrest && a_isnum(argv[1])) {
        c->lim = argv[1];
        *next  = 2;
        // a third number is the step, if the call still has the
        // arguments that follow
        if (argc>=3+nrest && a_isnum(argv[2])) {
            c->step = argv[2];
            *next   = 3;
        }
    }
    if (c->xs.type!=A_TAB && c->xs.type!=A_TASKS && c->xs.type!=A_FUNC && !a_isnum(c->xs)) {
        return a_error("invalid argument to '%s' : expected table, tasks, number or function : got %s",
            name, a_typename(c->xs));
    }
    // over a number or a function, the first value is the member
    c->swap = (c->xs.type==A_FUNC || a_isnum(c->xs));
    return A_OK;
}

static AStatus arg_fn (const char* name, AVal f) {
    if (f.type != A_FUNC) {
        return a_error("invalid argument to '%s' : expected function : got %s",
            name, a_typename(f));
    }
    return A_OK;
}

// What a function of `iter` does with each member: `each` gets the
// member and `f(v, k)` (or nil with no `f`), and stops the walk by
// returning 1, or by setting `*st`.
typedef int (*Each) (AVal v, AVal k, AVal r, void* ud, AStatus* st);

// Walks the collection of the arguments with `f(v, k)`, which is
// argument `*next` (optional if `opt`).
static AStatus walk (const char* name, int argc, AVal* argv, int nrest,
    int opt, Each each, void* ud)
{
    Cur c;
    int next;
    if (cur_init(name, argc, argv, nrest, &c, &next) != A_OK) {
        return A_ERROR;
    }
    AVal f = arg(argc, argv, next);
    if (!(opt && f.type==A_NIL) && arg_fn(name, f)!=A_OK) {
        return A_ERROR;
    }
    AStatus st = A_OK;
    AVal pos=a_nil(), k=a_nil(), v=a_nil();
    while (1) {
        st = a_next(c.xs, c.lim, c.step, &pos, &k, &v);
        if (st!=A_OK || k.type==A_NIL) {
            break;
        }
        AVal vv = c.swap ? k : v;
        AVal kk = c.swap ? v : k;
        AVal r = a_nil();
        if (f.type == A_FUNC) {
            int base = a_top;
            a_pushc(vv);
            a_pushc(kk);
            st = a_call(f, base);
            if (st != A_OK) {
                break;
            }
            r = (a_top > base) ? a_stk[base] : a_nil();
            a_inc(r);
            a_settop(base);
        }
        int stop = each(vv, kk, r, ud, &st);
        a_dec(r);
        if (stop || st!=A_OK) {
            break;
        }
    }
    a_clr(&k);
    a_clr(&v);
    return st;
}

// iter.find(xs, f): the first member `v` of `xs` for which `f(v, k)`
// is truthy, and its `k`. Gives nil if there is none.
static int find_each (AVal v, AVal k, AVal r, void* ud, AStatus* st) {
    (void) st;
    if (!a_truthy(r)) {
        return 0;
    }
    *((int*) ud) = 1;
    a_pushc(v);
    a_pushc(k);
    return 1;
}

static AStatus std_find (AFunc* self, int argc, AVal* argv) {
    (void) self;
    int found = 0;
    AStatus st = walk("find", argc, argv, 1, 0, find_each, &found);
    if (st==A_OK && !found) {
        a_push(a_nil());
    }
    return st;
}

// iter.any(xs, f), iter.all(xs, f): whether `f(v, k)` is truthy for
// some member, or for every member (true if there is none).
typedef struct {
    int want;       // the answer of `f` that decides
    int hit;
} Any;

static int any_each (AVal v, AVal k, AVal r, void* ud, AStatus* st) {
    (void) v;
    (void) k;
    (void) st;
    Any* a = ud;
    if (a_truthy(r) == a->want) {
        a->hit = 1;
        return 1;
    }
    return 0;
}

static AStatus std_any (AFunc* self, int argc, AVal* argv) {
    (void) self;
    Any a = { 1, 0 };
    if (walk("any", argc, argv, 1, 0, any_each, &a) != A_OK) {
        return A_ERROR;
    }
    a_push(a_bool(a.hit));
    return A_OK;
}

static AStatus std_all (AFunc* self, int argc, AVal* argv) {
    (void) self;
    Any a = { 0, 0 };
    if (walk("all", argc, argv, 1, 0, any_each, &a) != A_OK) {
        return A_ERROR;
    }
    a_push(a_bool(!a.hit));
    return A_OK;
}

// iter.filter(xs, f): a new vector with the members `v` of `xs` for
// which `f(v, k)` is truthy, in the order of the walk.
// iter.map(xs, f): a new vector with `f(v, k)` of each member; a nil
// is an error, since a vector has no nil (atmos-iter#22b).
typedef struct {
    AVal    t;
    int64_t n;
} Vec;

static int filter_each (AVal v, AVal k, AVal r, void* ud, AStatus* st) {
    (void) k;
    Vec* x = ud;
    if (a_truthy(r)) {
        *st = a_setindex(x->t, a_int(x->n++), v);
    }
    return 0;
}

static int map_each (AVal v, AVal k, AVal r, void* ud, AStatus* st) {
    (void) v;
    (void) k;
    Vec* x = ud;
    if (r.type == A_NIL) {
        *st = a_error("invalid result to 'map' : unexpected nil");
        return 1;
    }
    *st = a_setindex(x->t, a_int(x->n++), r);
    return 0;
}

static AStatus to_vec (const char* name, Each each, int argc, AVal* argv) {
    Vec x = { a_tab(), 0 };
    if (walk(name, argc, argv, 1, 0, each, &x) != A_OK) {
        a_dec(x.t);
        return A_ERROR;
    }
    a_push(x.t);
    return A_OK;
}

static AStatus std_filter (AFunc* self, int argc, AVal* argv) {
    (void) self;
    return to_vec("filter", filter_each, argc, argv);
}

static AStatus std_map (AFunc* self, int argc, AVal* argv) {
    (void) self;
    return to_vec("map", map_each, argc, argv);
}

// iter.fold(xs, acc, f): `acc` folded with `acc = f(acc, v, k)` over
// each member.
static AStatus std_fold (AFunc* self, int argc, AVal* argv) {
    (void) self;
    Cur c;
    int next;
    if (cur_init("fold", argc, argv, 2, &c, &next) != A_OK) {
        return A_ERROR;
    }
    AVal acc = a_nil();
    a_cpy(&acc, arg(argc, argv, next));
    AVal f = arg(argc, argv, next+1);
    if (arg_fn("fold", f) != A_OK) {
        a_clr(&acc);
        return A_ERROR;
    }
    AStatus st;
    AVal pos=a_nil(), k=a_nil(), v=a_nil();
    while (1) {
        st = a_next(c.xs, c.lim, c.step, &pos, &k, &v);
        if (st!=A_OK || k.type==A_NIL) {
            break;
        }
        int base = a_top;
        a_pushc(acc);
        a_pushc(c.swap ? k : v);
        a_pushc(c.swap ? v : k);
        st = a_call(f, base);
        if (st != A_OK) {
            break;
        }
        a_cpy(&acc, (a_top > base) ? a_stk[base] : a_nil());
        a_settop(base);
    }
    a_clr(&k);
    a_clr(&v);
    if (st != A_OK) {
        a_clr(&acc);
        return st;
    }
    a_push(acc);
    return A_OK;
}

// iter.sum(xs): the sum of the members, `0` if there is none.
static int sum_each (AVal v, AVal k, AVal r, void* ud, AStatus* st) {
    (void) k;
    (void) r;
    AVal* acc = ud;
    if (!a_isnum(v)) {
        *st = a_error("invalid argument to 'sum' : expected number : got %s",
            a_typename(v));
        return 1;
    }
    *st = a_add(acc, *acc, v);
    return 0;
}

static AStatus std_sum (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AVal acc = a_int(0);
    if (walk("sum", argc, argv, 0, 1, sum_each, &acc) != A_OK) {
        return A_ERROR;
    }
    a_push(acc);
    return A_OK;
}

// iter.min(xs [, f]), iter.max(xs [, f]): the least or the greatest
// member `v` of `xs`, compared by `v` or by `f(v, k)` with `<`, and its
// `k`; the first of those that tie. Gives nil if there is none.
typedef struct {
    int  max;
    int  hasf;
    int  some;
    AVal v, k, by;
} Best;

static int best_each (AVal v, AVal k, AVal r, void* ud, AStatus* st) {
    Best* b = ud;
    AVal by = b->hasf ? r : v;
    int better = !b->some;
    if (!better) {
        AVal lt = a_nil();
        *st = b->max ? a_lt(&lt, b->by, by) : a_lt(&lt, by, b->by);
        if (*st != A_OK) {
            return 1;
        }
        better = a_truthy(lt);
    }
    if (better) {
        b->some = 1;
        a_cpy(&b->v,  v);
        a_cpy(&b->k,  k);
        a_cpy(&b->by, by);
    }
    return 0;
}

static AStatus best (const char* name, int max, int argc, AVal* argv) {
    Best b = { max, 0, 0, a_nil(), a_nil(), a_nil() };
    // the function, if any, follows the collection or its range
    int i = (argc>=2 && a_isnum(arg(argc,argv,0)) && a_isnum(arg(argc,argv,1))) ? 2 : 1;
    b.hasf = (arg(argc, argv, i).type != A_NIL);
    AStatus st = walk(name, argc, argv, 0, 1, best_each, &b);
    if (st==A_OK && b.some) {
        a_pushc(b.v);
        a_pushc(b.k);
    } else if (st == A_OK) {
        a_push(a_nil());
    }
    a_clr(&b.v);
    a_clr(&b.k);
    a_clr(&b.by);
    return st;
}

static AStatus std_min (AFunc* self, int argc, AVal* argv) {
    (void) self;
    return best("min", 0, argc, argv);
}

static AStatus std_max (AFunc* self, int argc, AVal* argv) {
    (void) self;
    return best("max", 1, argc, argv);
}

// iter.zip(xs, ys): an iterator over the members of `xs` and `ys` side
// by side, `vx, vy` at each step, until the shorter ends. The captures
// are, for each side, the collection, its limit, its position, and its
// last two values.
static AStatus zip_next (AFunc* self, int argc, AVal* argv) {
    (void) argc;
    (void) argv;
    AVal* c = self->caps;
    AVal out[2];
    for (int i=0; i<2; i++) {
        AVal* x = &c[i*5];
        AStatus st = a_next(x[0], x[1], a_nil(), &x[2], &x[3], &x[4]);
        if (st != A_OK) {
            return st;
        }
        if (x[3].type == A_NIL) {
            a_push(a_nil());
            return A_OK;
        }
        // over a number or a function, the first value is the member
        out[i] = (x[0].type==A_FUNC || a_isnum(x[0])) ? x[3] : x[4];
    }
    a_pushc(out[0]);
    a_pushc(out[1]);
    return A_OK;
}

static AStatus std_zip (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AVal it = a_func(zip_next, 10);
    for (int i=0; i<2; i++) {
        AVal xs = arg(argc, argv, i);
        if (xs.type!=A_TAB && xs.type!=A_TASKS && xs.type!=A_FUNC && !a_isnum(xs)) {
            a_dec(it);
            return a_error("invalid argument to 'zip' : expected table, tasks, number or function : got %s",
                a_typename(xs));
        }
        AVal* x = &A_FUNC_OF(it)->caps[i*5];
        a_inc(xs);
        x[0] = xs;
        x[1] = x[2] = x[3] = x[4] = a_nil();
    }
    a_func_scope(it);
    a_push(it);
    return A_OK;
}

// The iterators of `iter.keys` and `iter.values`: the captures are the
// table or pool, the position of the walk, and the last key.
static AStatus iter_step (AFunc* self, int key) {
    AVal* c = self->caps;
    AVal k = a_nil(), v = a_nil();
    a_cpy(&k, c[2]);
    AStatus st = a_next(c[0], a_nil(), a_nil(), &c[1], &k, &v);
    if (st != A_OK) {
        a_clr(&k);
        a_clr(&v);
        return st;
    }
    a_cpy(&c[2], k);
    if (k.type == A_NIL) {
        a_push(a_nil());
    } else {
        a_pushc(key ? k : v);
    }
    a_clr(&k);
    a_clr(&v);
    return A_OK;
}

static AStatus keys_next (AFunc* self, int argc, AVal* argv) {
    (void) argc;
    (void) argv;
    return iter_step(self, 1);
}

static AStatus values_next (AFunc* self, int argc, AVal* argv) {
    (void) argc;
    (void) argv;
    return iter_step(self, 0);
}

static AStatus iter_of (const char* name, AFn fn, int argc, AVal* argv) {
    AVal xs = arg(argc, argv, 0);
    if (xs.type!=A_TAB && xs.type!=A_TASKS) {
        return a_error("invalid argument to '%s' : expected table or tasks : got %s",
            name, a_typename(xs));
    }
    AVal it = a_func(fn, 3);
    a_inc(xs);
    A_FUNC_OF(it)->caps[0] = xs;
    A_FUNC_OF(it)->caps[1] = a_nil();
    A_FUNC_OF(it)->caps[2] = a_nil();
    a_func_scope(it);
    a_push(it);
    return A_OK;
}

// iter.keys(xs): an iterator over the keys of the table or pool `xs`,
// in the order of the walk: `loop k in iter.keys(t) {...}`.
static AStatus std_keys (AFunc* self, int argc, AVal* argv) {
    (void) self;
    return iter_of("keys", keys_next, argc, argv);
}

// iter.values(xs): an iterator over the values of the table or pool
// `xs`, in the order of the walk: `loop v in iter.values(t) {...}`.
static AStatus std_values (AFunc* self, int argc, AVal* argv) {
    (void) self;
    return iter_of("values", values_next, argc, argv);
}

///////////////////////////////////////////////////////////////////////////////
// MATH
///////////////////////////////////////////////////////////////////////////////

// Argument `i` as a number.
static AStatus argn (int argc, AVal* argv, int i, const char* f, double* n) {
    AVal v = arg(argc, argv, i);
    if (!a_isnum(v)) {
        return a_error("invalid argument to '%s' : expected number : got %s",
            f, a_typename(v));
    }
    *n = a_dbl(v);
    return A_OK;
}

// Argument `i` as a float, for the functions that compute with floats.
static AStatus argr (int argc, AVal* argv, int i, const char* f, AFloat* n) {
    double d;
    if (argn(argc, argv, i, f, &d) != A_OK) {
        return A_ERROR;
    }
    *n = a_float(argv[i]);
    return A_OK;
}

#define MATH1(name, expr)                                               \
    static AStatus math_##name (AFunc* self, int argc, AVal* argv) {    \
        (void) self;                                                    \
        AFloat x = 0;                                                    \
        if (argr(argc, argv, 0, #name, &x) != A_OK) {                   \
            return A_ERROR;                                             \
        }                                                               \
        a_push(a_num(expr));                                            \
        return A_OK;                                                    \
    }

MATH1(sqrt,  A_RF(sqrt)(x))
MATH1(sin,   A_RF(sin)(x))
MATH1(cos,   A_RF(cos)(x))
MATH1(tan,   A_RF(tan)(x))
MATH1(asin,  A_RF(asin)(x))
MATH1(acos,  A_RF(acos)(x))
MATH1(exp,   A_RF(exp)(x))
MATH1(log,   A_RF(log)(x))

// math.floor(x) / math.ceil(x): an integer, if it fits
static AStatus math_round (int up, int argc, AVal* argv) {
    AFloat x = 0;
    if (argr(argc, argv, 0, up ? "ceil" : "floor", &x) != A_OK) {
        return A_ERROR;
    }
    a_push((argv[0].type == A_INT) ? argv[0] : a_numi(up ? A_RF(ceil)(x) : A_RF(floor)(x)));
    return A_OK;
}

static AStatus math_floor (AFunc* self, int argc, AVal* argv) {
    (void) self;
    return math_round(0, argc, argv);
}

static AStatus math_ceil (AFunc* self, int argc, AVal* argv) {
    (void) self;
    return math_round(1, argc, argv);
}

// math.abs(x): keeps the subtype
static AStatus math_abs (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AFloat x = 0;
    if (argr(argc, argv, 0, "abs", &x) != A_OK) {
        return A_ERROR;
    }
    if (argv[0].type == A_INT) {
        AInt i = argv[0].u.i;
        a_push(a_int((i < 0) ? (AInt)(0 - (AUInt)i) : i));
    } else {
        a_push(a_num(A_RF(fabs)(x)));
    }
    return A_OK;
}

// math.atan(y [, x]): the angle of the point (x, y), with x=1 by
// default, as in Lua.
static AStatus math_atan (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AFloat y = 0;
    AFloat x = 1;
    if (argr(argc,argv,0,"atan",&y) != A_OK) {
        return A_ERROR;
    }
    if (argc>1 && argr(argc,argv,1,"atan",&x)!=A_OK) {
        return A_ERROR;
    }
    a_push(a_num(A_RF(atan2)(y, x)));
    return A_OK;
}

// math.pow(x, y)
// math.atan2(y, x): the angle of the point (x, y), in (-pi, pi].
static AStatus math_atan2 (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AFloat y = 0;
    AFloat x = 0;
    if (argr(argc,argv,0,"atan2",&y)!=A_OK ||
        argr(argc,argv,1,"atan2",&x)!=A_OK)
    {
        return A_ERROR;
    }
    a_push(a_num(A_RF(atan2)(y, x)));
    return A_OK;
}

static AStatus math_pow (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AFloat x = 0;
    AFloat y = 0;
    if (argr(argc,argv,0,"pow",&x)!=A_OK || argr(argc,argv,1,"pow",&y)!=A_OK) {
        return A_ERROR;
    }
    a_push(a_num(A_RF(pow)(x, y)));
    return A_OK;
}

// math.min(...) / math.max(...)
static AStatus math_minmax (int max, int argc, AVal* argv) {
    const char* f = max ? "max" : "min";
    double x = 0;
    if (argn(argc, argv, 0, f, &x) != A_OK) {
        return A_ERROR;
    }
    // the chosen argument keeps its subtype
    AVal ret = argv[0];
    for (int i=1; i<argc; i++) {
        if (argn(argc, argv, i, f, &x) != A_OK) {
            return A_ERROR;
        }
        int c = a_num_cmp(argv[i], ret);
        if (max ? c==1 : c==-1) {
            ret = argv[i];
        }
    }
    a_push(ret);
    return A_OK;
}

static AStatus math_min (AFunc* self, int argc, AVal* argv) {
    (void) self;
    return math_minmax(0, argc, argv);
}

static AStatus math_max (AFunc* self, int argc, AVal* argv) {
    (void) self;
    return math_minmax(1, argc, argv);
}

// math.random(): [0,1)
// math.random(n): 0..n-1
// math.random(a, b): a..b
static AStatus math_random (AFunc* self, int argc, AVal* argv) {
    (void) self;
    double r = rand() / (RAND_MAX + 1.0);
    if (argc == 0) {
        a_push(a_num(r));
        return A_OK;
    }
    double a = 0;
    double b = 0;
    if (argn(argc, argv, 0, "random", &b) != A_OK) {
        return A_ERROR;
    }
    if (argc > 1) {
        a = b;
        if (argn(argc, argv, 1, "random", &b) != A_OK) {
            return A_ERROR;
        }
        b = b + 1;
    }
    if (b <= a) {
        return a_error("invalid argument to 'random' : empty interval");
    }
    a_push(a_numi(a + floor(r * (b - a))));
    return A_OK;
}

// math.randomseed(n)
static AStatus math_randomseed (AFunc* self, int argc, AVal* argv) {
    (void) self;
    double n = 0;
    if (argn(argc, argv, 0, "randomseed", &n) != A_OK) {
        return A_ERROR;
    }
    srand((unsigned) n);
    return A_OK;
}

///////////////////////////////////////////////////////////////////////////////
// X
///////////////////////////////////////////////////////////////////////////////

// Number of a key, if it is a number or a numeric string.
static int knum (AVal k, double* n) {
    if (a_isnum(k)) {
        *n = a_dbl(k);
        return 1;
    }
    char* end;
    const char* s = A_STR_OF(k)->s;
    *n = strtod(s, &end);
    return (end != s) && (*end == '\0');
}

// Numbers in order, then everything else by its string.
static int kcmp (const void* p1, const void* p2) {
    AVal k1 = ((const AEnt*)p1)->k;
    AVal k2 = ((const AEnt*)p2)->k;
    double n1, n2;
    if (knum(k1,&n1) && knum(k2,&n2)) {
        return (n1 > n2) - (n1 < n2);
    }
    AVal s1 = a_tostring(k1);
    AVal s2 = a_tostring(k2);
    int ret = strcmp(A_STR_OF(s1)->s, A_STR_OF(s2)->s);
    a_dec(s1);
    a_dec(s2);
    return ret;
}

static AStatus xtostring (Buf* b, AVal v) {
    if (v.type != A_TAB) {
        AVal s = a_tostring(v);
        buf_str(b, s);
        a_dec(s);
        return A_OK;
    }

    // collects the entries, except the tag
    ATab* t = A_TAB_OF(v);
    AEnt* es = malloc((t->an + t->en + 1) * sizeof(AEnt));
    AVal tag = t->tag;
    int n = 0;
    uint32_t pos = 0;
    AVal k, x;
    AStatus st = A_OK;
    while (a_tab_next(t, &pos, &k, &x)) {
        if (!a_isnum(k) && k.type!=A_STR) {
            st = a_error("invalid key : expected number or string : got %s",
                a_typename(k));
            break;
        }
        es[n].k = k;
        es[n].v = x;
        n++;
    }

    if (st == A_OK) {
        qsort(es, n, sizeof(AEnt), kcmp);
        if (tag.type != A_NIL) {
            AVal s = a_tostring(tag);
            buf_add(b, ":", 1);
            buf_str(b, s);
            buf_add(b, " ", 1);
            a_dec(s);
        }
        buf_add(b, "[", 1);

        // keys 0,1,... in sequence are positional
        double i = 0;
        for (int j=0; j<n && st==A_OK; j++) {
            if (j > 0) {
                buf_add(b, ", ", 2);
            }
            double kn;
            if (knum(es[j].k,&kn) && kn==i) {
                i++;
            } else {
                AVal s = a_tostring(es[j].k);
                buf_str(b, s);
                buf_add(b, "=", 1);
                a_dec(s);
            }
            st = xtostring(b, es[j].v);
        }
        buf_add(b, "]", 1);
    }
    free(es);
    return st;
}

AStatus a_xtostring (AVal* dst, AVal v) {
    Buf b = { NULL, 0, 0 };
    AStatus st = xtostring(&b, v);
    if (st == A_OK) {
        a_set(dst, a_str(b.s ? b.s : "", b.n));
    }
    free(b.s);
    return st;
}

AStatus a_xcopy (AVal* dst, AVal v) {
    if (v.type != A_TAB) {
        // the copy is free to go anywhere
        if (a_chk_store(v, 0) != A_OK) {
            return A_ERROR;
        }
        a_cpy(dst, v);
        return A_OK;
    }
    AVal ret = a_tab();
    uint32_t pos = 0;
    AVal k, x;
    AStatus st = A_OK;
    while (st==A_OK && a_tab_next(A_TAB_OF(v), &pos, &k, &x)) {
        AVal c = a_nil();
        st = a_xcopy(&c, x);
        if (st == A_OK) {
            st = a_chk_store(k, 0);
        }
        if (st == A_OK) {
            a_tab_set(A_TAB_OF(ret), k, c);
        }
        a_dec(c);
    }
    if (st == A_OK) {
        a_cpy(&A_TAB_OF(ret)->tag, A_TAB_OF(v)->tag);
        a_set(dst, ret);
    } else {
        a_dec(ret);
    }
    return st;
}

// X.tostring(v)
static AStatus x_tostring (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AVal s = a_nil();
    AStatus st = a_xtostring(&s, arg(argc,argv,0));
    if (st == A_OK) {
        a_push(s);
    }
    return st;
}

// X.print(...)
static AStatus x_print (AFunc* self, int argc, AVal* argv) {
    (void) self;
    Buf b = { NULL, 0, 0 };
    AStatus st = A_OK;
    for (int i=0; i<argc && st==A_OK; i++) {
        if (i > 0) {
            buf_add(&b, "\t", 1);
        }
        st = xtostring(&b, argv[i]);
    }
    if (st == A_OK) {
        fwrite(b.s, 1, b.n, stdout);
        fputc('\n', stdout);
    }
    free(b.s);
    return st;
}

// X.copy(v)
static AStatus x_copy (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AVal ret = a_nil();
    AStatus st = a_xcopy(&ret, arg(argc,argv,0));
    if (st == A_OK) {
        a_push(ret);
    }
    return st;
}

// X.eq(a, b): structural equality
static AStatus x_eq (AFunc* self, int argc, AVal* argv) {
    (void) self;
    a_push(a_bool(a_deq(arg(argc,argv,0), arg(argc,argv,1))));
    return A_OK;
}

// X.neq(a, b): structural inequality
static AStatus x_neq (AFunc* self, int argc, AVal* argv) {
    (void) self;
    a_push(a_bool(!a_deq(arg(argc,argv,0), arg(argc,argv,1))));
    return A_OK;
}

// X.gte(a, b): `a` is structurally more general than `b`
static AStatus x_gte (AFunc* self, int argc, AVal* argv) {
    (void) self;
    a_push(a_bool(a_gte(arg(argc,argv,0), arg(argc,argv,1))));
    return A_OK;
}

// X.lte(a, b): `a` is structurally more specific than `b`
static AStatus x_lte (AFunc* self, int argc, AVal* argv) {
    (void) self;
    a_push(a_bool(a_gte(arg(argc,argv,1), arg(argc,argv,0))));
    return A_OK;
}

// X.gt(a, b): more general, and not equal. The order is partial, so
// this is not the negation of X.lte.
static AStatus x_gt (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AVal a = arg(argc, argv, 0);
    AVal b = arg(argc, argv, 1);
    a_push(a_bool(a_gte(a,b) && !a_gte(b,a)));
    return A_OK;
}

// X.lt(a, b): more specific, and not equal
static AStatus x_lt (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AVal a = arg(argc, argv, 0);
    AVal b = arg(argc, argv, 1);
    a_push(a_bool(a_gte(b,a) && !a_gte(a,b)));
    return A_OK;
}

// X.in(v, t): membership (also `X.xin`, its former name)
static AStatus x_xin (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AVal ret = a_nil();
    AStatus st = a_in(&ret, arg(argc,argv,0), arg(argc,argv,1));
    if (st == A_OK) {
        a_push(ret);
    }
    return st;
}

// X.nin(v, t): the negation of X.in
static AStatus x_nin (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AVal ret = a_nil();
    AStatus st = a_in(&ret, arg(argc,argv,0), arg(argc,argv,1));
    if (st == A_OK) {
        a_push(a_bool(!a_truthy(ret)));
    }
    return st;
}

///////////////////////////////////////////////////////////////////////////////
// ATMOS
///////////////////////////////////////////////////////////////////////////////

// atmos.status(t): :running, :suspended, or :dead.
static AStatus atmos_status (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AVal t = arg(argc, argv, 0);
    if (t.type != A_XTASK) {
        return a_error("invalid status : expected task");
    }
    switch (A_TASK_OF(t)->status) {
        case T_RUNNING:   a_push(a_strz("running"));   break;
        case T_SUSPENDED: a_push(a_strz("suspended")); break;
        default:          a_push(a_strz("dead"));      break;
    }
    return A_OK;
}

// atmos.env(e): registers an environment, a table with `step` and `quit`
// (functions, optional) and `mode` (`[primary=true, secondary=true]`).
// The first one registered is the primary, and must support it; every
// other is a secondary, and must support it. The runtime writes the
// mode it got in `mode.current` (:primary or :secondary). The generic
// `main` (rt/main.c) calls the steps in order and the quits in reverse.
AVal atm_envs[ATM_ENVS_MAX];
int  atm_nenvs = 0;

static AStatus atmos_env (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AVal e = arg(argc, argv, 0);
    if (e.type != A_TAB) {
        return a_error("invalid environment : expected table");
    }
    if (atm_nenvs == ATM_ENVS_MAX) {
        return a_error("invalid environment : too many environments");
    }
    AVal k = a_strz("mode");
    AVal mode = a_tab_get(A_TAB_OF(e), k);
    a_dec(k);
    int primary = (atm_nenvs == 0);
    const char* want = primary ? "primary" : "secondary";
    AVal kw = a_strz(want);
    int ok = (mode.type == A_TAB) && a_truthy(a_tab_get(A_TAB_OF(mode), kw));
    a_dec(kw);
    if (!ok) {
        return a_error("invalid environment : expected %s mode", want);
    }
    AVal kc = a_strz("current");
    AVal vc = a_strz(want);
    a_tab_set(A_TAB_OF(mode), kc, vc);
    a_dec(kc);
    a_dec(vc);
    // the runtime owns it: a pinned table would be finalized with its
    // block, before its `quit` is called
    if (e.u.o->pinned) {
        return a_error("invalid environment : expected unpinned table");
    }
    a_inc(e);
    a_own(e);
    atm_envs[atm_nenvs++] = e;
    return A_OK;
}

void atm_env_quit (void) {
    for (int i=atm_nenvs-1; i>=0; i--) {
        AVal k = a_strz("quit");
        AVal f = a_tab_get(A_TAB_OF(atm_envs[i]), k);
        a_dec(k);
        if (f.type == A_NIL) {
            continue;
        }
        int base = a_top;
        AStatus st = a_call(f, base);
        a_settop(base);
        if (st != A_OK) {
            if (st == A_ESCAPE) {
                st = a_escape_err();
            }
            a_error_dump();
            a_pay_clear();
        }
    }
}

// atmos.next(): microseconds to the nearest timer, or nil.
static AStatus atmos_next (AFunc* self, int argc, AVal* argv) {
    (void) self;
    (void) argc;
    (void) argv;
    double us = a_clock_next();
    a_push((us < 0) ? a_nil() : a_numi(us));
    return A_OK;
}

///////////////////////////////////////////////////////////////////////////////

AStatus arg_str (int argc, AVal* argv, int i, const char* f, AStr** s) {
    AVal v = arg(argc, argv, i);
    if (v.type != A_STR) {
        return a_error("invalid argument to '%s' : expected string : got %s",
            f, a_typename(v));
    }
    *s = A_STR_OF(v);
    return A_OK;
}

AStatus arg_int (int argc, AVal* argv, int i, const char* f, int opt,
    int64_t def, int64_t* n)
{
    AVal v = arg(argc, argv, i);
    if (opt && v.type==A_NIL) {
        *n = def;
        return A_OK;
    }
    if (!a_toint(v, n)) {
        return a_error("invalid argument to '%s' : expected integer : got %s",
            f, a_isnum(v) ? "float" : a_typename(v));
    }
    return A_OK;
}

///////////////////////////////////////////////////////////////////////////////
// CLAUSES
///////////////////////////////////////////////////////////////////////////////

// A function with clauses, as declared by `func f (x :T, y) {...}`, is
// a function whose captures hold them:
//
//      caps[0]               its name
//      caps[i]               the function (or task prototype) of a clause
//      caps[i+1]             n, the number of its parameters
//      caps[i+2 .. i+2+n)    their tags (nil: any value)
//
// It is immutable: a new clause makes a new function, which the
// declaration assigns to the name.
//
// A call chooses the clause whose tags all match the arguments
// (`arg ?? tag`). Among those, the most specific one: the parameters
// are compared from the left, and a longer tag wins over a shorter one,
// which wins over the name of a type (`:table`), which wins over no tag.

static AStatus clause_call (AFunc* self, int argc, AVal* argv);

static int clause_is (AVal f) {
    return f.type==A_FUNC && A_FUNC_OF(f)->fn==clause_call;
}

int a_clause_is (AVal f) {
    return clause_is(f);
}

// The key `tag` of a tagged table.
static AVal CLAUSE_KTAG;

// The string by which an argument is matched against a tag: itself, if
// a string, or its field `tag`, if a table. NULL otherwise.
static AStr* clause_argtag (AVal arg) {
    if (arg.type == A_STR) {
        return A_STR_OF(arg);
    }
    if (arg.type == A_TAB) {
        AVal tag = a_tab_get(A_TAB_OF(arg), CLAUSE_KTAG);
        if (tag.type == A_STR) {
            return A_STR_OF(tag);
        }
    }
    return NULL;
}

// How specific a tag is for an argument, as `arg ?? tag`: 0 if it does
// not match.
static int clause_score (AVal tag, AVal arg, AStr* argtag) {
    if (tag.type == A_NIL) {
        return 1;
    }
    if (tag.type != A_STR) {
        return a_is(arg, tag) ? 3 : 0;
    }
    AStr* s = A_STR_OF(tag);
    if (argtag != NULL) {
        if (s==argtag || (argtag->len > s->len && argtag->s[s->len]=='.' &&
                          !memcmp(s->s, argtag->s, s->len)))
        {
            return 3 + (int) s->len;
        }
    }
    const char* type = a_typename(arg);
    if (type[0]==s->s[0] && !strcmp(type, s->s)) {
        return 2;                   // the name of its type
    }
    return 0;
}

// The tag by which an argument is chosen, for an error message.
static const char* clause_argname (AVal arg) {
    AStr* s = (arg.type == A_TAB) ? clause_argtag(arg) : NULL;
    return (s != NULL) ? s->s : a_typename(arg);
}

#define CLAUSE_PARS 16

AStatus a_clause_pick (AVal f, int argc, AVal* argv, AVal* out) {
    if (!clause_is(f)) {
        *out = f;
        return A_OK;
    }
    AFunc* g = A_FUNC_OF(f);
    AStr* tags[CLAUSE_PARS];
    for (int j=0; j<CLAUSE_PARS; j++) {
        tags[j] = (j < argc) ? clause_argtag(argv[j]) : NULL;
    }
    int best = -1;
    int bests[CLAUSE_PARS];
    for (int i=1; i<g->ncap; ) {
        int n = (int) g->caps[i+1].u.i;
        // applies? more specific than the best so far, from the left?
        int cur[CLAUSE_PARS];
        int ok  = 1;
        int cmp = (best == -1);
        for (int j=0; ok && j<CLAUSE_PARS; j++) {
            cur[j] = (j < n) ?
                clause_score(g->caps[i+2+j], arg(argc,argv,j), tags[j]) : 1;
            ok = (cur[j] > 0);
            if (cmp == 0) {
                cmp = cur[j] - bests[j];
            }
        }
        if (ok && cmp>0) {
            best = i;
            memcpy(bests, cur, sizeof(cur));
        }
        i += 2 + n;
    }
    if (best == -1) {
        char buf[256];
        size_t len = 0;
        buf[0] = '\0';
        for (int j=0; j<argc && len<sizeof(buf)-1; j++) {
            len += snprintf(buf+len, sizeof(buf)-len, "%s%s",
                (j > 0) ? ", " : "", clause_argname(argv[j]));
        }
        return a_error("invalid call : no clause of '%s' for (%s)",
            A_STR_OF(g->caps[0])->s, buf);
    }
    *out = g->caps[best];
    return A_OK;
}

static AStatus clause_call (AFunc* self, int argc, AVal* argv) {
    AVal f = a_nil();
    AVal g = a_obj(&self->hdr);
    g.type = A_FUNC;
    AStatus st = a_clause_pick(g, argc, argv, &f);
    if (st != A_OK) {
        return st;
    }
    if (f.type != A_FUNC) {
        return a_error("invalid call : expected function : got %s",
            a_typename(f));
    }
    return A_FUNC_OF(f)->fn(A_FUNC_OF(f), argc, argv);
}

// Whether the tag `p` of a clause is as or more general than the tag
// `t` of another, for the same parameter: no tag, the same tag, or a
// dot prefix of it.
static int clause_above (AVal p, AVal t) {
    if (p.type == A_NIL) {
        return 1;
    }
    if (a_eq(p, t)) {
        return 1;
    }
    if (p.type!=A_STR || t.type!=A_STR) {
        return 0;
    }
    AStr* a = A_STR_OF(p);
    AStr* b = A_STR_OF(t);
    return b->len > a->len && b->s[a->len]=='.' && !memcmp(a->s, b->s, a->len);
}

// $super(f, n, tag1, ..., tagN, args...): calls, with `args`, the most
// specific clause of `f` that applies to them among the parents of the
// clause of tags `tag1..tagN`: those whose every tag is as or more
// general, and that differ in some.
static AStatus std_super (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AVal f = arg(argc, argv, 0);
    int  n = (int) arg(argc, argv, 1).u.i;
    AVal* own = &argv[2];
    int   na  = argc - 2 - n;
    AVal* args = &argv[2+n];
    if (!clause_is(f)) {
        return a_error("invalid super : expected a function with clauses");
    }
    AFunc* g = A_FUNC_OF(f);
    AStr* tags[CLAUSE_PARS];
    for (int j=0; j<CLAUSE_PARS; j++) {
        tags[j] = (j < na) ? clause_argtag(args[j]) : NULL;
    }
    int best = -1;
    int bests[CLAUSE_PARS];
    for (int i=1; i<g->ncap; ) {
        int m = (int) g->caps[i+1].u.i;
        // a parent of the running clause?
        int above = 1;
        int diff  = 0;
        for (int j=0; above && (j<m || j<n); j++) {
            AVal p = (j < m) ? g->caps[i+2+j] : a_nil();
            AVal t = (j < n) ? own[j]         : a_nil();
            above = clause_above(p, t);
            diff  = diff || !a_eq(p, t);
        }
        if (above && diff) {
            int cur[CLAUSE_PARS];
            int ok  = 1;
            int cmp = (best == -1);
            for (int j=0; ok && j<CLAUSE_PARS; j++) {
                AVal a = (j < na) ? args[j] : a_nil();
                cur[j] = (j < m) ? clause_score(g->caps[i+2+j], a, tags[j]) : 1;
                ok = (cur[j] > 0);
                if (cmp == 0) {
                    cmp = cur[j] - bests[j];
                }
            }
            if (ok && cmp>0) {
                best = i;
                memcpy(bests, cur, sizeof(cur));
            }
        }
        i += 2 + m;
    }
    if (best == -1) {
        return a_error("invalid super : no parent clause of '%s'",
            A_STR_OF(g->caps[0])->s);
    }
    AVal h = g->caps[best];
    if (h.type != A_FUNC) {
        return a_error("invalid super : expected function : got %s",
            a_typename(h));
    }
    return A_FUNC_OF(h)->fn(A_FUNC_OF(h), na, args);
}

// $clause(cur, name, f, tag1, ..., tagN): the function `cur` (nil, a
// function with clauses, or a plain function, which becomes the default
// clause) with one more clause `f`, of N parameters.
static AStatus std_clause (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AVal cur  = arg(argc, argv, 0);
    AVal name = arg(argc, argv, 1);
    AVal f    = arg(argc, argv, 2);
    int  n    = (argc > 3) ? argc-3 : 0;
    const char* id = A_STR_OF(name)->s;
    if (n > CLAUSE_PARS) {
        return a_error("invalid clause : '%s' has too many parameters", id);
    }

    int old = 0;                // captures taken from `cur`
    int def = 0;                // `cur` is a plain function: the default
    if (clause_is(cur)) {
        old = A_FUNC_OF(cur)->ncap - 1;
        // the same tags twice
        AFunc* g = A_FUNC_OF(cur);
        for (int i=1; i<g->ncap; ) {
            int m = (int) g->caps[i+1].u.i;
            int same = 1;
            for (int j=0; same && (j<m || j<n); j++) {
                AVal t1 = (j < m) ? g->caps[i+2+j] : a_nil();
                AVal t2 = (j < n) ? argv[3+j]      : a_nil();
                same = a_eq(t1, t2);
            }
            if (same) {
                return a_error("invalid clause : '%s' already has it", id);
            }
            i += 2 + m;
        }
    } else if (cur.type==A_FUNC || cur.type==A_TASK) {
        def = 1;
        int any = 0;
        for (int j=0; j<n; j++) {
            any = any || (argv[3+j].type != A_NIL);
        }
        if (!any) {
            return a_error("invalid clause : '%s' already has it", id);
        }
    } else if (cur.type != A_NIL) {
        return a_error("invalid clause : '%s' is not a function : got %s",
            id, a_typename(cur));
    }

    AVal v = a_func(clause_call, 1 + old + 2*def + 2 + n);
    AFunc* g = A_FUNC_OF(v);
    int k = 0;
    g->caps[k++] = name;
    for (int i=0; i<old; i++) {
        g->caps[k++] = A_FUNC_OF(cur)->caps[1+i];
    }
    if (def) {
        g->caps[k++] = cur;
        g->caps[k++] = a_int(0);
    }
    g->caps[k++] = f;
    g->caps[k++] = a_int(n);
    for (int j=0; j<n; j++) {
        g->caps[k++] = argv[3+j];
    }
    for (int i=0; i<g->ncap; i++) {
        a_inc(g->caps[i]);
    }
    a_func_scope(v);
    a_push(v);
    return A_OK;
}

void atm_std_init (void) {
    atm_g_clause   = a_func(std_clause, 0);
    atm_g_super    = a_func(std_super, 0);
    CLAUSE_KTAG    = a_strz("tag");
    atm_g_print    = a_func(std_print, 0);
    atm_g_type     = a_func(std_type, 0);
    atm_g_tostring = a_func(std_tostring, 0);
    atm_g_tonumber = a_func(std_tonumber, 0);
    atm_g_assert   = a_func(std_assert, 0);
    atm_g_next     = a_func(std_next, 0);
    // the library `iter`, over what `loop ... in` walks
    atm_g_iter = a_tab();
    field(atm_g_iter, "find",   std_find);
    field(atm_g_iter, "filter", std_filter);
    field(atm_g_iter, "map",    std_map);
    field(atm_g_iter, "any",    std_any);
    field(atm_g_iter, "all",    std_all);
    field(atm_g_iter, "fold",   std_fold);
    field(atm_g_iter, "sum",    std_sum);
    field(atm_g_iter, "min",    std_min);
    field(atm_g_iter, "max",    std_max);
    field(atm_g_iter, "zip",    std_zip);
    field(atm_g_iter, "keys",   std_keys);
    field(atm_g_iter, "values", std_values);

    atm_g_X = a_tab();
    field(atm_g_X, "tostring", x_tostring);
    field(atm_g_X, "print",    x_print);
    field(atm_g_X, "copy",     x_copy);
    field(atm_g_X, "eq",       x_eq);
    field(atm_g_X, "neq",      x_neq);
    field(atm_g_X, "gte",      x_gte);
    field(atm_g_X, "lte",      x_lte);
    field(atm_g_X, "gt",       x_gt);
    field(atm_g_X, "lt",       x_lt);
    field(atm_g_X, "in",       x_xin);
    field(atm_g_X, "xin",      x_xin);   // former name, to be removed
    field(atm_g_X, "nin",      x_nin);

    atm_g_math = a_tab();
    field(atm_g_math, "floor",      math_floor);
    field(atm_g_math, "ceil",       math_ceil);
    field(atm_g_math, "abs",        math_abs);
    field(atm_g_math, "sqrt",       math_sqrt);
    field(atm_g_math, "sin",        math_sin);
    field(atm_g_math, "cos",        math_cos);
    field(atm_g_math, "tan",        math_tan);
    field(atm_g_math, "asin",       math_asin);
    field(atm_g_math, "acos",       math_acos);
    field(atm_g_math, "atan",       math_atan);
    field(atm_g_math, "atan2",      math_atan2);
    field(atm_g_math, "exp",        math_exp);
    field(atm_g_math, "log",        math_log);
    field(atm_g_math, "pow",        math_pow);
    field(atm_g_math, "min",        math_min);
    field(atm_g_math, "max",        math_max);
    field(atm_g_math, "random",     math_random);
    field(atm_g_math, "randomseed", math_randomseed);
    konst(atm_g_math, "huge", a_num(INFINITY));
    konst(atm_g_math, "pi",   a_num(3.14159265358979323846));

    atm_g_atmos = a_tab();
    field(atm_g_atmos, "status", atmos_status);
    field(atm_g_atmos, "next",   atmos_next);
    field(atm_g_atmos, "env",    atmos_env);

    atm_lib_load_init();
    atm_lib_string_init();
    atm_lib_vector_init();
    atm_lib_os_init();
    atm_lib_io_init();

    // the values of the libraries are owned by the runtime
    a_own(atm_g_print);
    a_own(atm_g_type);
    a_own(atm_g_tostring);
    a_own(atm_g_tonumber);
    a_own(atm_g_assert);
    a_own(atm_g_next);
    a_own(atm_g_iter);
    a_own(atm_g_clause);
    a_own(atm_g_super);
    a_own(atm_g_X);
    a_own(atm_g_math);
    a_own(atm_g_atmos);
    a_own(atm_g_string);
    a_own(atm_g_vector);
    a_own(atm_g_os);
    a_own(atm_g_io);
}

void atm_std_quit (void) {
    a_clr(&atm_g_print);
    a_clr(&atm_g_type);
    a_clr(&atm_g_tostring);
    a_clr(&atm_g_tonumber);
    a_clr(&atm_g_assert);
    a_clr(&atm_g_next);
    a_clr(&atm_g_iter);
    a_clr(&atm_g_clause);
    a_clr(&atm_g_super);
    a_clr(&CLAUSE_KTAG);
    a_clr(&atm_g_X);
    for (int i=0; i<atm_nenvs; i++) {
        a_clr(&atm_envs[i]);
    }
    atm_nenvs = 0;
    a_clr(&atm_g_atmos);
    a_clr(&atm_g_math);
    atm_lib_string_quit();
    atm_lib_vector_quit();
    atm_lib_os_quit();
    atm_lib_io_quit();
}
