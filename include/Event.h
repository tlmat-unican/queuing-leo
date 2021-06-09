#ifndef EVENT_H
#define EVENT_H

#include "Common.h"
#include "Packet.h"
#include <iostream>

class Event {
public:
    Event(EvType_t type, Time_t time, Id_t nodeId, Packet* pkt = nullptr);
//    Event(const Event& ) = delete;
    ~Event();
    EvType_t GetType(void) const;
    Time_t GetTime(void) const;
    Id_t GetNodeId(void) const;
    Packet* GetPkt(void) const;
    
    bool operator<(const Event &ev) const;
    void Print (void) const;
private:
    const EvType_t m_type;
    const Time_t m_time;
    const Id_t m_nodeId;
    Packet* m_pkt;

    friend std::ostream &operator<<(std::ostream &os, Event const&m);
};




#endif /* EVENT_H */