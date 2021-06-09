#include "Topology.h"
#include "Log.h"
#include "Utils.h"

#include "NodeMm1.h"
#include "NodeMultiRegime2.h"

LOG_REGISTER_MODULE("Topology")

std::ostream &
operator<<(std::ostream &os, const Topology &o)
{
  os << "=========== TOPOLOGY ===============\n";
  os << "Number of nodes " << o.m_nodes.size() << "\n ";
  os << "Forwarding matrices \n";
  for (auto &it : o.m_forwardRules)
  {
    os << "Flow " << it.first << "\n";
    os << it.second << "\n";
  }
  os << "Delays matrix "
     << "\n";
  os << o.m_linkDelays << "\n";
  //
  for (auto &item : o.m_nodes)
  {
    os << "---------------------------------------\n";
    os << *item.second;
  }
  os << "====================================\n";
  return os;
}

Topology::Topology()
{
  BEG;
  m_forwardRules.clear();
  m_linkDelays.clear();
  m_nodes.clear();
  END;
}

void Topology::SetZeroNode(BaseNode *zn)
{
  BEGEND;
  m_zeroNode = zn;
}

BaseNode *
Topology::GetNode(Id_t nodeId)
{
  BEG;
  if (nodeId == ZN_ID)
  {
    END;
    return m_zeroNode;
  }
  auto iter = m_nodes.find(nodeId);
  MSG_ASSERT(iter != m_nodes.end(), "Requested node " + std::to_string(nodeId) + " does not exist");

  END;
  return iter->second.get();
}

Id_t Topology::FindNextNode(Id_t flowId, Id_t nodeId)
{
  BEG;
  MSG_ASSERT(m_forwardRules.find(flowId) != m_forwardRules.end(),
             "Undefined forwarding rules for flow " + std::to_string(flowId));
  auto fwd = m_forwardRules.at(flowId);
  MSG_ASSERT(int(fwd.size()) >= nodeId, "Bad node Id");
  auto ret = ZN_ID;
  auto rule = fwd.at(nodeId);
  //
  auto acc = 0.0;
  auto val = UnitRand();
  for (auto idx = 0u; idx < rule.size(); ++idx)
  {
    if (rule[idx] == 0.0)
    {
      continue;
    }
    if (acc < val and val < acc + rule[idx])
    {
      ret = idx;
      break;
    }
    acc += rule[idx];
  }
  END;
  DBG("Packet from Flow", flowId, " ==>> ", ret);
  return ret;
}

double
Topology::FindDelay(Id_t nodeSrc, Id_t nodeDst)
{
  BEG;
  MSG_ASSERT(int(m_linkDelays.size()) >= nodeSrc and int(m_linkDelays.size()) >= nodeDst, "Bad node Id");
  END;
  return m_linkDelays[nodeSrc][nodeDst];
}

void Topology::SetConfiguration(const json &j)
{
  BEG;
  m_numNodes = JsonCheckAndGet<decltype(m_numNodes)>(j, "/NumNodes");
  LoadFwdRules(j);
  LoadDelays(j);
  CreateNodes(j);
  END;
}

void Topology::GenerateResults(const std::string &fn)
{
  BEG;
  //    std::cout << "==== TOPOLOGY RESULTS ====" << std::endl;
  for (auto &item : m_nodes)
  {
    item.second->GenerateResults(fn);
  }
  //    std::cout << "====================================" << std::endl;
  END;
}

void Topology::LoadFwdRules(const json &j)
{
  BEG;
  JSON_CHECK(j, "ForwardRules");
  auto flowId = 1;
  auto auxStr = "Flow" + std::to_string(flowId);
  while (j["ForwardRules"].contains(auxStr))
  {
    m_forwardRules.insert({flowId, Matrix<>(j["ForwardRules"][auxStr])});
    auto ruleSens = 1e-2;
    if (j.contains("RulesSensitivity"))
    {
      ruleSens = j["RulesSensitivity"];
    }
    for (auto idx = 0u; idx < m_forwardRules.at(flowId).size(); ++idx)
    {
      auto aux = std::accumulate(m_forwardRules.at(flowId)[idx].begin(),
                                 m_forwardRules.at(flowId)[idx].end(), 0.0);
      MSG_ASSERT((std::abs(1 - aux) <= ruleSens),
                 auxStr + " --> Incorrect forwarding rule probabilities of node " + std::to_string(idx));
    }
    ++flowId;
    auxStr = "Flow" + std::to_string(flowId);
  }

  END;
}

