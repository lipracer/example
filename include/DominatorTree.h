#ifndef INCLUDE_DOMINATORTREE_H
#define INCLUDE_DOMINATORTREE_H

#include <unordered_map>
#include <unordered_set>
#include <algorithm>

#include "Graph.h"

namespace example {

struct DominatorTree {};

template <typename T>
auto buildDfsNumMap(const GraphNode<T>* root) {
  size_t num = 0;
  std::unordered_map<const GraphNode<T>*, size_t> result;
  std::vector<const GraphNode<T>*> stack{root};
  while (!stack.empty()) {
    auto current = stack.back();
    stack.pop_back();
    if (result.find(current) != result.end()) {
      continue;
    }
    result.insert(std::make_pair(current, num++));
    stack.insert(stack.end(), current->successor().rbegin(),
                 current->successor().rend());
  }
  return result;
}

template <typename T>
DominatorTree buildDominatorTree(const GraphNode<T>* root) {
  auto dfsnumMap = buildDfsNumMap(root);

  std::unordered_map<const GraphNode<T>*, const GraphNode<T>*> ancestorMap;
  std::unordered_map<const GraphNode<T>*, const GraphNode<T>*> semiMap;

  for (auto it : dfsnumMap) {
    ancestorMap.insert(std::make_pair(it.first, nullptr));
  }

  for (auto it : dfsnumMap) {
    semiMap.insert(std::make_pair(it.first, nullptr));
  }

  auto parent = [&](const GraphNode<T>* n) -> const GraphNode<T>* {
    std::vector<const GraphNode<T>*> ancestors;
    ancestors.reserve(n->predecessor().size());
    for (auto v : n->predecessor()) {
      if (dfsnumMap[v] < dfsnumMap[n]) {
        ancestors.push_back(v);
      }
    }
    if (ancestors.empty()) return nullptr;
    return *std::max_element(
        ancestors.begin(), ancestors.end(),
        [&](auto* lhs, auto* rhs) { return dfsnumMap[lhs] < dfsnumMap[rhs]; });
  };

  auto ancestorWithLowestSemi =
      [&](const GraphNode<T>* v) -> const GraphNode<T>* {
    auto u = v;
    while (ancestorMap[v]) {
      if (dfsnumMap[semiMap[v]] < dfsnumMap[semiMap[u]]) {
        u = v;
      }
      v = ancestorMap[v];
    }
    return u;
  };

  auto link = [&](const GraphNode<T>* parent, const GraphNode<T>* node) {
    ancestorMap[node] = parent;
  };

  std::vector<const GraphNode<T>*> sortedByDfsnum(dfsnumMap.size(), nullptr);
  for (auto it : dfsnumMap) {
    sortedByDfsnum[it.second] = it.first;
  }
  std::reverse(sortedByDfsnum.begin(), sortedByDfsnum.end());

  std::unordered_map<const GraphNode<T>*, std::vector<const GraphNode<T>*>>
      bucket;

  std::unordered_map<const GraphNode<T>*, const GraphNode<T>*> idomMap;
  std::unordered_map<const GraphNode<T>*, const GraphNode<T>*> sameDom;

  for (auto n : sortedByDfsnum) {
    const GraphNode<T>* semi = nullptr;
    for (auto v : n->predecessor()) {
      const GraphNode<T>* tmp = nullptr;
      if (dfsnumMap[v] <= dfsnumMap[n]) {
        tmp = v;
      } else {
        tmp = semiMap[ancestorWithLowestSemi(v)];
      }
      if (!semi) {
        semi = tmp;
      } else if (dfsnumMap[tmp] < dfsnumMap[semi]) {
        semi = tmp;
      }
    }
    semiMap[n] = semi;
    auto p = parent(n);
    link(p, n);
    bucket[semi].push_back(n);
    for (auto v : bucket[p]) {
      auto y = ancestorWithLowestSemi(v);
      if (semiMap[v] == semiMap[y]) {
        idomMap[v] = p;
      } else {
        sameDom[v] = y;
      }
    }
    bucket[p].clear();
  }

  for (auto it : sortedByDfsnum) {
    if (sameDom[it]) {
      idomMap[it] = idomMap[sameDom[it]];
    }
  }

  for (auto it : idomMap) {
    std::cout << *it.first << "\n idom node:";
    if (it.second) {
      std::cout << *it.second;
    } else {
      std::cout << "nullptr";
    }
    std::cout << std::endl;
  }
  return {};
}

}  // namespace example

#endif