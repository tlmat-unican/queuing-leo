
#ifndef TRACELOADER_H
#define TRACELOADER_H

#include "Common.h"


class TraceLoader {
public:
    TraceLoader(std::string&& fn);
    ~TraceLoader();
private:
    const std::string m_fn;
    void LoadTrace();
    void AddStopEvent(Id_t nodeid, Time_t time);
    void AddRestartEvent(Id_t nodeid, Time_t time);

};

#endif /* TRACELOADER_H */

