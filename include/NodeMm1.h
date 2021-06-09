#ifndef SIMPLENODE_H
#define SIMPLENODE_H

#include <deque>

#include "BaseNode.h"
#include "Common.h"

class Packet;
class NodeMm1 final : public BaseNode
{
public:
  NodeMm1(Id_t nodeid, DistType dt, double servParam, int qsize);
  virtual ~NodeMm1();

  void OnEvent(void) override;
  std::ostream &Print(std::ostream &os) const override;

  double GetBusyRatio(void) const;
  std::map<std::uint32_t, double> GetStatesTime(void) const;
  double GetAvgElemNumber(void) const;
  double GetAvgQueueSize(void) const;
  void GenerateResults(const std::string &fn) const override;

private:
  void OnArrival();
  void OnDeparture();
  void SendToNext();
  void UpdateState(void);
  void UpdateQueueState(void);

  const DistType m_servDist;
  const double m_servParam;
  const int m_maxQueueSize;
  Packet *m_currentPkt;
  Time_t m_stateChange;
  Time_t m_queueStateChange;

  std::deque<Packet *> m_queue;
  std::map<std::uint32_t, double> m_statesTime;
  std::map<std::uint32_t, double> m_queueStatesTime;

  friend std::ostream &operator<<(std::ostream &os, const NodeMm1 &o);
};

#endif /* SIMPLENODE_H */
