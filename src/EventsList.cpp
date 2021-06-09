#include "EventsList.h"
#include "Log.h"

#include "BaseNode.h"
#include "Utils.h"

LOG_REGISTER_MODULE("EventsList")

std::ostream &
operator<<(std::ostream &os, const EventsList &o)
{
  os << "========== EVENT LIST =============\n";
  os << "Event list content at " << NOW << "\n";
  os << "------------------------------\n";
  for (auto it = o.m_events.begin(); it != o.m_events.end(); ++it)
  {
    os << *(it);
  }
  os << "=====================================\n";
  return os;
}

void EventsList::Print(void) const
{
  std::cout << "Event list content at " << NOW << "\n";
  std::cout << "------------------------------\n";
  for (auto it = m_events.begin(); it != m_events.end(); ++it)
  {
    it->Print();
  }
  std::cout << "=====================================\n";
}

EventsList::EventsList()
    : m_currentTime(0.0), m_currentEvent(nullptr)
{
  BEGEND;
}

void EventsList::CreateEvent(EvType_t type, Time_t time, Id_t nodeId, Packet *pkt)
{
  BEG;
  auto isFirst = m_events.size() == 0 ? true : false;
  m_events.emplace(Event(type, time, nodeId, pkt));
  if (isFirst or time < m_currentTime)
  {
    UpdateCurrentInfo();
  }
  END;
}

void EventsList::NextEvent(void)
{
  BEG;
  if (m_events.size() > 0)
  {
    m_events.erase(m_events.begin());
  }

  if (m_events.size() > 0)
  {
    UpdateCurrentInfo();
  }
  else
  {
    m_currentEvent = nullptr;
    // We keep the last time
  }
  END;
}

const Event *
EventsList::PickCurrentEvent(void) const
{
  BEGEND;
  return m_currentEvent;
}

Time_t
EventsList::GetCurrentTime(void) const
{
  BEGEND;
  return m_currentTime;
}

void EventsList::RemoveDepartures(Id_t nodeId)
{
  BEG;
  for (auto it = m_events.begin(); it != m_events.end(); it++)
  {
    if ((it->GetNodeId() == nodeId) and
        (it->GetType() == EvType_t::Departure))
    {
      it = m_events.erase(it);
    }
  }
  UpdateCurrentInfo();
  END;
}

void EventsList::Flush()
{
  BEG;
  m_events.clear();
  m_currentEvent = nullptr;
  // We keep the last time
  END;
}

void EventsList::UpdateCurrentInfo(void)
{
  BEG;
  if (m_events.size() > 0)
  {
    m_currentTime = m_events.begin()->GetTime();
    m_currentEvent = (Event *)(&(*m_events.begin()));
  }
  else
  {
    m_currentEvent = nullptr;
    m_currentTime = -1;
  }
  END;
}
