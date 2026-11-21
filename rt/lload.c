// X.load: reads a value written with the constructors of the language.
//
//      Value : `nil´ | `true´ | `false´
//            | [`-´] NUM | CLK | STR | TAG
//            | [TAG] `[´ { Entry `,´ } `]´
//      Entry : ID `=´ Value
//            | `@´ Key `=´ Value
//            | Value
//      Key   : `(´ Value `)´ | NUM | STR | TAG
//
// It is data only: there are no variables, operations, functions, or
// calls. Comments are accepted: `;; ...` up to the end of the line,
// and `;;; ... ;;;` over multiple lines.
//
// X.loadx: reads the same, with names defined before the value.
//
//      Text  : { Def } Value
//      Def   : ID `=´ Value
//            | `include´ STR
//      Value : ... | ID
//
// A name is used as a value only, and each use is a deep copy of what
// it holds: the result is what X.load gives for the text with each
// name replaced by its value. A name sees the names defined before it.
// The first definition of a name wins: a later one is read and ignored,
// so a text replaces a name of a file by defining it before the include.
// An include reads the definitions of another file, which has no value
// of its own. The file is relative to the directory given to X.loadx
// (or to the current one), and inside an included file, to that file.
// A file is only included once.

#include <ctype.h>
#include <math.h>
#include <stdio.h>

#include "lib.h"

// Nested tables allowed, to protect the C stack.
#define DEPTH_MAX 200

// A name of X.loadx: its text, its value, and how deep the value nests.
typedef struct Name {
    const char* s;
    size_t n;
    AVal v;
    int depth;
} Name;

// A file included by X.loadx: the names point into its text.
typedef struct File {
    char* path;
    char* buf;
} File;

// What X.loadx keeps along a text and the files it includes.
typedef struct Env {
    Name* names;
    int names_n;
    int names_max;
    File* files;
    int files_n;
    int files_max;
} Env;

typedef struct Load {
    const char* s;
    const char* end;
    int lin;
    int depth;
    int top;            // deepest table since the value in course started
    Env* e;             // X.loadx: names are accepted
    const char* file;   // an included file, or NULL for the text given
    const char* dir;    // where its includes are, or NULL for the current
} Load;

// "invalid load : [file : ]line N : msg[ : name]"
static AStatus failn (Load* l, const char* msg, const char* s, size_t n) {
    return a_error("invalid load : %s%sline %d : %s%s%.*s",
        l->file==NULL ? "" : l->file, l->file==NULL ? "" : " : ",
        l->lin, msg, s==NULL ? "" : " : ", (int)n, s==NULL ? "" : s);
}

static AStatus fail (Load* l, const char* msg) {
    return failn(l, msg, NULL, 0);
}

// Skips blanks and comments. Returns 0 on an unterminated comment.
static int skip (Load* l) {
    while (l->s < l->end) {
        char c = *l->s;
        if (c == '\n') {
            l->lin++;
            l->s++;
        } else if (c==' ' || c=='\t' || c=='\r') {
            l->s++;
        } else if (c==';' && l->s+1<l->end && l->s[1]==';') {
            // the number of semicolons tells the kind
            int n = 0;
            while (l->s<l->end && *l->s==';') {
                n++;
                l->s++;
            }
            if (n == 2) {
                while (l->s<l->end && *l->s!='\n') {
                    l->s++;
                }
                continue;
            }
            // up to the same number of semicolons
            int m = 0;
            while (l->s<l->end && m!=n) {
                if (*l->s == ';') {
                    m++;
                    if (m==n && l->s+1<l->end && l->s[1]==';') {
                        // a longer run does not close it
                        while (l->s<l->end && *l->s==';') {
                            l->s++;
                        }
                        m = 0;
                        continue;
                    }
                } else {
                    m = 0;
                    if (*l->s == '\n') {
                        l->lin++;
                    }
                }
                l->s++;
            }
            if (m != n) {
                return 0;
            }
        } else {
            break;
        }
    }
    return 1;
}

static int is_id1 (char c) {
    return isalpha((unsigned char)c) || c=='_';
}

static int is_id (char c) {
    return isalnum((unsigned char)c) || c=='_';
}

// The next token is the symbol `c`: consumes it.
static int accept (Load* l, char c) {
    if (l->s<l->end && *l->s==c) {
        l->s++;
        return 1;
    }
    return 0;
}

