#pragma once

#include <stack>
#include "flow_builder.hpp"

namespace tf {

/**
@class Taskflow 

@brief the class to create a task dependency graph

*/
class Taskflow : public FlowBuilder {

  friend class Topology;
  friend class Executor;

  public:

    /**
    @brief constructs the taskflow with an empty task dependency graph
    */
    Taskflow();

    /**
    @brief destroy the taskflow (virtual call)
    */
    virtual ~Taskflow();
    
    /**
    @brief dumps the taskflow to a std::ostream in DOT format

    @param ostream a std::ostream target
    */
    void dump(std::ostream& ostream) const;
    
    /**
    @brief dumps the taskflow in DOT format to a std::string
    */
    std::string dump() const;
    
    /**
    @brief queries the number of nodes in the taskflow
    */
    size_t num_nodes() const;

    /**
    @brief creates a module task from a taskflow

    @param taskflow a taskflow object to create the module
    */
    tf::Task composed_of(Taskflow& taskflow);

    /**
    @brief sets the name of the taskflow
    */
    auto& name(const std::string&) ; 

    /**
    @brief queries the name of the taskflow
    */
    const std::string& name() const ;

  private:
 
    std::string _name;
   
    Graph _graph;

    std::mutex _mtx;
    std::list<Topology*> _topologies;
};

// Constructor
inline Taskflow::Taskflow() : FlowBuilder{_graph} {
}

// Destructor
inline Taskflow::~Taskflow() {
  assert(_topologies.empty());
}

// Function: num_noces
inline size_t Taskflow::num_nodes() const {
  return _graph.size();
}

// Function: name
inline auto& Taskflow::name(const std::string &name) {
  _name = name;
  return *this;
}

// Function: name
inline const std::string& Taskflow::name() const {
  return _name;
}

// Function: composed_of
inline tf::Task Taskflow::composed_of(Taskflow& taskflow) {
  auto &node = _graph.emplace_back();
  node._module = &taskflow;
  return Task(node);
}

// Procedure: dump
inline std::string Taskflow::dump() const {
  std::ostringstream oss;
  dump(oss);
  return oss.str();
}

// Function: dump
inline void Taskflow::dump(std::ostream& os) const {

  std::stack<const Taskflow*> stack;
  std::unordered_set<const Taskflow*> visited; 
  
  os << "digraph Taskflow_";
  if(_name.empty()) os << 'p' << this;
  else os << _name;
  os << " {\nrankdir=\"LR\";\n";
  
  stack.push(this);
  visited.insert(this);
  
  while(!stack.empty()) {
    
    auto f = stack.top();
    stack.pop();
    
    // create a subgraph field for this taskflow
    os << "subgraph cluster_";
    if(f->_name.empty()) os << 'p' << f;
    else os << f->_name;
    os << " {\n";

    os << "label=\"Taskflow_";
    if(f->_name.empty()) os << 'p' << f;
    else os << f->_name;
    os << "\";\n";

    // dump the details of this taskflow
    for(const auto& n: f->_graph) {
      // regular task
      if(auto module = n._module; !module) {
        n.dump(os);
      }
      // module task
      else {
        os << 'p' << &n << "[shape=box3d, color=blue, label=\"";
        if(n._name.empty()) os << &n;
        else os << n._name;
        os << " (Taskflow_";
        if(module->_name.empty()) os << module;
        else os << module->_name;
        os << ")\"];\n";

        if(visited.find(module) == visited.end()) {
          visited.insert(module);
          stack.push(module);
        }

        for(const auto s : n._successors) {
          os << 'p' << &n << "->" << 'p' << s << ";\n";
        }
      }
    }
    os << "}\n";
  }

  os << "}\n";
}

// Backward compatibility
using Framework = Taskflow;

}  // end of namespace tf. ---------------------------------------------------

