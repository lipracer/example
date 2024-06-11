
#ifndef INCLUDE_GRAPH_H
#define INCLUDE_GRAPH_H

#include <iostream>
#include <list>
#include <unordered_map>
#include <vector>
#include <functional>
#include "STLExt.h"

namespace example {

template <typename T>
class GraphNode {
 public:
  using pointer = GraphNode<T>*;
  using const_pointer = const GraphNode<T>*;

  GraphNode(T&& data) : data_(std::forward<T>(data)) {}

  auto& predecessor() { return predecessor_; }
  auto& successor() { return successor_; }

  const auto& predecessor() const { return predecessor_; }
  const auto& successor() const { return successor_; }

  void addPredecessor(GraphNode<T>* node) {
    predecessor_.push_back(node);
    node->successor_.push_back(this);
  }
  void addSuccessor(GraphNode<T>* node) {
    successor_.push_back(node);
    node->predecessor_.push_back(this);
  }

  template <typename U>
  friend std::ostream& operator<<(std::ostream& os, const GraphNode<U>& node);

 private:
  T data_{};
  std::vector<GraphNode<T>*> predecessor_;
  std::vector<GraphNode<T>*> successor_;
};

template <typename T>
class Graph {
 public:
  void push_back(GraphNode<T>&& node) {
    nodes_.push_back(std::forward<GraphNode<T>>(node));
  }

  template <typename... Args>
  void emplace_back(Args... args) {
    nodes_.emplace_back(std::forward<Args>(args)...);
  }

  auto& nodes() { return nodes_; }
  const auto& nodes() const { return nodes_; }

  auto& operator[](size_t index) {
    auto first = nodes_.begin();
    std::advance(first, index);
    return *first;
  }
  const auto& operator[](size_t index) const {
    auto first = nodes_.begin();
    std::advance(first, index);
    return *first;
  }

  template <typename U>
  friend std::ostream& operator<<(std::ostream& os, const Graph<U>& graph);

 private:
  std::list<GraphNode<T>> nodes_;
};

#define EXPECT(p, msg)                                                     \
  do {                                                                     \
    if (!(p)) {                                                            \
      auto msg_ = std::string(__FILE__) + ":" + std::to_string(__LINE__) + \
                  "\nerror:" + msg;                                        \
      throw std::runtime_error(msg_);                                      \
    }                                                                      \
  } while (0)

template <typename U, typename IterT>
std::vector<GraphNode<U>*> topologicSort(
    Graph<U>& graph,
    const std::function<Range<IterT> (GraphNode<U>*)>& getAdjacencyNodes) {
  enum VisitState {
    unvisit = 0,
    visiting,
    visited,
  };
  std::vector<GraphNode<U>*> result;
  std::vector<GraphNode<U>*> stack = {&graph.nodes().front()};
  std::unordered_map<GraphNode<U>*, VisitState> visitMap;
  while (!stack.empty()) {
    auto cur = stack.back();
    auto iter = visitMap.find(cur);
    if (iter == visitMap.end()) {
      visitMap[cur] = visiting;
      for (auto successor : getAdjacencyNodes(cur)) {
        auto iter = visitMap.find(successor);
        if (iter == visitMap.end()) {
          stack.push_back(successor);
        } else if (iter->second == visiting) {
          EXPECT(visitMap[successor] == unvisit, "find cycle!");
        } else {  // visited do nothing
        }
      }
    } else if (iter->second == visiting) {
      for (auto successor : getAdjacencyNodes(cur)) {
        EXPECT(visitMap[successor] == visited, "find cycle!");
      }
      visitMap[cur] = visited;
      stack.pop_back();
      result.push_back(cur);
    } else {
      EXPECT(false, "unreachable!");
    }
  }
  return result;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const GraphNode<T>& node) {
  os << "data:" << node.data_ << " predecessor:[";
  size_t index = 0;
  for (auto it : node.predecessor_) {
    os << it->data_;
    if (++index < node.predecessor_.size()) os << ",";
  }
  os << "] successor:[";
  index = 0;
  for (auto it : node.successor_) {
    os << it->data_;
    if (++index < node.successor_.size()) os << ",";
  }
  os << "]";
  return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Graph<T>& graph) {
  for (const auto& node : graph.nodes_) {
    os << node << "\n";
  }
  return os;
}

}  // namespace example

#endif