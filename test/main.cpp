
#include <algorithm>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <regex>
#include <sstream>
#include <thread>
#include <tuple>
#include <type_traits>
#include <vector>
#include <atomic>
#include <cmath>

#include <regex>

#include "DominatorTree.h"
#include "Graph.h"
#include "STLExt.h"
#include "Tensor.hpp"
#include "../utils/ProcessBarrier.h"

using namespace example;

int main(int argc, char **args) {

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
  addSuccessor('L', {'M', 'B'});
  addSuccessor('C', {'E', 'H'});
  addSuccessor('E', {'H', 'C'});
  addSuccessor('H', {'M'});

  buildDominatorTree(&graph[0]);
  std::cout << graph << std::endl;

  // int T[7];
  // computeLPSArray("ABCDABD", 7, T);

  return 0;
}
