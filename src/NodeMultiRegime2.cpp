#include <iomanip>

#include "NodeMultiRegime2.h"
#include "Log.h"
#include "RandGenerator.h"
#include "Utils.h"

LOG_REGISTER_MODULE("NodeMultiRegime2")

std::ostream &
operator<<(std::ostream &os, const NodeMultiRegime2 &o)
{
  return o.Print(os);
}

std::ostream &
NodeMultiRegime2::Print(std::ostream &os) const
{
  BEG;
  os << "NODE_" << m_id << " (type NodeMultiRegime2) : "
     << "queue size " << m_maxQueueSize << "; "
     << "service dist " << DIST_TYPE_2_STR(m_mlConfig.m_servDist) << "; "
     << "regime dist " << DIST_TYPE_2_STR(m_mlConfig.m_regimeDist)
     << "\n";
  os << "service rates (mu) " << m_mlConfig.m_servParam << "\n";
  os << "change rates (gammas) " << m_mlConfig.m_regimeParam << "\n";
  os << "regime probs (alphas) \n"
     << m_mlConfig.m_regimeProb << "\n";

  END;
  return os;
}

NodeMultiRegime2::NodeMultiRegime2(Id_t nodeid, int maxQsize, NodeMultiRegime2::Config mlconf)
    : BaseNode(nodeid), m_mlConfig(mlconf), m_maxQueueSize(maxQsize), m_controllerActive(true), m_currentPkt(nullptr), m_stateChange(0), m_queueStateChange(0), m_regimeChange(0)
{
  BEG;
  for (auto i = 0u; i < m_mlConfig.m_regimeParam.size(); ++i)
  {
    m_regimesStaysTime[i] = {};
    for (auto j = 0u; j < m_mlConfig.m_regimeParam.size(); ++j)
    {
      m_regimesTransits[i][j] = 0;
      m_regimesTransits[j][i] = 0;
    }
  }

  m_queue.clear();
  m_currentRegime = std::floor(m_mlConfig.m_regimeProb.size() * UnitRand());
  m_currentRegime = m_mlConfig.m_regimeProb.size() ? m_currentRegime - 1 : m_currentRegime;
  // m_currentRegime = 1;
  GenerateRegimeChange();
  // DBG("Max queue size ", m_maxQueueSize);
  END;
}

NodeMultiRegime2::~NodeMultiRegime2()
{
  BEGEND;
}

void NodeMultiRegime2::OnEvent(void)
{
  BEG;

  // DBG(NOW, " | ", m_id, " ", EV_TYPE_2_STR(CURRENT_EVENT->GetType()), " packet ", CURRENT_EVENT->GetPkt() != nullptr ? CURRENT_EVENT->GetPkt()->GetPacketId() : 0);
  UpdateStateTrace();
  switch (CURRENT_EVENT->GetType())
  {
  case EvType_t::Arrival:
    OnArrival();
    break;
  case EvType_t::Departure:
    OnDeparture();
    break;
  case EvType_t::RegimeChange:
    OnRegimeChange();
    break;
  default:
    MSG_ASSERT(false, "Unknown event " + EV_TYPE_2_STR(CURRENT_EVENT->GetType()));
    break;
  }
  END;
}

void NodeMultiRegime2::OnArrival()
{
  BEG;
  auto pkt = CURRENT_EVENT->GetPkt();
  pkt->ForwardTo(Packet::Fwd::ToNode, m_id);
  if (m_currentPkt == nullptr)
  {
    MSG_ASSERT(m_queue.size() == 0, "Node bad behavior");
    m_currentPkt = pkt;
    if (m_controllerActive)
    {
      GenerateDeparture();
      pkt->ForwardTo(Packet::Fwd::ToProcessor, m_id);
    }
  }
  else if (m_maxQueueSize < 0 or int(m_queue.size()) < m_maxQueueSize)
  {
    UpdateQueueStateTrace();
    m_queue.push_back(pkt);
    pkt->ForwardTo(Packet::Fwd::ToQueue, m_id);
  }
  else
  {
    pkt->Drop();
    GENERATE_EVENT(EvType_t::PacketFinish, NOW, ZN_ID, pkt);
  }
  END;
}

