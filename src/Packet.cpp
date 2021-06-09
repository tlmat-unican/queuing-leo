#include "Packet.h"
#include "Log.h"
#include "Utils.h"

LOG_REGISTER_MODULE("Packet")

static const std::map<Packet::Fwd, std::string> PktFwdStr{
    {Packet::Fwd::ToNode, "TO_NODE"},
    {Packet::Fwd::ToQueue, "TO_QUEUE"},
    {Packet::Fwd::ToProcessor, "TO_PROCESSOR"},
    {Packet::Fwd::ToAir, "TO_AIR"}};

std::ostream &
operator<<(std::ostream &os, const Packet &o)
{
  os << "=============== PACKET_" << o.m_id << "(flow " << o.m_flowId << ")=================\n";
  Time_t prevT = 0.0;
  for (auto &item : o.m_nodeHist)
  {
    std::cout << "  At " << item.first << "(" << (item.first - prevT) << ") " << PktFwdStr.at(item.second.first)
              << " " << item.second.second << std::endl;
    prevT = item.first;
  }
  os << "Delay " << o.GetDelay() << "\n";
  os << "========================================\n";
  return os;
}

Packet::Packet(Id_t pktId, Id_t flowId)
    : m_id(pktId), m_flowId(flowId), m_currentNodeId(ZN_ID), m_dropped(false), m_ended(false), m_lastFwd(Fwd::None), m_inQueue(0.0), m_inProcessor(0.0), m_lastFwdTime(0.0), m_inAir(0.0)
{
  BEGEND;
}

Packet::~Packet()
{
  BEGEND;
}

void Packet::ForwardTo(Fwd fwd, Id_t nodeId)
{
  BEG;
  m_nodeHist.emplace(NOW, PktMove_t(fwd, nodeId));
  if (fwd == Fwd::ToNode)
  {
    m_currentNodeId = nodeId;
    m_nodeIn[m_currentNodeId] = NOW;
  }
  else if (fwd == Fwd::ToAir)
  {
    m_nodeOut[m_currentNodeId] = NOW;
  }

  if (m_lastFwd == Fwd::ToProcessor)
  {
    m_inProcessor += (NOW - m_lastFwdTime);
  }
  else if (m_lastFwd == Fwd::ToQueue)
  {
    m_inQueue += (NOW - m_lastFwdTime);
  }
  else if (m_lastFwd == Fwd::ToAir)
  {
    m_inAir += (NOW - m_lastFwdTime);
  }

  m_lastFwd = fwd;
  m_lastFwdTime = NOW;
  END;
}

Id_t Packet::GetFlowId(void) const
{
  BEGEND;
  return m_flowId;
}

Id_t Packet::GetPacketId(void) const
{
  BEGEND;
  return m_id;
}

void Packet::Drop(void)
{
  BEG;
  m_dropped = true;
  END;
}

bool Packet::IsDropped(void) const
{
  BEGEND;
  return m_dropped;
}

bool Packet::IsEnded(void) const
{
  BEGEND;
  return m_ended;
}

Time_t
Packet::GetDelay() const
{
  BEG;
  auto ret = 0.0;
  if (m_dropped)
  {
    ret = -1;
  }
  else
  {
    ret = m_endTime - m_startTime;
  }
  END;
  return ret;
}

void Packet::SetStart()
{
  BEG;
  m_startTime = NOW;
  END;
}

void Packet::SetEnd()
{
  BEG;
  m_endTime = NOW;
  m_ended = true;
  END;
}

Time_t
Packet::GetProcessingTime(void) const
{
  BEGEND;
  return m_inProcessor;
}

Time_t
Packet::GetWaitingTime(void) const
{
  BEGEND;
  return m_inQueue;
}

Time_t
Packet::GetAirTime(void) const
{
  BEGEND;
  MSG_ASSERT(m_inAir >= 0, "Error with air time!!");
  return m_inAir;
}

Time_t
Packet::GetTimeInNode(Id_t nodeId) const
{
  BEG;
  Time_t diff = 0;
  if (m_nodeIn.find(nodeId) != m_nodeIn.end() and m_nodeOut.find(nodeId) != m_nodeOut.end())
  {
    diff = m_nodeOut.at(nodeId) - m_nodeIn.at(nodeId);
  }
  END;
  return diff;
}