#include "PacketsStore.h"
#include "Utils.h"
#include "Log.h"

LOG_REGISTER_MODULE("PacketsStore");

std::ostream &
operator<<(std::ostream &os, [[maybe_unused]] PacketsStore const &o)
{
  os << "=========== PACKET STORE ===============\n";
  for (auto &item : o.m_packets)
  {
    os << *item.second.get();
  }
  os << "========================================\n";
  return os;
}

static Id_t s_pktCtr = 1;

PacketsStore::PacketsStore()
    : m_recordGlobalMetrics(false), m_recordPacketTrace(false), m_recordNodesTimes(false)
{
  BEGEND;
}

Packet *
PacketsStore::CreatePacket(Id_t flowId)
{
  BEG;
  auto pktId = s_pktCtr;
  ++s_pktCtr;

  auto p = m_packets.insert({pktId, std::make_unique<Packet>(pktId, flowId)});
  MSG_ASSERT(p.second, "Error creating packet");
  END;
  return p.first->second.get();
}

Packet *
PacketsStore::GetPacket(Id_t id)
{
  BEG;
  auto iter = m_packets.find(id);
  MSG_ASSERT(iter != m_packets.end(), "Requested packet " + std::to_string(id) + " does not exist");
  END;
  return iter->second.get();
}

void PacketsStore::DeletePacket(Packet *pkt)
{
  BEG;
  //    DBG("Deleting packet ", pkt->GetPacketId());
  if (m_recordGlobalMetrics)
  {
    //        DBG("Recording global metrics");
    RecordGlobalMetrics(pkt);
  }
  if (m_recordPacketTrace)
  {
    RecordPacketsTrace(pkt);
    //        DBG("Recording packet metrics");
  }
  if (m_recordNodesTimes)
  {
    RecordNodesTime(pkt);
    //        DBG("Recording node times metrics");
  }
  m_packets.erase(pkt->GetPacketId());
  END;
}

void PacketsStore::SetConfiguration(const json &j)
{
  BEG;
  auto resPath = JsonCheckAndGet<>(j, "/Results/ResultsPath");
  if (JsonContains(j, "/Results/PacketsMetrics"))
  {
    m_globFile = resPath + "/" + JsonCheckAndGet<>(j, "/Results/PacketsMetrics");
    m_recordGlobalMetrics = true;
  }
  if (JsonContains(j, "/Results/PacketsTrace"))
  {
    m_pktTracesFile = resPath + "/" + JsonCheckAndGet<>(j, "/Results/PacketsTrace");
    m_recordPacketTrace = true;
  }
  if (JsonContains(j, "/Results/PacketsNodesTimes"))
  {

    m_nodeTimesRange = JsonCheckAndGet<std::vector<Id_t>>(j, "/Results/PacketsNodesTimes/NodesRange");
    m_nodeTimesFile = resPath + "/" + JsonCheckAndGet<>(j, "/Results/PacketsNodesTimes/FileName");
    m_recordNodesTimes = true;
  }
  END;
}

void PacketsStore::GenerateResults(void) const
{
  BEG;

  if (m_recordGlobalMetrics)
  {
    DBG("Generating global metrics")
    GenerateGlobalMetrics();
  }
  if (m_recordPacketTrace)
  {
    DBG("Generating packet traces")
    GeneratePacketsTrace();
  }
  if (m_recordNodesTimes)
  {
    DBG("Generating node time results")
    GenerateNodeTime();
  }
  END;
}

void PacketsStore::RecordGlobalMetrics(Packet *pkt)
{
  BEG;
  auto flowId = pkt->GetFlowId();
  // if (flowId > 3)
  // {
  //   return;
  // }
  if (m_globMetrics.find(flowId) == m_globMetrics.end())
  {
    m_globMetrics.insert({flowId, {0.0, 0.0, 0.0, 0.0, 0u, 0u, 0u}});
    m_interDepartureTime[flowId] = {};
  }

  if (pkt->IsDropped())
  {
    ++std::get<MetricsGlob::G_Dropped>(m_globMetrics[flowId]);
  }
  else
  {
    std::get<MetricsGlob::G_Delay>(m_globMetrics[flowId]) += pkt->GetDelay();
    std::get<MetricsGlob::G_WaitingTime>(m_globMetrics[flowId]) += pkt->GetWaitingTime();
    std::get<MetricsGlob::G_ProcessingTime>(m_globMetrics[flowId]) += pkt->GetProcessingTime();
    std::get<MetricsGlob::G_PropagationTime>(m_globMetrics[flowId]) += pkt->GetAirTime();
    ++std::get<MetricsGlob::G_CounterSuccess>(m_globMetrics[flowId]);
  }
  ++std::get<MetricsGlob::G_Counter>(m_globMetrics[flowId]);

  m_interDepartureTime[flowId].push_back(NOW - m_lastDepartureTime[flowId]);
  m_lastDepartureTime[flowId] = NOW;

  END;
}

void PacketsStore::RecordPacketsTrace(Packet *pkt)
{
  BEG;
  auto flowId = pkt->GetFlowId();
  if (m_pktTraces.find(flowId) == m_pktTraces.end())
  {
    m_pktTraces.insert({flowId,
                        {}});
  }
  m_pktTraces[flowId].push_back({
      pkt->GetDelay(),
      pkt->GetWaitingTime(),
      pkt->GetProcessingTime(),
      pkt->GetAirTime(),
      pkt->IsDropped(),
  });
  END;
}

