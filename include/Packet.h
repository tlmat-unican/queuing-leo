#ifndef PACKET_H
#define PACKET_H

#include <map>
#include "Common.h"

class Packet
{
public:
  enum class Fwd : std::uint8_t
  {
    ToNode,
    ToQueue,
    ToProcessor,
    ToAir,
    None
  };

  Packet(Id_t pktId, Id_t flowId);
  Packet(const Packet&) = delete;
  ~Packet();

  void ForwardTo(Fwd fwd, Id_t nodeId);
  Id_t GetFlowId(void) const;
  Id_t GetPacketId(void) const;

  void Drop(void);
  bool IsDropped(void) const;
  bool IsEnded(void) const;
  double GetDelay() const;
  void SetStart();
  void SetEnd();

  Time_t GetProcessingTime(void) const;
  Time_t GetWaitingTime(void) const;
  Time_t GetAirTime(void) const;
  Time_t GetTimeInNode(Id_t) const;

private:
  Id_t m_id;
  Id_t m_flowId;
  Id_t m_currentNodeId;
  bool m_dropped;
  bool m_ended;
  Fwd m_lastFwd;
  Time_t m_inQueue;
  Time_t m_inProcessor;
  Time_t m_lastFwdTime;
  Time_t m_inAir;

  Time_t m_startTime;
  Time_t m_endTime;
  std::map<Id_t, Time_t> m_nodeIn;
  std::map<Id_t, Time_t> m_nodeOut;

  using PktMove_t = std::pair<Fwd, Id_t>;
  using NodeHist_t = std::multimap<Time_t, PktMove_t>;
  NodeHist_t m_nodeHist;

  friend std::ostream &operator<<(std::ostream &os, const Packet &o);
};

#endif /* PACKET_H */
