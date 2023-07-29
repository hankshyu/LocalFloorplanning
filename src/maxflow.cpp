#include "maxflow.h"

MaxFlow::MaxFlow(/* args */) {
    nodesCount = 0;
    edgesCount = 0;
    sourceCount = 0;
    sinkCount = 0;
}


MaxFlow::~MaxFlow() {}


void MaxFlow::addNode(std::string nodeName, NodeType node_type) {
    int curNodeID = nodesCount++;
    name2id[nodeName] = curNodeID;

    if ( node_type == SOURCE ) {
        sourceCount++;
        sources.push_back(curNodeID);
    }
    else if ( node_type == SINK ) {
        sinkCount++;
        sinks.push_back(curNodeID);
    }
}


void MaxFlow::addEdge(int from, int to, int capacity) {
    edgesCount++;
    capacities[from][to] = capacity;
    graph[from].push_back(to);
    graph[to].push_back(from);
}


void MaxFlow::addEdge(std::string from, std::string to, int capacity) {
    int fromID = name2id[from];
    int toID = name2id[to];
    this->addEdge(fromID, toID, capacity);
}


int MaxFlow::bfs(int startNode, int endNode) {
    memset(parentsList, -1, sizeof(parentsList));
    memset(currentPathCapacity, 0, sizeof(currentPathCapacity));

    std::queue<int> q;
    q.push(startNode);

    parentsList[startNode] = -2;
    currentPathCapacity[startNode] = INT_MAX;

    while ( !q.empty() ) {
        int currentNode = q.front();
        q.pop();

        for ( int i = 0; i < graph[currentNode].size(); i++ ) {
            int to = graph[currentNode][i];
            if ( parentsList[to] == -1 ) {
                if ( capacities[currentNode][to] - flowPassed[currentNode][to] > 0 ) {
                    parentsList[to] = currentNode;
                    currentPathCapacity[to] = std::min(currentPathCapacity[currentNode],
                        capacities[currentNode][to] - flowPassed[currentNode][to]);
                    if ( to == endNode ) {
                        return currentPathCapacity[endNode];
                    }
                    q.push(to);
                }
            }
        }
    }
    return 0;
}


int MaxFlow::EdmondsKarp(int startNode, int endNode) {
    int maxFlow = 0;
    while ( true ) {
        int flow = bfs(startNode, endNode);
        if ( flow == 0 ) {
            break;
        }
        maxFlow += flow;
        int currentNode = endNode;
        while ( currentNode != startNode ) {
            int previousNode = parentsList[currentNode];
            flowPassed[previousNode][currentNode] += flow;
            flowPassed[currentNode][previousNode] -= flow;
            currentNode = previousNode;
        }
    }
    return maxFlow;
}


void MaxFlow::solve() {
    superSource = nodesCount;
    superSink = nodesCount + 1;
    nodesCount += 2;

    for ( int s = 0; s < sourceCount; s++ ) {
        int curSource = sources[s];
        capacities[superSource][curSource] = INT_MAX;
        graph[superSource].push_back(curSource);
        graph[curSource].push_back(superSource);
    }

    for ( int s = 0; s < sinkCount; s++ ) {
        int curSink = sinks[s];
        capacities[curSink][superSink] = INT_MAX;
        graph[superSink].push_back(curSink);
        graph[curSink].push_back(superSink);
    }

    int maxFlow = EdmondsKarp(superSource, superSink);

    std::cout << "MaxFlow Solved. Flow = " << maxFlow << std::endl;
}


int MaxFlow::edgeFlow(int from, int to) {
    return flowPassed[from][to];
}

int MaxFlow::edgeFlow(std::string from, std::string to) {
    int fromID = name2id[from];
    int toID = name2id[to];
    return this->edgeFlow(fromID, toID);
}