void PacketsStore::RecordNodesTime(Packet *pkt)
{
  BEG;
  auto flowId = pkt->GetFlowId();
  if (m_nodeTimes.find(flowId) == m_nodeTimes.end())
  {
    m_nodeTimes.insert({flowId,
                        {}});
  }
  for (auto i = m_nodeTimesRange[0]; i <= m_nodeTimesRange[1]; ++i)
  {
    std::get<MetricsNodeTime::M_Time>(m_nodeTimes[flowId][i]) += pkt->GetTimeInNode(i);
    ++std::get<MetricsNodeTime::M_Counter>(m_nodeTimes[flowId][i]);
  }
  END;
}

void PacketsStore::GenerateGlobalMetrics() const
{
  BEG;
  auto allDel = 0.0;
  auto allWiat = 0.0;
  auto allProc = 0.0;
  auto allProp = 0.0;
  auto allDrop = 0.0;
  auto allCtr = 0u;
  auto allCtrSuccess = 0u;

  for (auto &item : m_globMetrics)
  {
    auto fn = m_globFile + "_flow_" + std::to_string(item.first) + ".dat";
    std::ofstream ofs;
    ofs.open(fn, std::ios::app);
    MSG_ASSERT(ofs.is_open(), "Could not open file " + fn);

    auto auxDel = std::get<MetricsGlob::G_Delay>(item.second);
    auto auxWiat = std::get<MetricsGlob::G_WaitingTime>(item.second);
    auto auxProc = std::get<MetricsGlob::G_ProcessingTime>(item.second);
    auto auxProp = std::get<MetricsGlob::G_PropagationTime>(item.second);
    auto auxDrop = std::get<MetricsGlob::G_Dropped>(item.second);
    auto auxCtr = std::get<MetricsGlob::G_Counter>(item.second);
    auto auxCtrSuccess = std::get<MetricsGlob::G_CounterSuccess>(item.second);
    auto rsd = std::sqrt(Variance(m_interDepartureTime.at(item.first))) / Mean(m_interDepartureTime.at(item.first));
    ofs << auxDel / auxCtrSuccess
        << "\t" << auxWiat / auxCtrSuccess
        << "\t" << auxProc / auxCtrSuccess
        << "\t" << auxProp / auxCtrSuccess
        << "\t" << double(auxDrop) / auxCtr
        //                << "\t" << rsd
        << "\n";
    ofs.close();

    allDel += auxDel;
    allWiat += auxWiat;
    allProc += auxProc;
    allProp += auxProp;
    allDrop += auxDrop;
    allCtr += auxCtr;
    allCtrSuccess += auxCtrSuccess;
    DBG("Pkt Counter of flow ", item.first, ": ", auxCtr);
  }
  // std::vector<double> auxV;

  // for (auto &item : m_interDepartureTime)
  // {
  //   auxV.insert(auxV.end(), item.second.begin(), item.second.end());
  // }
  // auto rsd = std::sqrt(Variance(auxV)) / Mean(auxV);
  // auto fn = m_globFile + "_all.dat";
  // std::ofstream ofs;
  // ofs.open(fn, std::ios::app);
  // MSG_ASSERT(ofs.is_open(), "Could not open file " + fn);
  // ofs << allDel / allCtrSuccess
  //     << "\t" << allWiat / allCtrSuccess
  //     << "\t" << allProc / allCtrSuccess
  //     << "\t" << allProp / allCtrSuccess
  //     << "\t" << double(allDrop) / allCtr
  //     << "\t" << rsd
  //     << "\n";
  // ofs.close();

  END;
}

void PacketsStore::GeneratePacketsTrace() const
{
  BEG;
  for (auto &item : m_pktTraces)
  {
    auto fn = m_pktTracesFile + "_flow_" + std::to_string(item.first) + ".dat";
    std::ofstream ofs;
    ofs.open(fn, std::ios::app);
    MSG_ASSERT(ofs.is_open(), "Could not open file " + fn);

    for (auto &item2 : item.second)
    {
      ofs << std::get<MetricsPacket::P_Delay>(item2)
          << "\t" << std::get<MetricsPacket::P_WaitingTime>(item2)
          << "\t" << std::get<MetricsPacket::P_ProcessingTime>(item2)
          << "\t" << std::get<MetricsPacket::P_PropagationTime>(item2)
          << "\t" << (std::get<MetricsPacket::P_Dropped>(item2) ? 1 : 0)
          << "\n";
    }
    ofs.close();
  }
  END;
}

void PacketsStore::GenerateNodeTime() const
{
  BEG;
  for (auto &item : m_nodeTimes)
  {
    auto fn = m_nodeTimesFile + "_flow_" + std::to_string(item.first) + ".dat";
    std::ofstream ofs;
    ofs.open(fn, std::ios::app);
    MSG_ASSERT(ofs.is_open(), "Could not open file " + fn);

    for (auto &item2 : item.second)
    {
      ofs << std::get<MetricsNodeTime::M_Time>(item2.second) / std::get<MetricsNodeTime::M_Counter>(item2.second) << "\t";
    }
    ofs << "\n";
    ofs.close();
  }
  END;
}