static AStatus value (Load* l, AVal* dst);

static Name* name_get (Load* l, const char* s, size_t n) {
    for (int i=0; i<l->e->names_n; i++) {
        Name* nm = &l->e->names[i];
        if (nm->n==n && !strncmp(nm->s,s,n)) {
            return nm;
        }
    }
    return NULL;
}

// CLK: digits and units in descending order (day h min s ms us), as
// in `1h30min`. Returns the duration in microseconds, or -1 if the
// text is not a clock.
static double clock_us (const char* s) {
    static const char*  US[] = { "day", "h", "min", "s", "ms", "us" };
    static const double VS[] = { 864e8, 36e8, 6e7, 1e6, 1e3, 1 };
    double ret = 0;
    int ok = 0;
    for (int u=0; u<6; u++) {
        const char* p = s;
        while (isdigit((unsigned char)*p)) {
            p++;
        }
        if (p == s) {
            continue;
        }
        if (*p == '.') {
            p++;
        }
        while (isdigit((unsigned char)*p)) {
            p++;
        }
        size_t n = strlen(US[u]);
        if (strncmp(p, US[u], n) != 0) {
            continue;
        }
        // "s" must not shadow a longer unit
        if (isalpha((unsigned char)p[n])) {
            continue;
        }
        ret += strtod(s, NULL) * VS[u];
        ok = 1;
        s = p + n;
    }
    return (ok && *s=='\0') ? ret : -1;
}

// NUM: as a literal of the language, plus an exponent with a sign.
static AStatus number (Load* l, int neg, AVal* dst) {
    const char* s = l->s;
    const char* p = s;
    int hex = (p+1<l->end && p[0]=='0' && (p[1]=='x' || p[1]=='X'));
    int real = 0;
    while (p < l->end) {
        char c = *p;
        if (isalnum((unsigned char)c) || c=='.') {
            real = real || (c == '.') ||
                   (!hex && (c=='e' || c=='E')) ||
                   ( hex && (c=='p' || c=='P'));
            p++;
        } else if ((c=='+' || c=='-') && p>s &&
                   ((!hex && (p[-1]=='e' || p[-1]=='E')) ||
                    ( hex && (p[-1]=='p' || p[-1]=='P'))))
        {
            p++;
        } else {
            break;
        }
    }
    char buf[64];
    size_t n = p - s;
    if (n >= sizeof(buf)) {
        return fail(l, "invalid number");
    }
    memcpy(buf, s, n);
    buf[n] = '\0';

    // a clock is a number of units of time: an integer, which must fit,
    // or a float if it has a fraction of the unit
    double us = neg ? -1 : clock_us(buf);
    if (us >= 0) {
        double t = us / A_CLOCK_US;
        if (t >= A_INT_LIM) {
            return fail(l, "invalid clock : out of range");
        }
        if (t == floor(t)) {
            a_set(dst, a_int((AInt)t));
        } else {
            a_set(dst, a_num(t));
        }
        l->s = p;
        return A_OK;
    }

    char* stop;
    if (!real) {
        unsigned long long v = strtoull(buf, &stop, 0);
        if (*stop != '\0') {
            return fail(l, "invalid number");
        }
        if (hex || (n<=19 && v<=(unsigned long long)A_INT_MAX)) {
            a_set(dst, a_int(neg ? (AInt)(0 - v) : (AInt)v));
            l->s = p;
            return A_OK;
        }
    }
    AFloat d = A_STRTOR(buf, &stop);
    if (*stop != '\0') {
        return fail(l, "invalid number");
    }
    a_set(dst, a_num(neg ? -d : d));
    l->s = p;
    return A_OK;
}

