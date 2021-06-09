#ifndef NODEMULTIREGIME2_H
#define NODEMULTIREGIME2_H

#include <deque>

#include "BaseNode.h"
#include "Common.h"
#include "RandGenerator.h"

class Packet;
class NodeMultiRegime2 : public BaseNode
{
public:
  struct Config
  {
    DistType m_servDist;
    std::vector<double> m_servParam;
    DistType m_regimeDist;
    std::vector<double> m_regimeParam;
    Matrix<double> m_regimeProb;
  };

  NodeMultiRegime2(Id_t nodeid, int maxQsize, NodeMultiRegime2::Config conf);
  virtual ~NodeMultiRegime2();

  void OnEvent(void) override;
  std::ostream &Print(std::ostream &os) const override;
  void GenerateResults(const std::string &fn) const override;

private:
  void OnArrival();
  void OnDeparture();
  void OnRegimeChange();
  void SendToNext();
  void SaveProcessFromQueue();
  std::uint32_t SelectRegime();
  void GenerateRegimeChange();
  void GenerateDeparture();
  void UpdateStateTrace(void);

  void UpdateQueueStateTrace(void);
  void UpdateRegimeTrace(std::uint32_t newRegime);
  void StatesResults(const std::string &fn) const;
  void ProbsResults(const std::string &fn) const;
  void QueueLogResults(const std::string &fn) const;

  NodeMultiRegime2::Config m_mlConfig;
  const int m_maxQueueSize;
  bool m_controllerActive;
  Packet *m_currentPkt;
  std::uint32_t m_currentRegime;
  Time_t m_stateChange;
  Time_t m_queueStateChange;
  Time_t m_regimeChange;

  std::deque<Packet *> m_queue;
  std::map<std::uint32_t, std::map<std::uint32_t, double>> m_statesTime;
  std::map<std::uint32_t, std::vector<double>> m_regimesStaysTime;
  //  std::map<std::uint32_t, double> m_regimesStaysTime;
  std::map<std::uint32_t, std::map<std::uint32_t, std::uint32_t>> m_regimesTransits;
  std::map<std::uint32_t, std::map<std::uint32_t, double>> m_queueStatesTime;
  std::map<double, std::pair<std::uint64_t, std::uint32_t>> m_queueLog;
  friend std::ostream &operator<<(std::ostream &os, const NodeMultiRegime2 &o);
};

#endif /* NODEMULTIREGIME2_H */
