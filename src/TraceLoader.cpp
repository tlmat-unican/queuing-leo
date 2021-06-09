#include "TraceLoader.h"
#include "Utils.h"
#include "Log.h"
#include <sstream>

LOG_REGISTER_MODULE("TraceLoader");

TraceLoader::TraceLoader(std::string&& fn)
: m_fn(fn) {
    LoadTrace();
}

TraceLoader::~TraceLoader() {
}

void
TraceLoader::LoadTrace() {
    BEG;
    std::ifstream ifs(m_fn);
    std::string line;
    Time_t time;
    Id_t nodeId;
    double state;
    while (getline(ifs, line)) {
        std::stringstream ss(line);
        std::string token;
        std::getline(ss, token, ',');
        time = std::stod(token);
        std::getline(ss, token, ',');
        nodeId = std::stoi(token);
        std::getline(ss, token, ',');
        state = std::stod(token);
        INFO ("At ", time, " node ", nodeId, " to ", state);
        if (state != 0) {
            AddRestartEvent(nodeId, time);
        } else {
            AddStopEvent(nodeId, time);
        }
    }
    END;
}

void
TraceLoader::AddStopEvent(Id_t nodeid, Time_t time) {
    BEG;
    GENERATE_EVENT(EvType_t::NodeStop, time, nodeid, nullptr);
    END;
}

void
TraceLoader::AddRestartEvent(Id_t nodeid, Time_t time) {
    BEG;
    GENERATE_EVENT(EvType_t::NodeRestart,  time, nodeid, nullptr);
    END;
}