void Topology::LoadDelays(const json &j)
{
  BEG;
  MSG_ASSERT(j.contains("DefaultLinkDelay") or j.contains("LinksDelay"), "Link delay not set");

  if (!j.contains("LinksDelay"))
  {
    auto defaultDelay = double(j["DefaultLinkDelay"]);
    InitMatrix(m_linkDelays, m_numNodes, m_numNodes, defaultDelay);
  }
  else
  {
    m_linkDelays = Matrix<>(j["LinksDelay"]);
  }
  END;
}

void Topology::CreateNodes(const json &j)
{
  BEG;
  for (auto idx = 1u; idx < m_numNodes; ++idx)
  {
    auto auxStr = "Node" + std::to_string(idx);
    json auxj;
    if (j.contains(auxStr))
    {
      auxj = j[auxStr];
    }
    else
    {
      JSON_CHECK(j, "DefaultNode");
      auxj = j["DefaultNode"];
    }
    DoCreateNode(auxj, idx);
  }
  END;
}

void Topology::DoCreateNode(const json &j, Id_t idx)
{
  BEG;
  JSON_CHECK(j, "Type");

  auto type = std::string(j["Type"]);
  if (type == "MM1")
  {
    DBG("Node ", idx, " of type MM1");
    DoCreateMm1Node(j, idx);
  }
  else if (type == "ML2")
  {
    DBG("Node ", idx, " of type ML2");
    DoCreateMl2Node(j, idx);
  }
  else
  {
    MSG_ASSERT(false, "Unknown node type " + type);
  }
  END;
}

void Topology::DoCreateMm1Node(const json &j, Id_t idx)
{
  BEG;
  JSON_CHECK(j, "Dist");
  JSON_CHECK(j, "Param");
  JSON_CHECK(j, "QueueSize");

  auto dt = STR_2_DIST_TYPE(j["Dist"]);
  auto servParam = double(j["Param"]);
  auto queueSize = double(j["QueueSize"]);
  m_nodes.insert({idx, std::make_unique<NodeMm1>(
                           idx,
                           dt,
                           servParam,
                           queueSize)});
  END;
}

void Topology::DoCreateMl2Node(const json &j, Id_t idx)
{
  BEG;
  JSON_CHECK(j, "Service");
  JSON_CHECK(j["Service"], "Dist");
  JSON_CHECK(j["Service"], "Param");
  JSON_CHECK(j, "Regime");
  JSON_CHECK(j["Regime"], "Dist");
  JSON_CHECK(j["Regime"], "Param");
  JSON_CHECK(j["Regime"], "Prob");
  JSON_CHECK(j, "QueueSize");

  auto servDist = STR_2_DIST_TYPE(j["Service"]["Dist"]);
  auto servParam = std::vector<double>(j["Service"]["Param"]);

  auto regimeDist = STR_2_DIST_TYPE(j["Regime"]["Dist"]);
  auto regimeParam = std::vector<double>(j["Regime"]["Param"]);
  auto regimeProb = std::vector<std::vector<double>>(j["Regime"]["Prob"]);

  NodeMultiRegime2::Config mlc;
  mlc.m_servDist = servDist;
  mlc.m_servParam = servParam;
  mlc.m_regimeDist = regimeDist;
  mlc.m_regimeParam = regimeParam;
  mlc.m_regimeProb = regimeProb;

  m_nodes.insert({idx, std::make_unique<NodeMultiRegime2>(
                           idx,
                           int(j["QueueSize"]),
                           mlc)});
}

Id_t Topology::GetnNodes(void) const
{
  BEGEND;
  return m_nodes.size();
}