// STR: one delimiter is a single line with escapes, three or more are
// multiple lines as they are, and two are the empty string.
static AStatus string (Load* l, AVal* dst) {
    char q = *l->s;
    int n = 0;
    while (l->s<l->end && *l->s==q) {
        n++;
        l->s++;
    }
    if (n == 2) {
        a_set(dst, a_strz(""));
        return A_OK;
    }

    Buf b = { NULL, 0, 0 };
    if (n > 1) {
        const char* s = l->s;
        int m = 0;
        while (l->s<l->end && m!=n) {
            m = (*l->s == q) ? m+1 : 0;
            if (*l->s == '\n') {
                l->lin++;
            }
            l->s++;
        }
        if (m != n) {
            return fail(l, "unterminated string");
        }
        a_set(dst, a_str(s, (l->s - n) - s));
        return A_OK;
    }

    while (1) {
        if (l->s>=l->end || *l->s=='\n') {
            free(b.s);
            return fail(l, "unterminated string");
        }
        char c = *l->s++;
        if (c == q) {
            break;
        }
        if (c != '\\') {
            buf_add(&b, &c, 1);
            continue;
        }
        if (l->s >= l->end) {
            free(b.s);
            return fail(l, "unterminated string");
        }
        c = *l->s++;
        int v = -1;
        switch (c) {
            case 'a':  v = '\a'; break;
            case 'b':  v = '\b'; break;
            case 'f':  v = '\f'; break;
            case 'n':  v = '\n'; break;
            case 'r':  v = '\r'; break;
            case 't':  v = '\t'; break;
            case 'v':  v = '\v'; break;
            case '\\': v = '\\'; break;
            case '"':  v = '"';  break;
            case '\'': v = '\''; break;
            case 'x':
                // \xHH
                if (l->s+1<l->end && isxdigit((unsigned char)l->s[0]) &&
                    isxdigit((unsigned char)l->s[1]))
                {
                    char hh[3] = { l->s[0], l->s[1], '\0' };
                    v = (int) strtol(hh, NULL, 16);
                    l->s += 2;
                }
                break;
            default:
                // \ddd
                if (isdigit((unsigned char)c)) {
                    v = c - '0';
                    for (int i=0; i<2 && l->s<l->end &&
                                  isdigit((unsigned char)*l->s); i++) {
                        v = v*10 + (*l->s++ - '0');
                    }
                    if (v > 255) {
                        v = -1;
                    }
                }
                break;
        }
        if (v < 0) {
            free(b.s);
            return fail(l, "invalid escape sequence");
        }
        char ch = (char) v;
        buf_add(&b, &ch, 1);
    }
    a_set(dst, buf_val(&b));
    return A_OK;
}

// TAG: the string after the colon.
static AStatus tag (Load* l, AVal* dst) {
    const char* s = ++l->s;
    while (l->s<l->end && (is_id(*l->s) || *l->s=='.')) {
        l->s++;
    }
    if (l->s == s) {
        return fail(l, "invalid tag");
    }
    a_set(dst, a_str(s, l->s - s));
    return A_OK;
}

// After `[`: the entries, into `t`.
static AStatus table (Load* l, AVal t) {
    if (++l->depth > DEPTH_MAX) {
        return fail(l, "too many nested tables");
    }
    if (l->depth > l->top) {
        l->top = l->depth;
    }
    int64_t n = 0;
    while (1) {
        if (!skip(l)) {
            return fail(l, "unterminated comment");
        }
        if (accept(l, ']')) {
            break;
        }

        AVal k = a_nil();
        AVal v = a_nil();
        AStatus st = A_OK;
        int key = 0;
        if (accept(l, '@')) {
            // @(v)=  @num=  @"s"=  @:tag=
            key = 1;
            if (accept(l, '(')) {
                st = value(l, &k);
                if (st==A_OK && !(skip(l) && accept(l,')'))) {
                    st = fail(l, "expected ')'");
                }
            } else if (l->s<l->end &&
                       (isdigit((unsigned char)*l->s) || *l->s=='"' ||
                        *l->s=='\'' || *l->s==':'))
            {
                st = value(l, &k);
            } else {
                st = fail(l, "expected key");
            }
        } else if (l->s<l->end && is_id1(*l->s)) {
            // id=  (or nil, true, false as a value)
            const char* p = l->s;
            while (p<l->end && is_id(*p)) {
                p++;
            }
            const char* q = p;
            while (q<l->end && (*q==' ' || *q=='\t')) {
                q++;
            }
            if (q<l->end && *q=='=') {
                key = 1;
                k = a_str(l->s, p - l->s);
                l->s = p;
            }
        }
        if (st==A_OK && key) {
            if (!(skip(l) && accept(l,'='))) {
                st = fail(l, "expected '='");
            } else if (k.type==A_NIL || (k.type==A_NUM && k.u.n!=k.u.n)) {
                st = fail(l, "invalid key");
            }
        }
        if (st == A_OK) {
            st = value(l, &v);
        }
        if (st == A_OK) {
            a_tab_set(A_TAB_OF(t), key ? k : a_int(n), v);
            n += !key;
        }
        a_dec(k);
        a_dec(v);
        if (st != A_OK) {
            return st;
        }

        if (!skip(l)) {
            return fail(l, "unterminated comment");
        }
        if (accept(l, ',')) {
            continue;
        }
        if (accept(l, ']')) {
            break;
        }
        return fail(l, "expected ',' or ']'");
    }
    l->depth--;
    return A_OK;
}

