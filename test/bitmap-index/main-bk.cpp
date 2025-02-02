#include <iostream>
#include "bitvector.h"

int main()
{
    int fdes = open("eventTime.idx", O_RDONLY);
    uint32_t* offset = NULL;
    char head[8];
    
    ssize_t ret;
    // read header
    ret = read(fdes, head, 8);
    if (ret == -1) {
        // handle error
    } else if (ret != 8) {
        // handle unexpected number of bytes read
    }
    
    uint32_t len[2];
    
    // read length
    ret = read(fdes, len, 8);
    if (ret == -1) {
        // handle error
    } else if (ret != 8) {
        // handle unexpected number of bytes read
    }
    
    offset = (uint32_t*) malloc (sizeof(uint32_t)*(len[1]+1));
    
    // read offset
    ret = read(fdes, offset, sizeof(uint32_t)*(len[1]+1));
    if (ret == -1) {
        // handle error
    } else if (ret != sizeof(uint32_t)*(len[1]+1)) {
        // handle unexpected number of bytes read
    }

    // read bitmap
    int i = 0;
    ibis::array_t<ibis::bitvector::word_t> a0(fdes, offset[i], offset[i+1]);
    ibis::bitvector* bv = new ibis::bitvector(a0);
    bv->sloppySize(len[0]);

    std::cout << "Size of bitvector: " << bv->size() << std::endl;
    
    close(fdes);
    return 0;
}
