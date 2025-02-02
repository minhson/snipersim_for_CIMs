#include "/home/sonlm/workplace/DRAM-CIM/Ours/sniper/include/sim_api.h"
 
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#include <iostream>
#include <string>
#include "bitvector.h"
#include "ibis.h"
#include "index.h"
//#include "iapi.h"

int main()
{
    int weeks = 4;
    int users = 8000000; // 8 million or 16 million users
    
    std::cout << "Number of weeks: " << weeks << std::endl;
    std::cout << "Number of users: " << users << std::endl;

    const char *dir;
    if (users == 8000000)
        dir = "/home/sonlm/workplace/DRAM-CIM/Ours/fastbit-2.0.3/tests/facebook-8"; // pointer to a FastBit data partition
    else
        dir = "/home/sonlm/workplace/DRAM-CIM/Ours/fastbit-2.0.3/tests/facebook-16"; // pointer to a FastBit data partition
    
    ibis::part *mypart = new ibis::part(dir);
   
    // Gender
    const char *colGender = "Gender";
    std::string colGenderIdx = std::string(dir) + "/" + std::string(colGender) + ".idx";
    ibis::column *genderCol = mypart->getColumn(colGender);
    ibis::bitvector males; // create a bitvector to store the males
    if (genderCol != 0) {
        ibis::index *genderIdx = ibis::index::create(genderCol, colGenderIdx.c_str(), 0, 1);
        if (genderIdx != 0){
            // gender is a pointer to the newly created gender index 
            // Returns the number of bit vectors used by the index 
            std::cout << "number of bit vectors used by the gender: " << genderIdx->numBitvectors() << std::endl;
            std::cout << "size in bytes: " << genderIdx->sizeInBytes() << std::endl;
            // Prints human readable information.  Outputs information about the
            // index as text to the specified output stream.
            genderIdx->print(std::cout);

            ibis::qKeyword genderQuery(colGender, "Male"); // create a query object for searching column for keyword
            long numMales = mypart->keywordSearch(genderQuery, males); // perform the keyword search
            if (numMales >= 0) {
                // the query was successful
                // the hits are stored in the 'males' bitvector
                std::cout << "the query was successful" << std::endl;
                std::cout << "numMales: " << numMales << std::endl;
            } else {
                // an error occurred while evaluating the query
                std::cout << "an error occurred while evaluating the query" << std::endl;
            }
           
            // check here
            males.decompress();

            if (males.isCompressed())
                std::cout << "males is compressed" << std::endl;
            else
                std::cout << "males is decompressed" << std::endl;
            std::cout << "number of bits in males: " << males.size() << std::endl;
            std::cout << "size in bytes of males: " << males.bytes() << std::endl;
        }
        delete genderIdx;
    }
    
    std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;

    // DateX
    std::string dateStr[7*weeks];
    ibis::column *dateCol[7*weeks];
    ibis::index *dateIdx[7*weeks];
    ibis::bitvector dateBv[7*weeks];
    for (int i = 0; i < 7*weeks; i++) {
        dateStr[i] = "Date" + std::to_string(i + 1);
        dateCol[i] = mypart->getColumn(dateStr[i].c_str());
        if (dateCol[i] != 0) {
            dateIdx[i] = ibis::index::create(dateCol[i], (std::string(dir) + "/" + dateStr[i] + ".idx").c_str(), 0, 1);
            if (dateIdx[i] != 0) {
                // dateIdx[i] is a pointer to the newly created dateIdx[i] index 
                // Returns the number of bit vectors used by the index 
                std::cout << "number of bit vectors used by the dateIdx" + std::to_string(i + 1) << ": " << dateIdx[i]->numBitvectors() << std::endl;
                std::cout << "size in bytes: " << dateIdx[i]->sizeInBytes() << std::endl;
                // Prints human readable information.  Outputs information about the
                // index as text to the specified output stream.
                dateIdx[i]->print(std::cout);

                ibis::qKeyword dateQuery(dateStr[i].c_str(), "Yes"); // create a query object for searching column for keyword
                long numYeses = mypart->keywordSearch(dateQuery, dateBv[i]); // perform the keyword search
                if (numYeses >= 0) {
                    // the query was successful
                    // the hits are stored in the 'males' bitvector
                    std::cout << "the query was successful" << std::endl;
                    std::cout << "numYeses: " << numYeses << std::endl;
                } else {
                    // an error occurred while evaluating the query
                    std::cout << "an error occurred while evaluating the query" << std::endl;
                }
                
                // check here
                dateBv[i].decompress();

                if (dateBv[i].isCompressed())
                    std::cout << "dateBv" + std::to_string(i + 1) << " is compressed" << std::endl;
                else
                    std::cout << "dateBv" + std::to_string(i + 1) << " is decompressed" << std::endl;
                std::cout << "number of bits in dateBv" + std::to_string(i + 1) << ": " << dateBv[i].size() << std::endl;
                std::cout << "size in bytes of dateBv" + std::to_string(i + 1) << ": " << dateBv[i].bytes() << std::endl;
            }
            delete dateIdx[i];
        std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
        }  
    }
    std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;

    // final result for query 1 (1 bitcount op)
    ibis::bitvector::word_t count1;
    // final result for query 2 (w bitcount op)
    ibis::bitvector::word_t count2[weeks];
   
    // result arrays of OR operations between 7 days for w weeks  
    ibis::bitvector w[weeks];
    // final result bitvector for query 1 (1 bitcount op)
    ibis::bitvector result1;
    // final result bitvector for query 2 (w bitcount op)
    ibis::bitvector* result2[weeks];
 
    SimRoiStart();

    // do the first query
    // How many unique users were active every week for the past w weeks?
    // we have w weeks, so we need 6w OR operations and w bitvectors
    for (int i = 0; i < weeks; i++) {
        w[i] = dateBv[i*7];
        for (int j = 1; j < 7; j++) {
            w[i] |= dateBv[i*7 + j]; 
        }
    }
    // we have w weeks, so we need (w-1) AND operations between weeks
    result1 = w[0];
    for (int i = 1; i < weeks; i++) {
        result1 &= w[i];
    }
    // do the bitcount operation for result1
    // Return the number of bits that are one
    count1 = result1.cnt(); 

    // do the second query
    // How many male users were active each of the past w weeks?
    // we have w weeks, so we need w AND operations between males bitvector and OR results of each week
    for (int i = 0; i < weeks; i++) {
        result2[i] = males & w[i];
        count2[i] = result2[i]->cnt();
    }

    SimRoiEnd();
    
    std::cout << "Result of query 1: " << count1 << std::endl;
    for (int i = 0; i < weeks; i++) {
        std::cout << "Result of query 2: " << count2[i] << std::endl;
    }

    std::cout << "Done!" << std::endl;

    return 0;
}
