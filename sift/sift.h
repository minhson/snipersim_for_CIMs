#ifndef __SIFT_H
#define __SIFT_H

// Sniper Instruction Trace File Format
//
// x86-64 only, little-endian

#include <cstdint>
#include <cstddef>

namespace Sift
{
   const uint32_t MAX_DYNAMIC_ADDRESSES = 2;

   typedef enum
   {
      MemInvalidLock = 0,
      MemNoLock,
      MemLock,
      MemUnlock,
   } MemoryLockType;

   typedef enum
   {
      MemInvalidOp = 0,
      MemRead,
      MemWrite,
      // add CIM cmd here
      MemCLFLUSH,
      MemCLINVAL,
      MemNOT,
      MemAND,
      MemOR,
      MemNAND,
      MemNOR,
      MemXOR,
      MemCIMREAD,
      MemAND_S,
      MemOR_S,
      // end add CIM cmd here
   } MemoryOpType;

   typedef enum
   {
      RoutineEnter = 0,
      RoutineExit,
      RoutineAssert,
   } RoutineOpType;
};

#endif // __SIFT_H
