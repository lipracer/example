
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <regex>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <vector>

#include "Graph.h"
#include "STLExt.h"
#include "Tensor.hpp"
#include "Test.h"

using namespace example;

TEST(graph, cycle) {
  Graph<char> graph;
  for (size_t i = 0; i < 13; ++i) {
    graph.emplace_back('A' + i);
  }

  auto addSuccessor = [&](char lhs, std::initializer_list<char> rhs) {
    for (auto it : rhs) {
      graph[lhs - 'A'].addSuccessor(&graph[it - 'A']);
    }
  };

  addSuccessor('A', {'B', 'C'});
  addSuccessor('B', {'D', 'G'});
  addSuccessor('D', {'F', 'G'});
  addSuccessor('F', {'I', 'K'});
  addSuccessor('G', {'J'});
  addSuccessor('J', {'I'});
  addSuccessor('I', {'L'});
  addSuccessor('K', {'L'});
  addSuccessor('L', {'M'});
  addSuccessor('C', {'E', 'H'});
  addSuccessor('E', {'H'});
  addSuccessor('H', {'M'});

  std::function<decltype(makeRange(
      std::declval<GraphNode<char>>().successor()))(GraphNode<char>*)>
      getNodes = [](GraphNode<char>* node)
      -> decltype(makeRange(std::declval<GraphNode<char>>().successor())) {
    return makeRange(node->successor());
  };

  auto result = topologicSort(graph, getNodes);
  for (auto node : result) {
    std::cout << *node << std::endl;
  }
  auto first = std::begin(result);
  for (; first != std::end(result); ++first) {
    for (auto adjacency : getNodes(*first)) {
      EXPECT(first != std::find(std::begin(result), first, adjacency), "");
    }
  }
}