static AStatus value (Load* l, AVal* dst) {
    if (!skip(l)) {
        return fail(l, "unterminated comment");
    }
    if (l->s >= l->end) {
        return fail(l, "expected value");
    }
    char c = *l->s;

    if (c == '[') {
        l->s++;
        AVal t = a_tab();
        AStatus st = table(l, t);
        if (st == A_OK) {
            a_set(dst, t);
        } else {
            a_dec(t);
        }
        return st;
    }
    if (c == ':') {
        AVal tg = a_nil();
        if (tag(l, &tg) != A_OK) {
            return A_ERROR;
        }
        // :X [...]: in the same line
        const char* p = l->s;
        while (p<l->end && (*p==' ' || *p=='\t')) {
            p++;
        }
        if (p>=l->end || *p!='[') {
            a_set(dst, tg);
            return A_OK;
        }
        l->s = p + 1;
        AVal t = a_tab();
        AVal k = a_strz("tag");
        a_tab_set(A_TAB_OF(t), k, tg);
        a_dec(k);
        a_dec(tg);
        AStatus st = table(l, t);
        if (st == A_OK) {
            a_set(dst, t);
        } else {
            a_dec(t);
        }
        return st;
    }
    if (c=='"' || c=='\'') {
        return string(l, dst);
    }
    if (isdigit((unsigned char)c)) {
        return number(l, 0, dst);
    }
    if (c=='-' && l->s+1<l->end && isdigit((unsigned char)l->s[1])) {
        l->s++;
        return number(l, 1, dst);
    }
    if (is_id1(c)) {
        const char* p = l->s;
        while (p<l->end && is_id(*p)) {
            p++;
        }
        size_t n = p - l->s;
        if (n==3 && !strncmp(l->s,"nil",3)) {
            a_set(dst, a_nil());
        } else if (n==4 && !strncmp(l->s,"true",4)) {
            a_set(dst, a_bool(1));
        } else if (n==5 && !strncmp(l->s,"false",5)) {
            a_set(dst, a_bool(0));
        } else if (l->e != NULL) {
            // a copy of the name: no table is shared
            Name* nm = name_get(l, l->s, n);
            if (nm == NULL) {
                return failn(l, "undefined name", l->s, n);
            }
            if (l->depth+nm->depth > DEPTH_MAX) {
                return fail(l, "too many nested tables");
            }
            if (l->depth+nm->depth > l->top) {
                l->top = l->depth + nm->depth;
            }
            AVal v = a_nil();
            if (a_xcopy(&v, nm->v) != A_OK) {
                return A_ERROR;
            }
            a_set(dst, v);
        } else {
            return fail(l, "unexpected name : expected value");
        }
        l->s = p;
        return A_OK;
    }
    return fail(l, "expected value");
}

static AStatus names (Load* l);

// Removes `.` and `dir/..` from a path, in place, so that a file reached
// in two ways is known as the same.
static void path_norm (char* path) {
    char* parts[64];
    int n = 0;
    int abs = (path[0] == '/');
    char* out = path + abs;
    char* p = path + abs;
    int more = 1;
    while (more) {
        char* q = p;
        while (*q!='\0' && *q!='/') {
            q++;
        }
        size_t len = q - p;
        more = (*q == '/');     // read now: the copy may write over it
        char* nxt = q + 1;
        if (len==0 || (len==1 && p[0]=='.')) {
            // nothing
        } else if (len==2 && p[0]=='.' && p[1]=='.' && n>0 &&
                   !(out-parts[n-1]==3 && parts[n-1][0]=='.' && parts[n-1][1]=='.'))
        {
            out = parts[--n];
        } else if (n < 64) {
            parts[n++] = out;
            memmove(out, p, len);
            out += len;
            *out++ = '/';
        }
        p = nxt;
    }
    if (n > 0) {
        out--;
    }
    *out = '\0';
}

