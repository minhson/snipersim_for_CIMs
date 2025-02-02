#FIXMETARGETS = base-test-bitset-15-12 base-test-bitset-15-14 base-test-bitset-15-16 base-test-bitset-19-12 base-test-bitset-19-16
TARGETS = base-test-bitset-19-12
TARGET = $(TARGETS)
include ../shared/Makefile.shared

CXX = g++
CXXFLAGS = -g -O2 -mavx -fopt-info-vec-optimized
#CXXFLAGS += -std=c++11 -Wall -Wextra -Werror

all: $(TARGETS)

$(TARGETS): %: %.o
	$(CXX) $(CXXFLAGS) $< -lm $(SNIPER_LDFLAGS) -o $@

#FIXMEfor speed in 1866 2400 3200;
run_all: clean all
	@for target in $(TARGETS); do \
		for speed in 3200; do \
			../../run-sniper -n 2 -c gainestown-ddr4-$$speed -d ./out-$$target-ddr4-$$speed --power --roi -- ./$$target > $$target-ddr4-$$speed.txt; \
		done \
	done
