#include "Simulator.h"
#include "Log.h"

#include "Utils.h"

LOG_REGISTER_MODULE("Simulator")

std::ostream &
operator<<(std::ostream &os, const Simulator &o)
{
  return o.Print(os);
}

std::ostream &
Simulator::Print(std::ostream &os) const
{
  BEG;
  os << "========== SIMULATOR =============\n";
  for (auto &item : m_flows)
  {
    os << "Fow " << item.first << " ==> "
       << "max. pkts " << item.second.m_maxPkts << ";  "
       << "dist " << DIST_TYPE_2_STR(item.second.m_dt) << "; "
       << "flow. param " << item.second.m_param << "; "
       << "\n";
  }
  os << "==================================\n";
  END;
  return os;
}

Simulator::Simulator(const json &j)
    : BaseNode(ZN_ID), m_finishedFlows(0), m_maxSimulTime(-1)
{
  BEG;
  JSON_CHECK(j, "NumFlows");
  TOPOLOGY.SetZeroNode(this);
  auto numFlows = std::uint32_t(j["NumFlows"]);
  for (auto i = 1u; i <= numFlows; ++i)
  {
    auto auxStr = "Flow" + std::to_string(i);
    json auxj;
    if (j.contains(auxStr))
    {
      auxj = j[auxStr];
    }
    else
    {
      JSON_CHECK(j, "DefaultFlow");
      auxj = j["DefaultFlow"];
    }
    FlowConf fc;
    JSON_CHECK(auxj, "Dist");
    JSON_CHECK(auxj, "Param");
    JSON_CHECK(auxj, "MaxPackets");

    fc.m_dt = STR_2_DIST_TYPE(auxj["Dist"]);
    fc.m_param = double(auxj["Param"]);
    fc.m_maxPkts = std::uint32_t(auxj["MaxPackets"]);
    fc.m_txPkt = 0;
    fc.m_rxPkt = 0;
    fc.m_rg = RandGen(fc.m_dt, fc.m_param);
    m_flows.insert({i, fc});
  }
  m_simulType = STR_2_SIMUL_TYPE(j["Type"]);
  if (m_simulType == SimulType::TimeLimited)
  {
    m_maxSimulTime = j["MaxSimulTime"];
  }

  END;
}

Simulator::~Simulator()
{
  BEGEND;
}

void Simulator::OnEvent(void)
{
  BEG;
  // INFO(NOW, " | ", m_id, " ", s_evTypeStr.at(CURRENT_EVENT->GetType()));
  switch (CURRENT_EVENT->GetType())
  {
  case EvType_t::Arrival:
    OnArrival();
    break;
  case EvType_t::PacketFinish:
    OnPacketFinish();
    break;
  default:
    MSG_ASSERT(false, "Unknown event " + EV_TYPE_2_STR(CURRENT_EVENT->GetType()));
    break;
  }
  END;
}

void Simulator::Run(void)
{
  BEG;
  for (auto &item : m_flows)
  {
    GenerateArrival(item.first, true);
  }
  while (!ForceStop() and CURRENT_EVENT != nullptr)
  {
    ProcessEvent();
    NEXT_EVENT;
  }
  EV_LIST.Flush();
  PKT_STORE.GenerateResults();
  END;
}

void Simulator::OnArrival()
{
  BEG;
  auto pkt = CURRENT_EVENT->GetPkt();

  pkt->SetStart();
  pkt->ForwardTo(Packet::Fwd::ToAir, m_id);

  auto flowId = pkt->GetFlowId();
  GenerateArrival(flowId);

  auto nodeId = FIND_NEXT_NODE(flowId, m_id);
  auto delay = FIND_DELAY(m_id, nodeId);
  GENERATE_EVENT(EvType_t::Arrival, (NOW + delay), nodeId, pkt);
  END;
}

void Simulator::OnPacketFinish()
{
  BEG;
  auto pkt = CURRENT_EVENT->GetPkt();

  pkt->ForwardTo(Packet::Fwd::ToNode, m_id);
  pkt->SetEnd();
  auto flowId = pkt->GetFlowId();

  PKT_STORE.DeletePacket(pkt);

  auto it = m_flows.find(flowId);
  MSG_ASSERT(it != m_flows.end(), "Bad flow ID " + std::to_string(flowId));

  ++it->second.m_rxPkt;

  if (it->second.m_rxPkt % int(1e5) == 0)
  {
    std::cout << "Flow  " << flowId << ": " << it->second.m_rxPkt << "/" << it->second.m_maxPkts << std::endl;
  }
  if (it->second.m_rxPkt == it->second.m_maxPkts)
  {
    std::cout << "Finished flow " << flowId << " with " << it->second.m_rxPkt << " packets received!! " << std::endl;
    ++m_finishedFlows;
  }
  if (m_finishedFlows == m_flows.size())
  {
    EV_LIST.Flush();
  }
  END;
}

void Simulator::ProcessEvent()
{
  BEG;
  auto nodeId = CURRENT_EVENT->GetNodeId();
  if (nodeId == m_id)
  {
    OnEvent();
  }
  else
  {
    GET_NODE(nodeId)->OnEvent();
  }
  END;
}

void Simulator::GenerateArrival(Id_t flowId, bool absTime)
{
  BEG;
  auto it = m_flows.find(flowId);
  MSG_ASSERT(it != m_flows.end(), "Bad flow id " + std::to_string(flowId));

  if (it->second.m_param <= 0)
  {
    ++m_finishedFlows;
    return;
  }
  if (m_simulType == SimulType::TimeLimited or it->second.m_txPkt < it->second.m_maxPkts)
  {
    ++it->second.m_txPkt;
    auto pkt = CREATE_PACKET(flowId);
    auto arrTime = GetRandSample(it->second.m_dt, it->second.m_param);
    if (absTime)
    {
      GENERATE_EVENT(EvType_t::Arrival, arrTime, m_id, pkt);
    }
    else
    {
      GENERATE_EVENT(EvType_t::Arrival, NOW + arrTime, m_id, pkt);
    }
  }

  END;
}

bool Simulator::ForceStop(void) const
{
  BEG;
  auto ret = false;
  if (m_simulType == SimulType::TimeLimited and NOW >= m_maxSimulTime)
  {
    ret = true;
  }
  else if (m_finishedFlows == m_flows.size())
  {
    ret = true;
  }
  END;
  return ret;
}