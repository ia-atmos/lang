// Generic driver, used when the program defines no `main` of its own:
// runs the main chunk, then drives the environments registered with
// `atmos.env` (see atmos.h), or, with none, lets time pass by the clock
// until the program terminates or nothing can awake it.

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <time.h>

#include "atmos.h"

static double now_us (void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec*1e6 + ts.tv_nsec/1e3;
}

// The clock, with no environment: sleeps until the nearest timer and
// delivers the time elapsed.
static AStatus run_clock (AStatus st) {
    while (st==A_OK && atm_alive()) {
        double us = atm_next();
        if (us < 0) {
            break;
        }
        fflush(stdout);
        double t0 = now_us();
        struct timespec ts;
        ts.tv_sec  = (time_t) (us / 1e6);
        ts.tv_nsec = (long) ((us - ts.tv_sec*1e6) * 1e3);
        nanosleep(&ts, NULL);
        double dt = now_us() - t0;
        st = atm_emit(a_num((dt > us) ? dt : us));
    }
    return st;
}

// The function `name` of the environment `i`, or nil.
static AVal env_fn (int i, const char* name) {
    AVal k = a_strz(name);
    AVal f = a_tab_get(A_TAB_OF(atm_envs[i]), k);
    a_dec(k);
    return f;
}

// Calls `name` of the environment `i`, if it has one: `*end` tells
// whether it gave a true value.
static AStatus env_call (int i, const char* name, int* end) {
    *end = 0;
    AVal f = env_fn(i, name);
    if (f.type == A_NIL) {
        return A_OK;
    }
    int base = a_top;
    atm_step_begin();
    AStatus st = atm_step_end(a_call(f, base));
    if (st == A_OK) {
        *end = (a_top > base) && a_truthy(a_stk[base]);
    }
    a_settop(base);
    return st;
}

// The environments: each `step`, in order, until one gives true or the
// main chunk is no longer pending.
static AStatus run_envs (AStatus st) {
    int end = 0;
    while (st==A_OK && !end && atm_alive()) {
        for (int i=0; i<atm_nenvs && st==A_OK && !end; i++) {
            st = env_call(i, "step", &end);
        }
        fflush(stdout);
    }
    return st;
}

static AStatus fail (AStatus st) {
    fflush(stdout);
    if (st == A_ESCAPE) {
        st = a_escape_err();
    }
    if (st != A_OK) {
        a_error_dump();
    }
    return st;
}

int main (int argc, char** argv) {
    atm_init(argc, argv);
    AStatus st = atm_main();

    // one environment with no step: the host drives the program (a
    // browser), which stays alive after this returns
    if (st==A_OK && atm_nenvs==1 && env_fn(0,"step").type==A_NIL) {
        fflush(stdout);
        return 0;
    }

    st = (atm_nenvs == 0) ? run_clock(st) : run_envs(st);

    // the program ended, or nothing can awake the main chunk: it is
    // aborted
    if (st == A_OK) {
        st = atm_stop();
    }
    st = fail(st);

    // the environments quit in reverse order, also after an error,
    // inside `atm_quit`, after the values of the program are released
    fflush(stdout);
    atm_quit();
    return (st == A_OK) ? 0 : 1;
}
