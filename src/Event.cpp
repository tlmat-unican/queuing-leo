#include "Event.h"
#include "Log.h"

LOG_REGISTER_MODULE("Event")

std::ostream &
operator<<(std::ostream &os, const Event &o)
{
  os << "Event " << EV_TYPE_2_STR(o.m_type)
     << "\t at " << o.m_time
     << " | node " << o.m_nodeId;
  if (o.m_pkt != nullptr)
  {
    os << " | packet " << o.m_pkt->GetPacketId();
  }
  os << "\n";
  return os;
}

void Event::Print(void) const
{
  std::cout << "Event " << EV_TYPE_2_STR(m_type)
            << "\t at " << m_time
            << " | node " << m_nodeId;
  if (m_pkt != nullptr)
  {
    std::cout << " | packet " << m_pkt->GetPacketId();
  }
  std::cout << "\n";
}

Event::Event(EvType_t type, Time_t time, Id_t nodeId, Packet *pkt)
    : m_type(type), m_time(time), m_nodeId(nodeId), m_pkt(pkt)
{
  BEGEND;
  DBG("Generate event of type ", EV_TYPE_2_STR(type), " at ", time, " for node ", m_nodeId);
}

Event::~Event()
{
  BEGEND;
}

EvType_t
Event::GetType(void) const
{
  BEGEND;
  return m_type;
}

Time_t
Event::GetTime(void) const
{
  BEGEND;
  return m_time;
}

Id_t Event::GetNodeId(void) const
{
  BEGEND;
  return m_nodeId;
}

Packet *
Event::GetPkt(void) const
{
  BEGEND;
  return m_pkt;
}

bool Event::operator<(const Event &ev) const
{
  return ev.m_time > m_time;
}
