#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <iostream>

#include "Common.h"
#include "BaseNode.h"
#include "RandGenerator.h"

class Topology {
public:

    static Topology &GetInstance() {
        static Topology instance;
        return instance;
    }

    void SetZeroNode(BaseNode* zn);
    BaseNode *GetNode(Id_t nodeId);
    Id_t FindNextNode(Id_t flowId, Id_t nodeId);
    double FindDelay(Id_t nodeSrc, Id_t nodeDst);
    void SetConfiguration(const json& j);
    void GenerateResults(const std::string& fn);
    Id_t GetnNodes(void) const;

private:
    Topology();
    Topology(Topology const &) = delete;
    void operator=(Topology const &) = delete;

    void LoadFwdRules(const json& j);
    void LoadDelays(const json& j);
    void CreateNodes(const json& j);
    void DoCreateNode(const json& j, Id_t idx);
    void DoCreateMm1Node(const json& j, Id_t idx);
    void DoCreateMl2Node(const json& j, Id_t idx);

    using NodePtr_t = std::unique_ptr<BaseNode>;
    using NodeList_t = std::map<Id_t, NodePtr_t>;

    BaseNode* m_zeroNode;
    NodeList_t m_nodes;

    std::map<Id_t, Matrix<>> m_forwardRules;
    std::uint16_t m_numNodes;
    Matrix<> m_linkDelays;

    friend std::ostream& operator<<(std::ostream &os, const Topology& o);
};

#endif /* TOPOLOGY_H */