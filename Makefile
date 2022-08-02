# Set you prefererred CFLAGS/compiler compiler here.
# Our github runner provides gcc-10 by default.
CC ?= cc
CFLAGS ?= -g -Wall -O2
CXX ?= c++
CXXFLAGS ?= -g -std=c++11 -Wall -O2 -D cppext
CARGO ?= cargo
RUSTFLAGS ?= -g

# this target should build all executables for all tests
# all:
# 	@echo "Please set a concrete build command here"
# 	false

.PHONY: all clean check

## Rust Example
#all:
#	cargo build

## C/C++ example
all: libcspinlock.so liblockhashmap.so liblockfreehashmap.so
libcspinlock.so: cspinlock.c
#$(CXX) $(CXXFLAGS) -shared -fPIC -ldl -o $@ $<
	$(CC) $(CFLAGS) -shared -fPIC -ldl -o $@ $<

liblockhashmap.so: lockhashmap.c
#$(CXX) $(CXXFLAGS) -shared -fPIC -ldl -o $@ $<
	$(CC) $(CFLAGS) -shared -fPIC -ldl -o $@ $<

liblockfreehashmap.so: lockfreehashmap.c
#$(CXX) $(CXXFLAGS) -shared -fPIC -ldl -o $@ $<
	$(CC) $(CFLAGS) -shared -fPIC -ldl -o $@ $<

# Usually there is no need to modify this
check: all
	$(MAKE) -C tests check

clean:
	$(MAKE) -C tests clean
	rm -rf *.so* *.o