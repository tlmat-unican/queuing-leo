#include <iomanip>

#include "NodeMm1.h"
#include "Log.h"
#include "Utils.h"

LOG_REGISTER_MODULE("NodeMm1");

std::ostream &
operator<<(std::ostream &os, const NodeMm1 &o) {
    return o.Print(os);
}

std::ostream &
NodeMm1::Print(std::ostream &os) const {
    BEG;
    os << "NODE_" << m_id << "(type MM1)\n";
    END;
    return os;
}

NodeMm1::NodeMm1(Id_t nodeid, DistType dt, double servParam, int qsize)
: BaseNode(nodeid), m_servDist(dt), m_servParam(servParam),
m_maxQueueSize(qsize), m_currentPkt(nullptr), m_stateChange(0.0),
m_queueStateChange(0.0) {
    BEGEND;
}

NodeMm1::~NodeMm1() {
    BEGEND;
}

void NodeMm1::OnEvent(void) {
    BEG;
    DBG(NOW, " | ", m_id, " ", EV_TYPE_2_STR(CURRENT_EVENT->GetType())
            , " packet ", CURRENT_EVENT->GetPkt() != nullptr
            ? CURRENT_EVENT->GetPkt()->GetPacketId() : 0);
    switch (CURRENT_EVENT->GetType()) {
        case EvType_t::Arrival:
            OnArrival();
            break;
        case EvType_t::Departure:
            OnDeparture();
            break;
        default:
            MSG_ASSERT(false, "Unknown event " + EV_TYPE_2_STR(CURRENT_EVENT->GetType()));
            break;
    }
    END;
}

double
NodeMm1::GetBusyRatio(void) const {
    BEGEND;
    return 1;
}

std::map<std::uint32_t, double>
NodeMm1::GetStatesTime(void) const {
    BEGEND;
    return m_statesTime;
}

double
NodeMm1::GetAvgElemNumber(void) const {
    BEG;
    auto acc = 0.0;
    for (auto &item : m_statesTime) {
        acc += (item.first * item.second);
    }
    END;
    return (acc / NOW);
}

double
NodeMm1::GetAvgQueueSize(void) const {
    BEGEND;
    auto acc = 0.0;
    for (auto &item : m_queueStatesTime) {
        acc += (item.first * item.second);
    }
    END;
    return (acc / NOW);
}

void NodeMm1::OnArrival() {
    BEG;
    auto pkt = CURRENT_EVENT->GetPkt();
    pkt->ForwardTo(Packet::Fwd::ToNode, m_id);

    if (m_currentPkt == nullptr) {
        MSG_ASSERT(m_queue.size() == 0, "Node bad behavior");
        UpdateState();
        m_currentPkt = pkt;
        auto time = GetRandSample(m_servDist, m_servParam);
        GENERATE_EVENT(EvType_t::Departure, NOW + time, m_id, m_currentPkt);
        pkt->ForwardTo(Packet::Fwd::ToProcessor, m_id);
    } else if (m_maxQueueSize < 0 or int(m_queue.size()) < m_maxQueueSize) {
        UpdateState();
        UpdateQueueState();
        m_queue.push_back(pkt);
        pkt->ForwardTo(Packet::Fwd::ToQueue, m_id);
    } else {
        pkt->Drop();
        GENERATE_EVENT(EvType_t::PacketFinish, NOW, ZN_ID, pkt);
    }
    END;
}

void NodeMm1::OnDeparture() {
    BEG;
    SendToNext();
    UpdateState();
    m_currentPkt = nullptr;
    if (!m_queue.empty()) {
        UpdateState();
        m_currentPkt = m_queue.front();
        UpdateQueueState();
        m_queue.pop_front();
        
        auto time = GetRandSample(m_servDist, m_servParam);
        GENERATE_EVENT(EvType_t::Departure, NOW + time, m_id, m_currentPkt);
        m_currentPkt->ForwardTo(Packet::Fwd::ToProcessor, m_id);
    }
    END;
}

void NodeMm1::SendToNext() {
    BEG;
    auto pkt = CURRENT_EVENT->GetPkt();
    auto nodeId = FIND_NEXT_NODE(pkt->GetFlowId(), m_id);
    pkt->ForwardTo(Packet::Fwd::ToAir, nodeId);

    auto delay = FIND_DELAY(m_id, nodeId);

    if (nodeId != ZN_ID) {
        GENERATE_EVENT(EvType_t::Arrival, NOW + delay, nodeId, pkt);
    } else {
        GENERATE_EVENT(EvType_t::PacketFinish, NOW + delay, nodeId, pkt);
    }
    END;
}

void NodeMm1::UpdateState(void) {
    BEG;
    auto delta = NOW - m_stateChange;
    m_stateChange = NOW;
    auto busy = (m_currentPkt != nullptr ? 1 : 0);
    auto state = m_queue.size() + busy;

    if (m_statesTime.find(state) == m_statesTime.end()) {
        m_statesTime.insert({state, 0.0});
    }
    m_statesTime[state] += delta;
    END;
}

void NodeMm1::UpdateQueueState(void) {
    BEG;
    auto delta = NOW - m_queueStateChange;
    m_queueStateChange = NOW;
    auto state = m_queue.size();

    if (m_queueStatesTime.find(state) == m_queueStatesTime.end()) {
        m_queueStatesTime.insert({state, 0.0});
    }

    m_queueStatesTime[state] += delta;
    END;
}

void NodeMm1::GenerateResults(const std::string &fn) const {
    BEG;
    std::ofstream ofs;
    std::stringstream ss;
    ss << fn << "_node_" << std::setw(3) << std::setfill('0') << m_id << ".dat";
    ofs.open(ss.str(), std::ios::app);
    INFO("Results file ", ss.str())
    MSG_ASSERT(ofs.is_open(), "Could not open file " + fn);

    for (auto &it : m_statesTime) {
        ofs << it.first << "\t"
                << it.second / NOW << "\n";
    }

    ofs.close();
    END;
}