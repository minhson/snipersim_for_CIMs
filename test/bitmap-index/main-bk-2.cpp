#include <iostream>
#include "bitvector.h"
#include "ibis.h"
#include "index.h"
//#include "iapi.h"

int main()
{
    //FIXMEibis::bitvector bv;
    //FIXMEbv.read("eventTime.idx");
    //FIXMEbv.print(std::cout);
    //FIXMEstd::cout << "size of bv: " << bv.size() << std::endl;
   
    //FIXMEif (bv.isCompressed())
    //FIXME    std::cout << "bv is compressed" << std::endl;
    //FIXMEelse
    //FIXME    std::cout << "bv is decompressed" << std::endl;
    //FIXME
    //FIXME// decompress bv to literal word
    //FIXMEbv.decompress();
    //FIXME
    //FIXMEif (bv.isCompressed())
    //FIXME    std::cout << "bv is compressed" << std::endl;
    //FIXMEelse
    //FIXME    std::cout << "bv is decompressed" << std::endl;
   
    const char *dir = "/home/sonlm/workplace/DRAM-CIM/Ours/fastbit-2.0.3/tests/facebook"; // pointer to a FastBit data partition
    ibis::part *mypart = new ibis::part(dir);
    const char *colname = "Date8";
    ibis::column *col = mypart->getColumn(colname);
    
    if (col != 0) {
        ibis::index *idx = ibis::index::create(col, 0, 0, 1);
        if (idx != 0) {
            // idx is a pointer to the newly created index
            // Returns the number of bit vectors used by the index 
            std::cout << "number of bit vectors used by the index: " << idx->numBitvectors() << std::endl;
            std::cout << "size in bytes: " << idx->sizeInBytes() << std::endl;
            // Prints human readable information.  Outputs information about the
            // index as text to the specified output stream.
            idx->print(std::cout);
           
            ibis::qKeyword myQuery(colname, "No"); // create a query object for searching column for keyword
            //ibis::qContinuousRange myQuery(colname, ibis::qExpr::OP_EQ, (double)2.76149e+06); // create a query
            ibis::bitvector hits; // create a bitvector to store the hits
            //long numHits = idx->evaluate(myQuery, hits); // evaluate the query
            long numHits = mypart->keywordSearch(myQuery, hits); // perform the keyword search
            if (numHits >= 0) {
                // the query was successful
                // the hits are stored in the 'hits' bitvector
                std::cout << "the query was successful" << std::endl;
                std::cout << "numHits: " << numHits << std::endl;
            } else {
                // an error occurred while evaluating the query
                std::cout << "an error occurred while evaluating the query" << std::endl;
            }
            
            if (hits.isCompressed())
                std::cout << "hits is compressed" << std::endl;
            else
                std::cout << "hits is decompressed" << std::endl;
            std::cout << "number of bits: " << hits.size() << std::endl;
            std::cout << "size in bytes of hits: " << hits.bytes() << std::endl;
            
            // decompress bv to literal word
            hits.decompress();
            
            if (hits.isCompressed())
                std::cout << "hits is compressed" << std::endl;
            else
                std::cout << "hits is decompressed" << std::endl;       
            std::cout << "size in bytes of hits: " << hits.bytes() << std::endl;
            
            delete idx;
        }
    }

    return 0;
}
