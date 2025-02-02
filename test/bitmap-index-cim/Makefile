TARGETS = main-cim-8-2 main-cim-8-3 main-cim-8-4 main-cim-16-2 main-cim-16-3 main-cim-16-4
TARGET = $(TARGETS)
include ../shared/Makefile.shared

CXX = g++
#CC = gcc
#CFLAGS = -g -O2 -std=c99 -pthread -I/home/sonlm/workplace/DRAM-CIM/Ours/fastbit-2.0.3/src
CXXFLAGS = -g -O2 -std=c++11 -pthread -I/home/sonlm/workplace/DRAM-CIM/Ours/fastbit-2.0.3/src
CXXFLAGS += -L/home/sonlm/workplace/DRAM-CIM/Ours/fastbit-2.0.3/src/.libs

all: $(TARGETS)

$(TARGETS): %: %.o
	$(CXX) $(CXXFLAGS) $< -lfastbit -lm $(SNIPER_LDFLAGS) -o $@

run_all: clean all
	@for target in $(TARGETS); do \
		for speed in 1866 2400 3200; do \
			for pim in ambit pipf roc elp2im ours; do \
				../../run-sniper -n 2 -c gainestown-$$pim-ddr4-$$speed -d ./out-$$target-$$pim-ddr4-$$speed --power --roi -- ./$$target > $$target-$$pim-ddr4-$$speed.txt; \
			done \
		done \
	done