// After `include`, at the string: reads the definitions of the file.
static AStatus include (Load* l) {
    Env* e = l->e;
    AVal v = a_nil();
    if (string(l,&v) != A_OK) {
        a_dec(v);
        return A_ERROR;
    }
    AStr* f = A_STR_OF(v);

    // dir/file, unless the file is absolute
    size_t dn = (l->dir==NULL || (f->len>0 && f->s[0]=='/')) ? 0 : strlen(l->dir);
    char* path = malloc(dn + 1 + f->len + 2);
    size_t n = 0;
    if (dn > 0) {
        memcpy(path, l->dir, dn);
        n = dn;
        if (path[n-1] != '/') {
            path[n++] = '/';
        }
    }
    memcpy(path+n, f->s, f->len);
    n += f->len;
    path[n] = '\0';
    a_dec(v);
    path_norm(path);
    if (path[0] == '\0') {
        strcpy(path, ".");
    }
    n = strlen(path);

    // only once
    for (int i=0; i<e->files_n; i++) {
        if (!strcmp(e->files[i].path, path)) {
            free(path);
            return A_OK;
        }
    }

    FILE* fp = fopen(path, "rb");
    if (fp == NULL) {
        AStatus st = failn(l, "include not found", path, n);
        free(path);
        return st;
    }
    char* buf = NULL;
    size_t len = 0;
    size_t max = 0;
    while (1) {
        if (len == max) {
            max = max*2 + 1024;
            buf = realloc(buf, max);
        }
        size_t r = fread(buf+len, 1, max-len, fp);
        if (r == 0) {
            break;
        }
        len += r;
    }
    // a directory opens, and fails to be read
    int err = ferror(fp);
    fclose(fp);
    if (err) {
        AStatus st = failn(l, "include not found", path, n);
        free(buf);
        free(path);
        return st;
    }

    // the names point into the text: it lasts as long as they do
    if (e->files_n == e->files_max) {
        e->files_max = e->files_max*2 + 8;
        e->files = realloc(e->files, e->files_max * sizeof(File));
    }
    e->files[e->files_n++] = (File) { path, buf };

    // its own includes are relative to it
    char* dir = NULL;
    const char* bar = strrchr(path, '/');
    if (bar != NULL) {
        size_t m = (bar==path) ? 1 : (size_t)(bar - path);
        dir = malloc(m + 1);
        memcpy(dir, path, m);
        dir[m] = '\0';
    }

    Load l2 = { buf, buf+len, 1, 0, 0, e, path, dir };
    AStatus st = names(&l2);
    if (st==A_OK && l2.s<l2.end) {
        st = fail(&l2, "expected definition");
    }
    free(dir);
    return st;
}

// X.loadx: `ID = Value` and `include STR` before the value.
static AStatus names (Load* l) {
    while (1) {
        if (!skip(l)) {
            return fail(l, "unterminated comment");
        }
        if (!(l->s<l->end && is_id1(*l->s))) {
            return A_OK;
        }
        const char* p = l->s;
        while (p<l->end && is_id(*p)) {
            p++;
        }
        const char* q = p;
        while (q<l->end && (*q==' ' || *q=='\t')) {
            q++;
        }
        const char* s = l->s;
        size_t n = p - s;
        if (n==7 && !strncmp(s,"include",7) && q<l->end && (*q=='"' || *q=='\'')) {
            l->s = q;
            if (include(l) != A_OK) {
                return A_ERROR;
            }
            continue;
        }
        if (!(q<l->end && *q=='=')) {
            return A_OK;
        }
        if ((n==7 && !strncmp(s,"include",7)) || (n==3 && !strncmp(s,"nil",3)) || (n==4 && !strncmp(s,"true",4)) ||
            (n==5 && !strncmp(s,"false",5)))
        {
            return fail(l, "invalid name");
        }
        l->s = q + 1;
        l->top = 0;
        AVal v = a_nil();
        if (value(l,&v) != A_OK) {
            a_dec(v);
            return A_ERROR;
        }
        if (name_get(l,s,n) != NULL) {
            // the first definition wins: this one is read and ignored
            a_dec(v);
            continue;
        }
        Env* e = l->e;
        if (e->names_n == e->names_max) {
            e->names_max = e->names_max*2 + 8;
            e->names = realloc(e->names, e->names_max * sizeof(Name));
        }
        e->names[e->names_n++] = (Name) { s, n, v, l->top };
    }
}

