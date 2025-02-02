#include "/home/sonlm/workplace/DRAM-CIM/Ours/sniper/include/sim_api.h"

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#include <iostream>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <bitset>
//#include <array>
#include <vector>
//#include <boost/dynamic_bitset.hpp>
//#include <llvm/ADT/BitVector.h>

using namespace std;

int main() {
    
    const int size_total = 16;
    const int total = pow(2, size_total);
    const int size_bits = 15;
    const int bits = pow(2, size_bits);

    int size_in_byte = bits / 8; // size of bitvector in byte
    int row_buffer_size = pow(2, 16) / 8; // row buffer size in byte (8KB)
    int split = ceil((double)size_in_byte / row_buffer_size); // split bitvector that is larger than row buffer size

    bitset< bits > dst(0);
    vector< bitset<bits> > src; 
    
    cout << "Initializing dataset!" << endl;
    
    for (int i = 0; i < total; i++)
    {
        src.push_back(bitset<bits>(i));
    }

    cout << "Initialized dataset!" << endl;
    
    SimRoiStart();
   
    CLINVAL((uint64_t) &dst, (uint64_t) (row_buffer_size*split));
    
    // dst = src[0] & src[1]
    AND((uint64_t) &dst, (uint64_t) (64*split));
    for (int i = 2; i < total; i++)
    {
        // dst &= src[i]
        AND_S((uint64_t) &src[i], (uint64_t) (64*split));
    }
    
    READ((uint64_t) &dst, (uint64_t) (row_buffer_size*split));
    
    SimRoiEnd();

    cout << "Test is done!" << endl;

    return 0;
}
