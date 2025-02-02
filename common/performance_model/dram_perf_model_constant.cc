#include "dram_perf_model_constant.h"
#include "simulator.h"
#include "config.h"
#include "config.hpp"
#include "stats.h"
#include "shmem_perf.h"

DramPerfModelConstant::DramPerfModelConstant(core_id_t core_id,
      UInt32 cache_block_size):
   DramPerfModel(core_id, cache_block_size),
   m_queue_model(NULL),
   m_dram_bandwidth(8 * Sim()->getCfg()->getFloat("perf_model/dram/per_controller_bandwidth")), // Convert bytes to bits
   m_total_queueing_delay(SubsecondTime::Zero()),
   m_total_access_latency(SubsecondTime::Zero())
{
   m_dram_access_cost = SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(Sim()->getCfg()->getFloat("perf_model/dram/latency"))); // Operate in fs for higher precision before converting to uint64_t/SubsecondTime

   // added by MSON
   m_dram_not_cost = SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(Sim()->getCfg()->getFloat("perf_model/dram/not")));
   m_dram_and_cost = SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(Sim()->getCfg()->getFloat("perf_model/dram/and")));
   m_dram_or_cost = SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(Sim()->getCfg()->getFloat("perf_model/dram/or")));
   m_dram_nand_cost = SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(Sim()->getCfg()->getFloat("perf_model/dram/nand")));
   m_dram_nor_cost = SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(Sim()->getCfg()->getFloat("perf_model/dram/nor")));
   m_dram_xor_cost = SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(Sim()->getCfg()->getFloat("perf_model/dram/xor")));
   m_dram_cimread_cost = SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(Sim()->getCfg()->getFloat("perf_model/dram/cimread")));
   
   m_dram_and_s_cost = SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(Sim()->getCfg()->getFloat("perf_model/dram/and_s")));
   m_dram_or_s_cost = SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(Sim()->getCfg()->getFloat("perf_model/dram/or_s")));
   
   if (Sim()->getCfg()->getBool("perf_model/dram/queue_model/enabled"))
   {
      m_queue_model = QueueModel::create("dram-queue", core_id, Sim()->getCfg()->getString("perf_model/dram/queue_model/type"),
                                         m_dram_bandwidth.getRoundedLatency(8 * cache_block_size)); // bytes to bits
   }

   registerStatsMetric("dram", core_id, "total-access-latency", &m_total_access_latency);
   registerStatsMetric("dram", core_id, "total-queueing-delay", &m_total_queueing_delay);
}

DramPerfModelConstant::~DramPerfModelConstant()
{
   if (m_queue_model)
   {
     delete m_queue_model;
      m_queue_model = NULL;
   }
}

SubsecondTime
DramPerfModelConstant::getAccessLatency(SubsecondTime pkt_time, UInt64 pkt_size, core_id_t requester, IntPtr address, DramCntlrInterface::access_t access_type, ShmemPerf *perf)
{
   // pkt_size is in 'Bytes'
   // m_dram_bandwidth is in 'Bits per clock cycle'
   if ((!m_enabled) ||
         (requester >= (core_id_t) Config::getSingleton()->getApplicationCores()))
   {
      return SubsecondTime::Zero();
   }
   // added by MSON
   SubsecondTime processing_time;
   // no data transfer for bitwise operation except for CIMREAD cmd
   if (access_type == DramCntlrInterface::NOT || access_type == DramCntlrInterface::AND || access_type == DramCntlrInterface::OR
           || access_type == DramCntlrInterface::NAND || access_type == DramCntlrInterface::NOR || access_type == DramCntlrInterface::XOR
           || access_type == DramCntlrInterface::AND_S || access_type == DramCntlrInterface::OR_S)
   {
       processing_time = SubsecondTime::Zero();
   }
   else
   {
       processing_time = m_dram_bandwidth.getRoundedLatency(8 * pkt_size); // bytes to bits
       //FIXMEstd::cout << "processing_time: " << processing_time << std::endl;
   }
   // added by MSON
   
   //FIXMESubsecondTime processing_time = m_dram_bandwidth.getRoundedLatency(8 * pkt_size); // bytes to bits

   // Compute Queue Delay
   SubsecondTime queue_delay;
   if (m_queue_model)
   {
      queue_delay = m_queue_model->computeQueueDelay(pkt_time, processing_time, requester);
      // added by MSON
      //FIXMEstd::cout << "queue_delay: " << queue_delay << std::endl;
      // added by MSON
   }
   else
   {
      queue_delay = SubsecondTime::Zero();
   }

   // added by MSON
   switch (access_type)
   {
       case DramCntlrInterface::NOT:
           m_dram_access_cost = m_dram_not_cost;
           break;
       case DramCntlrInterface::AND:
           m_dram_access_cost = m_dram_and_cost;
           break;
       case DramCntlrInterface::OR:
           m_dram_access_cost = m_dram_or_cost;
           break;
       case DramCntlrInterface::NAND:
           m_dram_access_cost = m_dram_nand_cost;
           break;
       case DramCntlrInterface::NOR:
           m_dram_access_cost = m_dram_nor_cost;
           break;
       case DramCntlrInterface::XOR:
           m_dram_access_cost = m_dram_xor_cost;
           break;
       case DramCntlrInterface::CIMREAD:
           m_dram_access_cost = m_dram_cimread_cost;
           break;
       case DramCntlrInterface::AND_S:
           m_dram_access_cost = m_dram_and_s_cost;
           break;
       case DramCntlrInterface::OR_S:
           m_dram_access_cost = m_dram_or_s_cost;
           break;
       default:
           LOG_PRINT("Unsupported access_type!\n");
   }

   SubsecondTime access_latency = queue_delay + processing_time + m_dram_access_cost;


   perf->updateTime(pkt_time);
   perf->updateTime(pkt_time + queue_delay, ShmemPerf::DRAM_QUEUE);
   perf->updateTime(pkt_time + queue_delay + processing_time, ShmemPerf::DRAM_BUS);
   perf->updateTime(pkt_time + queue_delay + processing_time + m_dram_access_cost, ShmemPerf::DRAM_DEVICE);

   // Update Memory Counters
   m_num_accesses ++;
   m_total_access_latency += access_latency;
   m_total_queueing_delay += queue_delay;

   return access_latency;
}
