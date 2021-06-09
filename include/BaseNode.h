#ifndef BASENODE_H
#define BASENODE_H

#include "Common.h"

class BaseNode {
public:
    BaseNode(Id_t id);

    virtual ~BaseNode();
    virtual void OnEvent() = 0;
    virtual std::ostream& Print(std::ostream & os) const  = 0;
    friend std::ostream& operator<<(std::ostream& os, const BaseNode& e);
    virtual void GenerateResults(const std::string& fn) const = 0;

protected:
    BaseNode() = delete;
    const Id_t m_id;
};



#endif /* BASENODE_H */