void NodeMultiRegime2::OnDeparture()
{
  BEG;
  SendToNext();
  m_currentPkt = nullptr;
  SaveProcessFromQueue();
  END;
}

void NodeMultiRegime2::OnRegimeChange()
{
  BEG;
  auto newRegime = SelectRegime();
  UpdateRegimeTrace(newRegime);
  m_currentRegime = newRegime;
  INFO(NOW, " | Node ", m_id, " to regime ", m_currentRegime);
  REMOVE_DEPARTURES(m_id);
  if (m_mlConfig.m_servParam.at(m_currentRegime) <= 0)
  {
    m_controllerActive = false;
  }
  else
  {
    m_controllerActive = true;
    if (m_currentPkt != nullptr)
    {
      GenerateDeparture();
      m_currentPkt->ForwardTo(Packet::Fwd::ToProcessor, m_id);
    }
  }
  GenerateRegimeChange();
  END;
}

void NodeMultiRegime2::SendToNext()
{
  BEG;
  auto pkt = CURRENT_EVENT->GetPkt();
  auto nodeId = FIND_NEXT_NODE(pkt->GetFlowId(), m_id);
  pkt->ForwardTo(Packet::Fwd::ToAir, nodeId);
  auto delay = FIND_DELAY(m_id, nodeId);
  if (nodeId != ZN_ID)
  {
    GENERATE_EVENT(EvType_t::Arrival, NOW + delay, nodeId, pkt);
  }
  else
  {
    GENERATE_EVENT(EvType_t::PacketFinish, NOW + delay, nodeId, pkt);
  }
  END;
}

void NodeMultiRegime2::SaveProcessFromQueue()
{
  BEG;
  if (!m_queue.empty())
  {
    m_currentPkt = m_queue.front();
    UpdateQueueStateTrace();
    m_queue.pop_front();
    if (m_controllerActive)
    {
      auto time = GetRandSample(m_mlConfig.m_servDist, m_mlConfig.m_servParam.at(m_currentRegime));
      GENERATE_EVENT(EvType_t::Departure, NOW + time, m_id, m_currentPkt);
      m_currentPkt->ForwardTo(Packet::Fwd::ToProcessor, m_id);
    }
  }
  END;
}

std::uint32_t
NodeMultiRegime2::SelectRegime()
{
  BEG;
  MSG_ASSERT(m_mlConfig.m_regimeProb.size() > m_currentRegime, "Current regime out of bounds");
  auto currRegimeProbs = m_mlConfig.m_regimeProb.at(m_currentRegime);
  auto r = UnitRand();
  auto newLevel = 0u;
  auto aux1 = 0.0;
  auto aux2 = 0.0;
  auto ctr = 0u;
  for (auto v : currRegimeProbs)
  {
    aux2 += v;
    if (r < aux2 && r >= aux1)
    {
      newLevel = ctr;
      break;
    }
    ++ctr;
  }
  END;
  return newLevel;
}

void NodeMultiRegime2::GenerateRegimeChange()
{
  BEG;
  if (m_mlConfig.m_regimeParam.at(m_currentRegime) > 0)
  {
    auto time = GetRandSample(m_mlConfig.m_regimeDist, m_mlConfig.m_regimeParam.at(m_currentRegime));
    GENERATE_EVENT(EvType_t::RegimeChange, NOW + time, m_id, nullptr);
  }
  END;
}

void NodeMultiRegime2::GenerateDeparture()
{
  BEG;
  if (m_mlConfig.m_servParam.at(m_currentRegime) > 0)
  {
    auto time = GetRandSample(m_mlConfig.m_servDist, m_mlConfig.m_servParam.at(m_currentRegime));
    GENERATE_EVENT(EvType_t::Departure, NOW + time, m_id, m_currentPkt);
  }
  END;
}

void NodeMultiRegime2::UpdateStateTrace(void)
{
  BEG;
  auto delta = NOW - m_stateChange;
  m_stateChange = NOW;
  auto busy = (m_currentPkt != nullptr ? 1 : 0);
  auto state = m_queue.size() + busy;
  if (m_statesTime.find(state) == m_statesTime.end())
  {
    for (auto i = 0u; i < m_mlConfig.m_regimeParam.size(); ++i)
    {
      m_statesTime[state].insert({i, 0.0});
    }
  }
  m_statesTime[state][m_currentRegime] += delta;
  END;
}

