CC     ?= cc
CFLAGS ?= -O2 -g -Wall -Wextra
ASAN   := -fsanitize=address -fno-omit-frame-pointer

RT_SRC := rt/val.c rt/str.c rt/tab.c rt/ops.c rt/task.c rt/std.c \
          rt/lstring.c rt/lvector.c rt/los.c rt/lio.c \
          rt/lload.c
RT_HDR := rt/atmos.h rt/lib.h

CC_SRC := src/aux.c src/lexer.c src/parser.c src/ast.c src/resolver.c \
          src/coder.c src/main.c
CC_HDR := src/atmc.h

# The runtime, once per configuration of the numeric types, and once
# more with the address sanitizer: `libatmos[-f32][-i32][-ms][-asan].a`
# (`ATMOS_REAL=float`, `ATMOS_INT=32`, `ATMOS_TIME=ms`, `ATMOS_ASAN`).
# `_` stands for an option that is off.
CFGS := $(foreach r,_ -f32,$(foreach i,_ -i32,$(foreach t,_ -ms,\
            $(foreach a,_ -asan,$(subst _,,$(r)$(i)$(t)$(a))+))))
CFGS := $(patsubst %+,%,$(CFGS))
# Only the default configuration is built in advance: the compiler
# builds another one when a program first asks for it, and those that
# exist are kept up to date.
LIBS := $(sort build/libatmos.a build/libatmos-asan.a \
               $(wildcard build/libatmos-*.a))

all: atmos $(LIBS)

atmos: $(CC_SRC) $(CC_HDR)
	$(CC) $(CFLAGS) -DATMOS_HOME='"$(CURDIR)"' -Isrc $(CC_SRC) -o $@

# A compiler dedicated to an environment, such as `atmos-pico`: this
# compiler, with the file of the environment included before every
# program, and with the flags that the environment needs. Paths must be
# absolute, and the flags must have no quotes.
#
# `ENV_NUM` lists the numeric options that the environment supports
# beyond the defaults, as in ENV_NUM="float 32 ms" (`ATMOS_REAL=float`,
# `ATMOS_INT=32`, `ATMOS_TIME=ms`): the compiler refuses the others.
#
#   make -C <lang> env OUT=<dir>/atmos-pico ENV=<dir>/pico.atm \
#        ENV_CFLAGS="-I<dir>/c/src" ENV_LDFLAGS="<dir>/c/libpico.a -lSDL2"
env: $(CC_SRC) $(CC_HDR) $(LIBS)
	@test -n "$(OUT)" -a -n "$(ENV)" || \
	    { echo "usage: make env OUT=<executable> ENV=<file.atm> [ENV_CFLAGS=...] [ENV_LDFLAGS=...] [ENV_NUM=...]"; exit 1; }
	$(CC) $(CFLAGS) -DATMOS_HOME='"$(CURDIR)"' \
	    -DATMOS_NAME='"$(notdir $(OUT))"' \
	    -DATMOS_ENV='"$(abspath $(ENV))"' \
	    -DATMOS_ENV_CFLAGS='"$(ENV_CFLAGS)"' \
	    -DATMOS_ENV_LDFLAGS='"$(ENV_LDFLAGS)"' \
	    -DATMOS_ENV_NUM='"$(ENV_NUM)"' \
	    -Isrc $(CC_SRC) -o $(OUT)

# $(1): the suffix of a configuration
define RT_RULES
build/rt$(1)/%.o: rt/%.c $$(RT_HDR)
	@mkdir -p build/rt$(1)
	$$(CC) $$(CFLAGS) \
	    $$(if $$(findstring -asan,$(1)),$$(ASAN)) \
	    $$(if $$(findstring -f32,$(1)),-DATM_REAL_FLOAT) \
	    $$(if $$(findstring -i32,$(1)),-DATM_INT_32) \
	    $$(if $$(findstring -ms,$(1)),-DATM_TIME_MS) \
	    -Irt -c $$< -o $$@

build/libatmos$(1).a: $$(RT_SRC:rt/%.c=build/rt$(1)/%.o) build/rt$(1)/main.o
	ar rcs $$@ $$^
endef

# the default configuration has an empty suffix
$(eval $(call RT_RULES,))
$(foreach c,$(CFGS),$(eval $(call RT_RULES,$(c))))

# runtime unit tests, built with the address sanitizer
RT_TST := $(wildcard tst/rt/*.c)

test-rt: $(RT_TST:tst/rt/%.c=build/tst/rt/%)
	@for t in $^; do echo "$$t"; $$t || exit 1; done

build/tst/rt/%: tst/rt/%.c $(RT_SRC) $(RT_HDR)
	@mkdir -p build/tst/rt
	$(CC) $(CFLAGS) $(ASAN) -Irt $< $(RT_SRC) -lm -o $@

test-lex: atmos
	tst/run.sh lex

test-ast: atmos
	tst/run.sh ast

test-cases: all
	tst/run.sh cases

test-golden: all
	tst/run.sh golden

test-env: all
	tst/env/run.sh

test: test-rt test-lex test-ast test-cases test-golden test-env

clean:
	rm -rf build

.PHONY: all env test test-rt test-lex test-ast test-cases test-golden test-env clean
