TESTS=trivial

VPATH=../../doc


all: $(TESTS:%=%_example) $(TESTS:%=%_test)
clean:
	@rm -f $(TESTS:%=%_example) $(TESTS:%=%_test)

.PHONY: all clean

CFLAGS=$(shell pkg-config --cflags opus)
LDLIBS=$(shell pkg-config --libs opus)
infile=input.wav

%_test: %_example
	./$^ $(infile) 2>&1 >$@
trivial_test: trivial_example
	./$^ $(infile) $@