// Reads the single value in `s[0..len)`, after its names if `x`, with
// the includes in `dir`.
static AStatus load (const char* s, size_t len, int x, const char* dir) {
    Env e = { NULL, 0, 0, NULL, 0, 0 };
    Load l = { s, s+len, 1, 0, 0, x ? &e : NULL, NULL, dir };
    AVal v = a_nil();
    AStatus st = x ? names(&l) : A_OK;
    if (st == A_OK) {
        st = value(&l, &v);
    }
    if (st==A_OK && !skip(&l)) {
        st = fail(&l, "unterminated comment");
    }
    if (st==A_OK && l.s<l.end) {
        st = fail(&l, "expected end of input");
    }
    for (int i=0; i<e.names_n; i++) {
        a_dec(e.names[i].v);
    }
    free(e.names);
    for (int i=0; i<e.files_n; i++) {
        free(e.files[i].path);
        free(e.files[i].buf);
    }
    free(e.files);
    if (st == A_OK) {
        a_push(v);
    } else {
        a_dec(v);
    }
    return st;
}

// X.load(s): the value written in the string `s`.
static AStatus x_load (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AStr* s;
    if (arg_str(argc, argv, 0, "load", &s) != A_OK) {
        return A_ERROR;
    }
    return load(s->s, s->len, 0, NULL);
}

// X.loadx([dir,] s): the same, with names and includes before the
// value. The includes are in `dir`, or in the current directory.
static AStatus x_loadx (AFunc* self, int argc, AVal* argv) {
    (void) self;
    AStr* d = NULL;
    AStr* s;
    if (argc>=2 && arg_str(argc, argv, 0, "loadx", &d)!=A_OK) {
        return A_ERROR;
    }
    if (arg_str(argc, argv, argc>=2, "loadx", &s) != A_OK) {
        return A_ERROR;
    }
    char* dir = NULL;
    if (d!=NULL && d->len>0) {
        dir = malloc(d->len + 1);
        memcpy(dir, d->s, d->len);
        dir[d->len] = '\0';
    }
    AStatus st = load(s->s, s->len, 1, dir);
    free(dir);
    return st;
}

///////////////////////////////////////////////////////////////////////////////
// DUMP
///////////////////////////////////////////////////////////////////////////////

// X.dump writes what X.load reads: `X.load(X.dump(v))` is structurally
// equal to `v`. Only data can be written: nil, booleans, numbers,
// strings, and tables of them, with no cycles.

typedef struct Dump {
    Buf   b;
    ATab* path[DEPTH_MAX];
    int   depth;
} Dump;

static void dstr (Dump* d, const char* s) {
    buf_add(&d->b, s, strlen(s));
}

static AStatus dfail (const char* msg) {
    return a_error("invalid dump : %s", msg);
}

// A name that `X.load` reads as a field: `id=`.
static int is_field (AVal k) {
    if (k.type!=A_STR || A_STR_OF(k)->len==0 || !is_id1(A_STR_OF(k)->s[0])) {
        return 0;
    }
    for (uint32_t i=0; i<A_STR_OF(k)->len; i++) {
        if (!is_id(A_STR_OF(k)->s[i])) {
            return 0;
        }
    }
    return 1;
}

// A string that `X.load` reads as a tag: `:X.Y`.
static int is_tag (AVal v) {
    if (v.type!=A_STR || A_STR_OF(v)->len==0) {
        return 0;
    }
    for (uint32_t i=0; i<A_STR_OF(v)->len; i++) {
        char c = A_STR_OF(v)->s[i];
        if (!is_id(c) && c!='.') {
            return 0;
        }
    }
    return 1;
}

