#include "dram_cntlr.h"
#include "memory_manager.h"
#include "core.h"
#include "log.h"
#include "subsecond_time.h"
#include "stats.h"
#include "fault_injection.h"
#include "shmem_perf.h"
#include "dvfs_manager.h"

#if 0
   extern Lock iolock;
#  include "core_manager.h"
#  include "simulator.h"
#  define MYLOG(...) { ScopedLock l(iolock); fflush(stdout); printf("[%s] %d%cdr %-25s@%3u: ", itostr(getShmemPerfModel()->getElapsedTime()).c_str(), getMemoryManager()->getCore()->getId(), Sim()->getCoreManager()->amiUserThread() ? '^' : '_', __FUNCTION__, __LINE__); printf(__VA_ARGS__); printf("\n"); fflush(stdout); }
#else
#  define MYLOG(...) {}
#endif

class TimeDistribution;

namespace PrL1PrL2DramDirectoryMSI
{

DramCntlr::DramCntlr(MemoryManagerBase* memory_manager,
      ShmemPerfModel* shmem_perf_model,
      UInt32 cache_block_size)
   : DramCntlrInterface(memory_manager, shmem_perf_model, cache_block_size)
   , m_reads(0)
   , m_writes(0)
   // added by MSON
   , m_not(0)
   , m_and(0)
   , m_or(0)
   , m_nand(0)
   , m_nor(0)
   , m_xor(0)
   , m_cimread(0)
   , m_and_s(0)
   , m_or_s(0)
{
   m_dram_perf_model = DramPerfModel::createDramPerfModel(
         memory_manager->getCore()->getId(),
         cache_block_size);

   m_fault_injector = Sim()->getFaultinjectionManager()
      ? Sim()->getFaultinjectionManager()->getFaultInjector(memory_manager->getCore()->getId(), MemComponent::DRAM)
      : NULL;

   m_dram_access_count = new AccessCountMap[DramCntlrInterface::NUM_ACCESS_TYPES];
   registerStatsMetric("dram", memory_manager->getCore()->getId(), "reads", &m_reads);
   registerStatsMetric("dram", memory_manager->getCore()->getId(), "writes", &m_writes);
   // added by MSON
   registerStatsMetric("dram", memory_manager->getCore()->getId(), "nots", &m_not);
   registerStatsMetric("dram", memory_manager->getCore()->getId(), "ands", &m_and);
   registerStatsMetric("dram", memory_manager->getCore()->getId(), "ors", &m_or);
   registerStatsMetric("dram", memory_manager->getCore()->getId(), "nands", &m_nand);
   registerStatsMetric("dram", memory_manager->getCore()->getId(), "nors", &m_nor);
   registerStatsMetric("dram", memory_manager->getCore()->getId(), "xors", &m_xor);
   registerStatsMetric("dram", memory_manager->getCore()->getId(), "cimreads", &m_cimread);
   registerStatsMetric("dram", memory_manager->getCore()->getId(), "and_ss", &m_and_s);
   registerStatsMetric("dram", memory_manager->getCore()->getId(), "or_ss", &m_or_s);
}

DramCntlr::~DramCntlr()
{
   printDramAccessCount();
   delete [] m_dram_access_count;

   delete m_dram_perf_model;
}

boost::tuple<SubsecondTime, HitWhere::where_t>
DramCntlr::getDataFromDram(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now, ShmemPerf *perf)
{
   if (Sim()->getFaultinjectionManager())
   {
      if (m_data_map.count(address) == 0)
      {
         m_data_map[address] = new Byte[getCacheBlockSize()];
         memset((void*) m_data_map[address], 0x00, getCacheBlockSize());
      }

      // NOTE: assumes error occurs in memory. If we want to model bus errors, insert the error into data_buf instead
      if (m_fault_injector)
         m_fault_injector->preRead(address, address, getCacheBlockSize(), (Byte*)m_data_map[address], now);

      memcpy((void*) data_buf, (void*) m_data_map[address], getCacheBlockSize());
   }

   SubsecondTime dram_access_latency = runDramPerfModel(requester, now, address, READ, perf);

   ++m_reads;
   #ifdef ENABLE_DRAM_ACCESS_COUNT
   addToDramAccessCount(address, READ);
   #endif
   LOG_PRINT("R @ %08lx latency %s", address, itostr(dram_access_latency).c_str());
   MYLOG("R @ %08lx latency %s", address, itostr(dram_access_latency).c_str());

   // for DRAMsim3/NVMain memory traces file
   // Convert SubsecondTime to cycles in global clock domain
   const ComponentPeriod *dom_global = Sim()->getDvfsManager()->getGlobalDomain();
   UInt64 cycles = SubsecondTime::divideRounded(now, *dom_global);

   //FIXMEchar* data = new char[getCacheBlockSize()];
   //FIXMEgetMemoryManager()->getCore()->accessMemory(Core::NONE, Core::READ, (IntPtr) address, data, getCacheBlockSize());
   // only write the mem_trace for DETAILED mode only
   // only work for model = none in [perf_model/fast_forward] ???
   // check here
   if (Sim()->getInstrumentationMode() == InstMode::DETAILED)
   {
       // for NVMain
       //MEM_TRACE("%llu R 0x%x %0128x", cycles, address, data_buf);
       // for DRAMsim3
       MEM_TRACE("0x%X READ %llu", address, cycles);
   }

   return boost::tuple<SubsecondTime, HitWhere::where_t>(dram_access_latency, HitWhere::DRAM);
}

// added by MSON
boost::tuple<SubsecondTime, HitWhere::where_t>
DramCntlr::sendCimToMM(Core::mem_op_t mem_op_type, IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now, ShmemPerf *perf)
{
    const char * type;
    DramCntlrInterface::access_t cim_type = READ;
    switch (mem_op_type)
    {
        case Core::NOT:
            cim_type = NOT;
            ++m_not;
            type = "NOT";
            break;
        case Core::AND:
            cim_type = AND;
            ++m_and;
            type = "AND";
            break;
        case Core::OR:
            cim_type = OR;
            ++m_or;
            type = "OR";
            break;
        case Core::NAND:
            cim_type = NAND;
            ++m_nand;
            type = "NAND";
            break;
        case Core::NOR:
            cim_type = NOR;
            ++m_nor;
            type = "NOR";
            break;
        case Core::XOR:
            cim_type = XOR;
            ++m_xor;
            type = "XOR";
            break;
        case Core::CIMREAD:
            cim_type = CIMREAD;
            ++m_cimread;
            type = "CIM_READ";
            break;
        case Core::AND_S:
            cim_type = AND_S;
            ++m_and_s;
            type = "AND_S";
            break;
        case Core::OR_S:
            cim_type = OR_S;
            ++m_or_s;
            type = "OR_S";
            break;
        default:
            LOG_PRINT_ERROR("Unsupported CIM Op Type(%u)", cim_type);

    }
    SubsecondTime cim_access_latency = runDramPerfModel(requester, now, address, cim_type, perf);
    
    ++m_reads;
    #ifdef ENABLE_DRAM_ACCESS_COUNT
    addToDramAccessCount(address, cim_type);
    #endif
    
    LOG_PRINT("%s @ %08lx latency %s", type, address, itostr(cim_access_latency).c_str());

    // for DRAMsim3/NVMain memory traces file
    // Convert SubsecondTime to cycles in global clock domain
    const ComponentPeriod *dom_global = Sim()->getDvfsManager()->getGlobalDomain();
    UInt64 cycles = SubsecondTime::divideRounded(now, *dom_global);
  
    //FIXMEchar* data = new char[getCacheBlockSize()];
    //FIXMEgetMemoryManager()->getCore()->accessMemory(Core::NONE, Core::READ, (IntPtr) address, data, getCacheBlockSize());
    // only write the mem_trace for DETAILED mode only
    // only work for model = none in [perf_model/fast_forward] ???
    // check here
    if (Sim()->getInstrumentationMode() == InstMode::DETAILED)
    {
        // for NVMain
        //MEM_TRACE("%llu %s 0x%x %0128x", cycles, type, address, data_buf);
        // for DRAMsim3
        //MEM_TRACE("0x%X %s %llu", address, type, cycles);
    }

    return boost::tuple<SubsecondTime, HitWhere::where_t>(cim_access_latency, HitWhere::DRAM);
}

boost::tuple<SubsecondTime, HitWhere::where_t>
DramCntlr::putDataToDram(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now)
{
   if (Sim()->getFaultinjectionManager())
   {
      if (m_data_map[address] == NULL)
      {
         LOG_PRINT_ERROR("Data Buffer does not exist");
      }
      memcpy((void*) m_data_map[address], (void*) data_buf, getCacheBlockSize());
      
      // NOTE: assumes error occurs in memory. If we want to model bus errors, insert the error into data_buf instead
      if (m_fault_injector)
         m_fault_injector->postWrite(address, address, getCacheBlockSize(), (Byte*)m_data_map[address], now);
   }

   SubsecondTime dram_access_latency = runDramPerfModel(requester, now, address, WRITE, &m_dummy_shmem_perf);

   ++m_writes;
   #ifdef ENABLE_DRAM_ACCESS_COUNT
   addToDramAccessCount(address, WRITE);
   #endif
   MYLOG("W @ %08lx", address);

   // for DRAMsim3/NVMain memory traces file
   // Convert SubsecondTime to cycles in global clock domain
   const ComponentPeriod *dom_global = Sim()->getDvfsManager()->getGlobalDomain();
   UInt64 cycles = SubsecondTime::divideRounded(now, *dom_global);

   //FIXMEchar *data = new char[getCacheBlockSize()];
   //FIXMEgetMemoryManager()->getCore()->accessMemory(Core::NONE, Core::READ, (IntPtr) address, data, getCacheBlockSize());
   // only write the mem_trace for DETAILED mode only
   // only work for model = none in [perf_model/fast_forward] ???
   // check here
   if (Sim()->getInstrumentationMode() == InstMode::DETAILED)
   {
       // for NVMain
       //MEM_TRACE("%llu W 0x%x %0128x", cycles, address, data_buf);
       // for DRAMsim3
       MEM_TRACE("0x%X WRITE %llu", address, cycles);
   }

   return boost::tuple<SubsecondTime, HitWhere::where_t>(dram_access_latency, HitWhere::DRAM);
}

SubsecondTime
DramCntlr::runDramPerfModel(core_id_t requester, SubsecondTime time, IntPtr address, DramCntlrInterface::access_t access_type, ShmemPerf *perf)
{
   UInt64 pkt_size = getCacheBlockSize();
   SubsecondTime dram_access_latency = m_dram_perf_model->getAccessLatency(time, pkt_size, requester, address, access_type, perf);
   return dram_access_latency;
}

void
DramCntlr::addToDramAccessCount(IntPtr address, DramCntlrInterface::access_t access_type)
{
   m_dram_access_count[access_type][address] = m_dram_access_count[access_type][address] + 1;
}

void
DramCntlr::printDramAccessCount()
{
   for (UInt32 k = 0; k < DramCntlrInterface::NUM_ACCESS_TYPES; k++)
   {
      for (AccessCountMap::iterator i = m_dram_access_count[k].begin(); i != m_dram_access_count[k].end(); i++)
      {
         if ((*i).second > 100)
         {
            LOG_PRINT("Dram Cntlr(%i), Address(0x%x), Access Count(%llu), Access Type(%s)",
                  m_memory_manager->getCore()->getId(), (*i).first, (*i).second,
                  (k == READ)? "READ" : "WRITE");
         }
      }
   }
}

}
