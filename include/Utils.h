#ifndef UTILS_H
#define UTILS_H

#include "EventsList.h"
#define EV_LIST EventsList::GetInstance()
#define NEXT_EVENT EventsList::GetInstance().NextEvent ()
#define CURRENT_EVENT EventsList::GetInstance().PickCurrentEvent()
#define NOW EventsList::GetInstance().GetCurrentTime()
#define GENERATE_EVENT(type, time, nodeId, pktId) \
    EventsList::GetInstance().CreateEvent(type, time, nodeId, pktId)
#define REMOVE_DEPARTURES(nodeId) \
    EventsList::GetInstance().RemoveDepartures(nodeId)

#include "Topology.h"
#define TOPOLOGY Topology::GetInstance()
#define GET_NODE(id) \
    Topology::GetInstance().GetNode(id)
#define FIND_NEXT_NODE(flowId, nodeId) \
    Topology::GetInstance().FindNextNode(flowId, nodeId)
#define FIND_DELAY(nodeSrc, nodeDst) \
    Topology::GetInstance().FindDelay(nodeSrc, nodeDst)

#include "PacketsStore.h"
#define PKT_STORE PacketsStore::GetInstance()
#define GET_PACKET(id) \
    PacketsStore::GetInstance().GetPacket(id)
#define CREATE_PACKET(flowId) \
    PacketsStore::GetInstance().CreatePacket(flowId)

#endif /* UTILS_H */