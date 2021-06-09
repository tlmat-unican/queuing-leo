#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <map>

#include "BaseNode.h"
#include "Common.h"
#include "RandGenerator.h"

class Simulator : public BaseNode
{
public:
  Simulator(const json &j);
  ~Simulator();

  void OnEvent(void) override;
  std::ostream &Print(std::ostream &ofs) const override;
  void Run(void);
  void GenerateResults([[maybe_unused]] const std::string &fn) const override{};

private:
  struct FlowConf
  {
    DistType m_dt;
    double m_param;
    std::uint32_t m_maxPkts;
    std::uint32_t m_txPkt;
    std::uint32_t m_rxPkt;
    RandGen m_rg;
  };
  std::uint32_t m_finishedFlows;

  SimulType m_simulType;
  Time_t m_maxSimulTime;

  void OnArrival();
  void OnPacketFinish();
  void ProcessEvent();
  void GenerateArrival(Id_t flowId, bool absTime = false);
  bool ForceStop(void) const;

  std::map<Id_t, FlowConf> m_flows;

  friend std::ostream &operator<<(std::ostream &os, const Simulator &o);
};

#endif /* SIMULATOR_H */