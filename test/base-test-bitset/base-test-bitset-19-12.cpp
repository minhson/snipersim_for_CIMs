#include "/home/sonlm/workplace/DRAM-CIM/Ours/sniper/include/sim_api.h"

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <bitset>
//#include <array>
#include <vector>
//#include <boost/dynamic_bitset.hpp>
//#include <llvm/ADT/BitVector.h>

using namespace std;

int main() {
    
    const int size_total = 12;
    const int total = pow(2, size_total);
    const int size_bits = 19;
    const int bits = pow(2, size_bits);

    bitset< bits > dst(0);
    vector< bitset<bits> > src; 
    
    cout << "Initializing dataset!" << endl;
    
    for (int i = 0; i < total; i++)
    {
        src.push_back(bitset<bits>(i));
    }

    cout << "Initialized dataset!" << endl;

    SimRoiStart();
    
    // calculate the bitwise operation
    dst = src[0];
    for (int i = 1; i < total; i++)
    {
        // for baseline sequential access
        dst &= src[i];
    }
    
    SimRoiEnd();
    
    cout << "Test is done!" << endl;

    return 0;
}
