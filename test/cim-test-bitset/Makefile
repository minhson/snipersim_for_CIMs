#FIXMETARGETS = cim-test-bitset-15-12 cim-test-bitset-15-14 cim-test-bitset-15-16 cim-test-bitset-19-12 cim-test-bitset-19-16
TARGETS = cim-test-bitset-15-16
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
		for speed in 2400; do \
			for pim in ambit pipf roc elp2im ours; do \
				../../run-sniper -n 2 -c gainestown-$$pim-ddr4-$$speed -d ./out-$$target-$$pim-ddr4-$$speed --power --roi -- ./$$target > $$target-$$pim-ddr4-$$speed.txt; \
			done \
		done \
	done
