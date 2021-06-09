#ifndef EVENTSLIST_H
#define EVENTSLIST_H

#include <memory>
#include <set>
#include <iostream>

#include "Event.h"

class EventsList {
public:

    static EventsList& GetInstance() {
        static EventsList instance;
        return instance;
    }

    void CreateEvent(EvType_t t, Time_t time, Id_t nodeId, Packet* pkt = nullptr);
    void NextEvent(void);
    const Event* PickCurrentEvent(void) const;
    Time_t GetCurrentTime(void) const;
    void RemoveDepartures(Id_t nodeId);
    void Flush();
    void Print (void) const;

private:

    struct EventPtrComp {

        bool operator()(const Event& lhs, const Event& rhs) const {
            if (lhs.GetTime() == rhs.GetTime()
                    and (rhs.GetType() == EvType_t::NodeStop
                    or rhs.GetType() == EvType_t::NodeRestart)) {
                return true;
            }
            return (lhs.GetTime() < rhs.GetTime());
        }
    };
    using EvList_t = std::multiset<Event, EventPtrComp>;

    EventsList();
    EventsList(EventsList const&) = delete;
    void operator=(EventsList const&) = delete;
    void UpdateCurrentInfo(void);

    EvList_t m_events;
    Time_t m_currentTime;
    Event* m_currentEvent;

    friend std::ostream& operator<<(std::ostream &os, const EventsList& o);

};



#endif /* EVENTSLIST_H */