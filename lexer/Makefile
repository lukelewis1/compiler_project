CC     ?= clang
CFLAGS  = -Wall -Wextra -std=c11
SRC     = lexer.c

.PHONY: all clean test test-unit test-io

all: lexer

# Platform-specific linker flags for size optimisation
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
    LDFLAGS = -s -Wl,-z,max-page-size=4096
else
    # macOS: strip symbols as a post-build step instead
    LDFLAGS =
endif

lexer: $(SRC)
	$(CC) $(CFLAGS) -Os -flto $(LDFLAGS) -o lexer $(SRC)
ifeq ($(UNAME), Darwin)
	strip lexer
endif

unit_tests: tests/test_unit.c $(SRC)
	$(CC) $(CFLAGS) -o unit_tests tests/test_unit.c

test: test-unit test-io

test-unit: unit_tests
	./unit_tests

test-io: lexer
	./tests/run_io_tests.sh ./lexer

clean:
	rm -f lexer unit_tests tokens.txt