void NodeMultiRegime2::UpdateQueueStateTrace(void)
{
  BEG;
  auto delta = NOW - m_queueStateChange;
  m_queueStateChange = NOW;
  auto state = m_queue.size();

  if (m_queueStatesTime.find(state) == m_queueStatesTime.end())
  {
    for (auto i = 0u; i < m_mlConfig.m_regimeParam.size(); ++i)
    {
      m_queueStatesTime[state].insert({i, 0.0});
    }
  }

  m_queueStatesTime[state][m_currentRegime] += delta;
  m_queueLog.insert({NOW, {state, m_currentRegime}});
  END;
}

void NodeMultiRegime2::UpdateRegimeTrace(std::uint32_t newRegime)
{
  BEG;
  auto delta = NOW - m_regimeChange;
  m_regimeChange = NOW;

  m_regimesStaysTime[m_currentRegime].push_back(delta);
  ++m_regimesTransits[m_currentRegime][newRegime];

  END;
}

void NodeMultiRegime2::GenerateResults(const std::string &fn) const
{
  BEG;
  StatesResults(fn);
  // ProbsResults(fn);
  // QueueLogResults(fn);
  END;
}

void NodeMultiRegime2::StatesResults(const std::string &fn) const
{
  BEG;
  std::ofstream ofs;
  std::stringstream ss;
  ss << fn << "_states_node_" << std::setw(3) << std::setfill('0') << m_id << ".dat";
  ofs.open(ss.str(), std::ios::app);
  INFO("Results file ", ss.str())
  MSG_ASSERT(ofs.is_open(), "Could not open file " + fn);

  for (auto &it : m_statesTime)
  {
    ofs << it.first << "\t";
    auto acc = 0.0;

    for (auto &it2 : it.second)
    {
      ofs << it2.second / NOW << "\t";
      acc += it2.second;
    }

    ofs << acc / NOW << "\n";
  }

  ofs.close();
  END;
}

void NodeMultiRegime2::ProbsResults(const std::string &fn) const
{
  BEG;
  std::ofstream ofs;
  std::stringstream ss;
  ss << fn << "_staysRegime_node_" << std::setw(3) << std::setfill('0') << m_id << ".dat";
  ofs.open(ss.str(), std::ios::app);
  INFO("Results file ", ss.str())
  MSG_ASSERT(ofs.is_open(), "Could not open file " + fn);
  auto tot = 0.0;
  for (auto &item : m_regimesStaysTime)
  {
    tot += std::accumulate(item.second.begin(), item.second.end(), 0);
  }
  for (auto &item : m_regimesStaysTime)
  {
    auto aux = std::accumulate(item.second.begin(), item.second.end(), 0);
    aux = (aux > 0 ? aux / tot : 0.0);
    ofs << aux << "\t";
  }
  ofs << "\n";

  for (auto &item : m_regimesStaysTime)
  {
    auto aux = std::accumulate(item.second.begin(), item.second.end(), 0.0);
    aux = aux > 0 ? aux / item.second.size() : 0;
    ofs << aux << "\t";
  }
  ofs << "\n";

  for (auto &item : m_regimesTransits)
  {
    for (auto &item2 : item.second)
    {
      ofs << item2.second << "\t";
    }
    ofs << "\n";
  }

  ofs.close();
  END;
}

void NodeMultiRegime2::QueueLogResults(const std::string &fn) const
{
  BEG;
  std::ofstream ofs;
  std::stringstream ss;
  ss << fn << "_queueLog_node_" << std::setw(3) << std::setfill('0') << m_id << ".dat";
  ofs.open(ss.str(), std::ios::app);
  INFO("Results file ", ss.str())
  MSG_ASSERT(ofs.is_open(), "Could not open file " + fn);
  for (auto &item : m_queueLog)
  {
    ofs << item.first
        << "\t" << item.second.first
        << "\t" << item.second.second
        << "\n";
  }
  ofs.close();
  END;
}