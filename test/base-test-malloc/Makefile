TARGET = base-test-malloc
include ../shared/Makefile.shared
CXX = g++ 
CXXFLAGS = -O3 -mavx -fopt-info-vec-optimized
#CXXFLAGS = -std=c++11 -Wall -Wextra -Werror

$(TARGET): $(TARGET).o 
	$(CXX) $(CXXFLAGS) $(TARGET).o -lm $(SNIPER_LDFLAGS) -o $(TARGET) 

run_$(TARGET):
	../../run-sniper -n 2 -c gainestown -d ./out-19-16 --power --roi -- ./base-test-malloc
