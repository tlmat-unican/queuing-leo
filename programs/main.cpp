#include <cstdlib>
#include <iostream>
#include <iomanip>
#include "Simulator.h"
#include "Utils.h"
#include "Common.h"
#include "TraceLoader.h"
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

int main(int argc, char *argv[])
{
  // LOG_SET_ALL_LEVEL(LogLevel::DEBUG);
  LOG_SET_LEVEL("NodeMultiRegime2", LogLevel::DEBUG);

  MSG_ASSERT(argc == 2, "Bad number of arguments");
  std::string confFile = argv[1];
  std::ifstream ifs(confFile);
  MSG_ASSERT(ifs.is_open(), "Cannot find config file");
  json j;
  ifs >> j;

  TOPOLOGY.SetConfiguration(j["Topology"]);
  // std::cout << TOPOLOGY << std::endl;
  PKT_STORE.SetConfiguration(j);
  Simulator simul(j["Simulation"]);

  auto resPath = JsonCheckAndGet<>(j, "/Results/ResultsPath");
  fs::create_directories(resPath);

  auto start = SYSTEM_NOW;
  simul.Run();
  auto end = SYSTEM_NOW;
  std::cout << "**** ELEPASED TIME " << DurationSec(end, start) << " sec for " << NOW << " ms of simulation \t****\n";

  if (JsonContains(j, "/Results/NodesMetrics"))
  {
    // Node state probabilities
    auto str = JsonCheckAndGet<>(j, "/Results/NodesMetrics");
    TOPOLOGY.GenerateResults(resPath + "/" + str);
  }

  return 0;
}