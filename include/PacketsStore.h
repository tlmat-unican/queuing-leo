#ifndef PACKETSSTORE_H
#define PACKETSSTORE_H

#include <memory>
#include <map>

#include "Common.h"
#include "Packet.h"

class PacketsStore
{

  enum MetricsGlob
  {
    G_Delay = 0,
    G_WaitingTime = 1,
    G_ProcessingTime = 2,
    G_PropagationTime = 3,
    G_Dropped = 4,
    G_Counter = 5,
    G_CounterSuccess = 6
  };

  enum MetricsPacket
  {
    P_Delay = 0,
    P_WaitingTime = 1,
    P_ProcessingTime = 2,
    P_PropagationTime = 3,
    P_Dropped = 4
  };

  enum MetricsNodeTime
  {
    M_Time = 0,
    M_Counter = 1
  };

public:
  static PacketsStore &GetInstance()
  {
    static PacketsStore instance;
    return instance;
  }

  Packet *CreatePacket(Id_t flowId);
  Packet *GetPacket(Id_t id);
  void DeletePacket(Packet *pkt);

  void SetConfiguration(const json &j);
  void GenerateResults(void) const;
  //    void Print(void) const;

private:
  PacketsStore();
  PacketsStore(PacketsStore const &) = delete;
  void operator=(PacketsStore const &) = delete;

  void RecordGlobalMetrics(Packet *pkt);
  void RecordPacketsTrace(Packet *pkt);
  void RecordNodesTime(Packet *pkt);

  void GenerateGlobalMetrics() const;
  void GeneratePacketsTrace() const;
  void GenerateNodeTime() const;

  using PacketList_t = std::map<Id_t, std::unique_ptr<Packet>>;
  PacketList_t m_packets;

  bool m_recordGlobalMetrics;
  bool m_recordPacketTrace;
  bool m_recordNodesTimes;

  std::map<Id_t, std::tuple<double, double, double, double, std::uint32_t, std::uint32_t, std::uint32_t>> m_globMetrics;
  std::map<Id_t, std::vector<std::tuple<double, double, double, double, bool>>> m_pktTraces;
  std::map<Id_t, Time_t> m_lastDepartureTime;
  std::map<Id_t, std::vector<double>> m_interDepartureTime;

  std::vector<Id_t> m_nodeTimesRange;
  std::map<Id_t, std::map<Id_t, std::tuple<double, std::uint32_t>>> m_nodeTimes;

  std::string m_globFile;
  std::string m_pktTracesFile;
  std::string m_nodeTimesFile;

  friend std::ostream &operator<<(std::ostream &os, const PacketsStore &o);
};

#endif /* PACKETSSTORE_H */