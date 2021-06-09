#include "BaseNode.h"
#include "Log.h"

LOG_REGISTER_MODULE("BaseNode")

std::ostream& 
operator<<(std::ostream& os, const BaseNode& e) {
    return e.Print(os);
}

BaseNode::BaseNode(Id_t id)
: m_id(id) {
    BEGEND;
}

BaseNode::~BaseNode() {
    BEGEND;
}
