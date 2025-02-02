TARGET = cim-test-malloc
include ../shared/Makefile.shared
CXX = g++ 
CXXFLAGS = -O3 -mavx -fopt-info-vec-optimized
CXXFLAGS = -Wall -Wextra -Werror

$(TARGET): $(TARGET).o 
	$(CXX) $(CXXFLAGS) $(TARGET).o -lm $(SNIPER_LDFLAGS) -o $(TARGET) 

run_$(TARGET):
	../../run-sniper -n 2 -c gainestown-elp2im -d ./out-19-12-elp2im --power --roi -- ./cim-test-malloc > 19-12-elp2im.txt