static AStatus dump (Dump* d, AVal v) {
    char num[64];
    switch (v.type) {
        case A_NIL:
            dstr(d, "nil");
            return A_OK;
        case A_BOOL:
            dstr(d, v.u.b ? "true" : "false");
            return A_OK;
        case A_INT:
            snprintf(num, sizeof(num), "%lld", (long long)v.u.i);
            dstr(d, num);
            return A_OK;
        case A_NUM:
            if (v.u.n != v.u.n) {
                return dfail("unexpected nan");
            }
            if (v.u.n-v.u.n != 0) {
                // an infinity: reads back as one
                dstr(d, (v.u.n > 0) ? "1e999" : "-1e999");
                return A_OK;
            }
#ifdef ATM_FLOAT_32
            snprintf(num, sizeof(num), "%.9g", (double)v.u.n);
#else
            snprintf(num, sizeof(num), "%.17g", v.u.n);
#endif
            dstr(d, num);
            // a float must read back as a float
            if (strpbrk(num, ".eE") == NULL) {
                dstr(d, ".0");
            }
            return A_OK;
        case A_STR: {
            AStr* s = A_STR_OF(v);
            dstr(d, "\"");
            for (uint32_t i=0; i<s->len; i++) {
                unsigned char c = (unsigned char) s->s[i];
                switch (c) {
                    case '\n': dstr(d, "\\n");  break;
                    case '\t': dstr(d, "\\t");  break;
                    case '\r': dstr(d, "\\r");  break;
                    // the language has no escape for the delimiter
                    case '"':  dstr(d, "\\x22"); break;
                    case '\\': dstr(d, "\\\\"); break;
                    default:
                        if (c < 32 || c == 127) {
                            snprintf(num, sizeof(num), "\\x%02X", c);
                            dstr(d, num);
                        } else {
                            buf_add(&d->b, (char*)&c, 1);
                        }
                        break;
                }
            }
            dstr(d, "\"");
            return A_OK;
        }
        case A_TAB:
            break;
        default:
            return a_error("invalid dump : unexpected %s", a_typename(v));
    }

    // table
    ATab* t = A_TAB_OF(v);
    for (int i=0; i<d->depth; i++) {
        if (d->path[i] == t) {
            return dfail("unexpected cycle");
        }
    }
    if (d->depth == DEPTH_MAX) {
        return dfail("too many nested tables");
    }
    d->path[d->depth++] = t;

    // :X [...]
    AVal ktag = a_strz("tag");
    AVal tag = a_tab_get(t, ktag);
    int tagged = is_tag(tag);
    if (tagged) {
        dstr(d, ":");
        buf_str(&d->b, tag);
        dstr(d, " ");
    }
    dstr(d, "[");

    AStatus st = A_OK;
    uint32_t pos = 0;
    int64_t  nxt = 0;
    int first = 1;
    AVal k, x;
    while (st==A_OK && a_tab_next(t, &pos, &k, &x)) {
        if (tagged && k.type==A_STR && k.u.o==ktag.u.o) {
            continue;
        }
        if (!first) {
            dstr(d, ", ");
        }
        first = 0;
        if (k.type==A_INT && k.u.i==nxt) {
            // positional: numbered by `X.load` in the same way
            nxt++;
        } else if (is_field(k)) {
            buf_str(&d->b, k);
            dstr(d, "=");
        } else {
            dstr(d, "@(");
            st = dump(d, k);
            dstr(d, ")=");
        }
        if (st == A_OK) {
            st = dump(d, x);
        }
    }
    a_dec(ktag);
    dstr(d, "]");
    d->depth--;
    return st;
}

// X.dump(v): the value as a text that X.load reads.
static AStatus x_dump (AFunc* self, int argc, AVal* argv) {
    (void) self;
    Dump* d = calloc(1, sizeof(Dump));
    AStatus st = dump(d, arg(argc,argv,0));
    if (st == A_OK) {
        a_push(buf_val(&d->b));
    } else {
        free(d->b.s);
    }
    free(d);
    return st;
}

// The text of `X.dump(v)`, for a message (owned), or NULL if the value
// cannot be written. A failure leaves an error pending: who calls
// must not have one to keep.
char* a_dump_text (AVal v) {
    Dump* d = calloc(1, sizeof(Dump));
    char* s = NULL;
    if (dump(d,v) == A_OK) {
        s = malloc(d->b.n + 1);
        memcpy(s, d->b.s, d->b.n);
        s[d->b.n] = '\0';
    }
    free(d->b.s);
    free(d);
    return s;
}

void atm_lib_load_init (void) {
    field(atm_g_X, "load", x_load);
    field(atm_g_X, "loadx", x_loadx);
    field(atm_g_X, "dump", x_dump);
}
