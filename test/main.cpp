#include <iostream>
#include <limits>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <regex>

#include "Graph.h"
#include "DominatorTree.h"

using namespace example;

template <typename T>
struct A {
  void static call() {
    std::cout << typeid(T).name() << std::endl;
  }
};

struct B {
  template <typename T>
  B(T&& t) : fptr(&A<T>::call) {}
  void (*fptr)(void);

  void operator()(){
    (*fptr)();
  }
};

int main(int argc, char** args) {

  Graph<char> graph;
  for(size_t i = 0; i < 13; ++i) {
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
  A<int> ai;
  B b(ai);
  b();

  // std::cout << graph << std::endl;
  return 0;
}
