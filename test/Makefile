CC=clang
CFLAGS=-Wall -g -I..
LDLIBS=../asa.o -lfftw3 -lm

EXES=window power
DEP=$(SRC:.c=.d)

-include $(DEP)

all: test

.PHONY: clean test
clean:
	$(RM) *.o *.d $(EXES)
	$(RM) -r *.dSYM

test: $(EXES)
	run_test.sh

%: %.o ../asa.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.d: %.c
	$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

%.o: %.d

