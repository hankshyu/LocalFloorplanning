#ifndef MAXFLOW_H
#define MAXFLOW_H

#include <cstdio>
#include <queue>
#include <cstring>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>

const int MAXNODE = 1000;
const int MAXFLOW_MAX_INT = 2147483647;

enum NodeType { NODE, SOURCE, SINK };

class MaxFlow {
private:
    int capacities[MAXNODE][MAXNODE];
    int flowPassed[MAXNODE][MAXNODE];
    std::vector<int> graph[MAXNODE];
    int parentsList[MAXNODE];
    int currentPathCapacity[MAXNODE];
    int nodesCount, edgesCount;
    int sourceCount, sinkCount;
    int superSource, superSink;
    std::vector<int> sources, sinks;
    std::unordered_map<std::string, int> name2id;

    int edgeFlow(int from, int to);
    void addEdge(int from, int to, int capacity);
    int bfs(int startNode, int endNode);
    int EdmondsKarp(int startNode, int endNode);

public:
    MaxFlow(/* args */);
    ~MaxFlow();
    void addNode(std::string nodeName, NodeType node_type);
    void addEdge(std::string from, std::string to, int capacity);
    void solve();
    int edgeFlow(std::string from, std::string to);
};

#endif  // MAXFLOW_H
