#include "/home/sonlm/workplace/DRAM-CIM/Ours/sniper/include/sim_api.h"

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
//#include <bitset>
#include <vector>
#include <boost/dynamic_bitset.hpp>
//#include <llvm/ADT/BitVector.h>

using namespace std;

int main() {
    
    const int size_total = 16;
    const int total = pow(2, size_total);
    const int size_bits = 19;
    const int bits = pow(2, size_bits);

    vector< boost::dynamic_bitset<> > src;
    boost::dynamic_bitset<> dst(bits, 0);

    cout << "Initializing dataset!" << endl;
    
    for (int i = 0; i < total; i++)
    {
        src.push_back(boost::dynamic_bitset<> (bits, i));
    }

    cout << "Initialized dataset!" << endl;

    SimRoiStart();

    dst = src[0]; 
    // calculate the bitwise operation
    for (int i = 1; i < total; i++)
    {
        // for baseline sequential access
        dst &= src[i];
    }
    //FIXMEcout << dst << endl;
    
    SimRoiEnd();
    
    cout << "Test is done!" << endl;

    return 0;
}
