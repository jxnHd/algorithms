#pragma GCC optimize("Ofast","inline","-ffast-math")
#pragma GCC target("avx,sse2,sse3,sse4,mmx")
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <limits>
#include <algorithm>
#include <set>
#include <queue>
#include <unordered_map>
#include <numeric>
#include <functional>

using namespace std;

struct Agent {
    int memory;
};

struct Team {
    vector<int> agents;
};

bool validatePartition(const vector<int>& partition, const vector<Agent>& agents, 
                      const vector<Team>& teams, int maxCapacity, int k);
int calculateCost(const vector<int>& serverAssignment, const Team& team, int k);
int calculateGain(const vector<int>& partition, const vector<Agent>& agents, 
                 const vector<Team>& teams, int agentIdx, int fromPart, int toPart);
vector<pair<int, int>> computeGains(const vector<int>& partition, 
                                   const vector<Agent>& agents,
                                   const vector<Team>& teams);
pair<int, vector<int>> FMPartition(const vector<Agent>& agents, 
                                  const vector<Team>& teams, int maxCapacity);
pair<int, vector<int>> kWayPartition(const vector<Agent>& agents, 
                                    const vector<Team>& teams, int maxCapacity, int k);
int findOptimalK(const vector<Agent>& agents, const vector<Team>& teams, 
                 int maxCapacity, vector<int>& bestPartition);
void outputResult(ofstream& outputFile, int cost, int k, const vector<int>& partition);
bool validateKWayPartition(const vector<int>& partition, const vector<Agent>& agents, 
                          const vector<Team>& teams, int maxCapacity, int k);

int calculateCost(const vector<int>& serverAssignment, const Team& team, int k) {
    vector<bool> serversUsed(k, false);
    for (size_t i = 0; i < team.agents.size(); ++i) {
        serversUsed[serverAssignment[team.agents[i]]] = true;
    }
    int serverCount = count(serversUsed.begin(), serversUsed.end(), true);
    int diff = serverCount - 1;
    return diff * diff;
}

int computeD(const vector<int>& partition, const vector<Agent>& agents, 
            const vector<Team>& teams, int vertex) {
    int internal = 0, external = 0;
    int currentPart = partition[vertex];
    
    for (const Team& team : teams) {
        if (find(team.agents.begin(), team.agents.end(), vertex) != team.agents.end()) {
            for (int other : team.agents) {
                if (other != vertex) {
                    if (partition[other] == currentPart) {
                        internal++;
                    } else {
                        external++;
                    }
                }
            }
        }
    }
    return external - internal;
}

vector<int> KLInitialPartition(const vector<Agent>& agents, const vector<Team>& teams, int maxCapacity) {
    int n = agents.size();
    vector<int> partition(n);
    vector<int> partitionSizes(2, 0);
    vector<int> partitionMemory(2, 0);
    
    for (int i = 0; i < n; i++) {
        int targetPart = (partitionMemory[0] <= partitionMemory[1]) ? 0 : 1;
        if (partitionMemory[targetPart] + agents[i].memory <= maxCapacity &&
            partitionSizes[targetPart] < maxCapacity) {
            partition[i] = targetPart;
            partitionSizes[targetPart]++;
            partitionMemory[targetPart] += agents[i].memory;
        } else {
            targetPart = 1 - targetPart;
            if (partitionMemory[targetPart] + agents[i].memory <= maxCapacity &&
                partitionSizes[targetPart] < maxCapacity) {
                partition[i] = targetPart;
                partitionSizes[targetPart]++;
                partitionMemory[targetPart] += agents[i].memory;
            }
        }
    }
    
    bool improved;
    int iterations = 0;
    do {
        improved = false;
        iterations++;
        cout << "iterations: " << iterations << endl;
        vector<bool> locked(n, false);
        vector<pair<int, int>> gains;  
        
        for (int i = 0; i < n; i++) {
            if (!locked[i]) {
                int d = computeD(partition, agents, teams, i);
                gains.push_back(make_pair(d, i));
            }
        }
        
        sort(gains.begin(), gains.end(), greater<pair<int, int>>());
        
        for (size_t i = 0; i < gains.size(); i++) {
            int a = gains[i].second;
            if (locked[a]) continue;
            
            int bestB = -1;
            int bestGain = 0;
            
            for (size_t j = i + 1; j < gains.size(); j++) {
                int b = gains[j].second;
                if (!locked[b] && partition[a] != partition[b]) {
                    int memDiffA = agents[b].memory - agents[a].memory;
                    int memDiffB = agents[a].memory - agents[b].memory;
                    
                    if (partitionMemory[partition[a]] + memDiffA <= maxCapacity &&
                        partitionMemory[partition[b]] + memDiffB <= maxCapacity) {
                        
                        int gain = gains[i].first + gains[j].first;
                        if (gain > bestGain) {
                            bestGain = gain;
                            bestB = b;
                        }
                    }
                }
            }
            
            if (bestB != -1 && bestGain > 0) {
                int partA = partition[a];
                int partB = partition[bestB];
                
                partition[a] = partB;
                partition[bestB] = partA;
                
                partitionMemory[partA] += (agents[bestB].memory - agents[a].memory);
                partitionMemory[partB] += (agents[a].memory - agents[bestB].memory);
                
                locked[a] = locked[bestB] = true;
                improved = true;
            }
        }
        
        if (iterations >= 100) break;
    } while (improved);
    
    return partition;
}

pair<int, vector<int>> FMPartition(const vector<Agent>& agents, const vector<Team>& teams, int maxCapacity) {
    vector<int> partition = KLInitialPartition(agents, teams, maxCapacity);
    vector<int> partitionSizes(2, 0);
    vector<int> partitionMemory(2, 0);
    
    for (size_t i = 0; i < partition.size(); i++) {
        partitionSizes[partition[i]]++;
        partitionMemory[partition[i]] += agents[i].memory;
    }
    
    bool improved;
    int totalCost = 0;
    for (const Team& team : teams) {
        totalCost += calculateCost(partition, team, 2);
    }
    
    vector<int> bestPartition = partition;
    int bestCost = totalCost;
    int maxIterations = agents.size();
    int iterations = 0;

    do {
        improved = false;
        vector<pair<int, int>> gains = computeGains(partition, agents, teams);
        
        sort(gains.begin(), gains.end(),
             [](const pair<int, int>& a, const pair<int, int>& b) { 
                 return a.first > b.first; 
             });
        
        for (const auto& p : gains) {
            int gain = p.first;
            int agentIdx = p.second;
            
            int fromPart = partition[agentIdx];
            int toPart = 1 - fromPart;
            
            if (partitionSizes[fromPart] > 1 && 
                partitionSizes[toPart] < maxCapacity && 
                partitionMemory[toPart] + agents[agentIdx].memory <= maxCapacity) {
                
                int newCost = totalCost - gain;
                
                if (newCost < totalCost) {
                    partitionMemory[fromPart] -= agents[agentIdx].memory;
                    partitionMemory[toPart] += agents[agentIdx].memory;
                    
                    partition[agentIdx] = toPart;
                    partitionSizes[fromPart]--;
                    partitionSizes[toPart]++;
                    totalCost = newCost;
                    improved = true;

                    if (totalCost < bestCost) {
                        bestCost = totalCost;
                        bestPartition = partition;
                    }
                    break;
                }
            }
        }
        iterations++;
        cout << "iterations for FM: " << iterations << endl;
        if(iterations >= 100) break;
    } while (improved && iterations < maxIterations);

    if (!validatePartition(bestPartition, agents, teams, maxCapacity, 2)) {
        return {-1, bestPartition};
    }

    return {bestCost, bestPartition};
}

int calculateGain(const vector<int>& partition, const vector<Agent>& agents, 
                 const vector<Team>& teams, int agentIdx, int fromPart, int toPart) {
    int gain = 0;
    
    for (const Team& team : teams) {
        if (find(team.agents.begin(), team.agents.end(), agentIdx) != team.agents.end()) {
            int oldCost = calculateCost(partition, team, 2);
            vector<int> newPartition = partition;
            newPartition[agentIdx] = toPart;
            int newCost = calculateCost(newPartition, team, 2);
            gain += (oldCost - newCost);
        }
    }
    
    return gain;
}

int calculateGain(const vector<int>& partition, const vector<Agent>& agents, 
                 const vector<Team>& teams, int agentIdx, int fromPart, int toPart);

vector<pair<int, int>> computeGains(const vector<int>& partition, 
                                   const vector<Agent>& agents,
                                   const vector<Team>& teams) {
    vector<pair<int, int>> gains;
    for (size_t i = 0; i < partition.size(); ++i) {
        int fromPart = partition[i];
        int toPart = 1 - fromPart;
        int gain = calculateGain(partition, agents, teams, i, fromPart, toPart);
        gains.push_back({gain, i});
    }
    return gains;
}

pair<int, vector<int>> kWayPartition(const vector<Agent>& agents, const vector<Team>& teams, int maxCapacity, int k) {
    vector<int> partition(agents.size(), -1);
    vector<int> partitionSizes(k, 0);
    vector<int> partitionMemory(k, 0);

    vector<pair<int, int>> teamsBySize;  
    for (int i = 0; i < teams.size(); i++) {
        teamsBySize.push_back({teams[i].agents.size(), i});
    }
    sort(teamsBySize.begin(), teamsBySize.end(), greater<pair<int, int>>());

    for (const auto& team_info : teamsBySize) {
        int size = team_info.first;
        int teamIdx = team_info.second;
        const Team& team = teams[teamIdx];
        
        bool alreadyAssigned = false;
        int assignedPart = -1;
        for (int agent : team.agents) {
            if (partition[agent] != -1) {
                alreadyAssigned = true;
                assignedPart = partition[agent];
                break;
            }
        }

        if (alreadyAssigned) {
            for (int agent : team.agents) {
                if (partition[agent] == -1) {
                    if (partitionMemory[assignedPart] + agents[agent].memory <= maxCapacity &&
                        partitionSizes[assignedPart] < maxCapacity) {
                        partition[agent] = assignedPart;
                        partitionSizes[assignedPart]++;
                        partitionMemory[assignedPart] += agents[agent].memory;
                    } else {
                        int minPart = 0;
                        for (int p = 1; p < k; p++) {
                            if (partitionSizes[p] < partitionSizes[minPart]) {
                                minPart = p;
                            }
                        }
                        if (partitionMemory[minPart] + agents[agent].memory <= maxCapacity &&
                            partitionSizes[minPart] < maxCapacity) {
                            partition[agent] = minPart;
                            partitionSizes[minPart]++;
                            partitionMemory[minPart] += agents[agent].memory;
                        }
                    }
                }
            }
        } else {
            int part1 = 0, part2 = 1;
            for (int p = 2; p < k; p++) {
                if (partitionSizes[p] < partitionSizes[part1]) {
                    part2 = part1;
                    part1 = p;
                } else if (partitionSizes[p] < partitionSizes[part2]) {
                    part2 = p;
                }
            }

            for (size_t i = 0; i < team.agents.size(); i++) {
                int agent = team.agents[i];
                int targetPart = (i < team.agents.size()/2) ? part1 : part2;
                
                if (partitionMemory[targetPart] + agents[agent].memory <= maxCapacity &&
                    partitionSizes[targetPart] < maxCapacity) {
                    partition[agent] = targetPart;
                    partitionSizes[targetPart]++;
                    partitionMemory[targetPart] += agents[agent].memory;
                }
            }
        }
    }

    for (int i = 0; i < agents.size(); i++) {
        if (partition[i] == -1) {
            int minPart = 0;
            for (int p = 1; p < k; p++) {
                if (partitionSizes[p] < partitionSizes[minPart]) {
                    minPart = p;
                }
            }
            
            if (partitionMemory[minPart] + agents[i].memory <= maxCapacity &&
                partitionSizes[minPart] < maxCapacity) {
                partition[i] = minPart;
                partitionSizes[minPart]++;
                partitionMemory[minPart] += agents[i].memory;
            } else {
                bool assigned = false;
                for (int p = 0; p < k; p++) {
                    if (partitionMemory[p] + agents[i].memory <= maxCapacity &&
                        partitionSizes[p] < maxCapacity) {
                        partition[i] = p;
                        partitionSizes[p]++;
                        partitionMemory[p] += agents[i].memory;
                        assigned = true;
                        break;
                    }
                }
                if (!assigned) {
                    return {-1, partition};
                }
            }
        }
    }

    bool improved;
    int iterations = 0;
    do {
        improved = false;
        iterations++;

        for (int i = 0; i < agents.size(); i++) {
            int currentPart = partition[i];
            
            int bestPart = -1;
            int minSize = maxCapacity;
            for (int p = 0; p < k; p++) {
                if (p != currentPart && partitionSizes[p] < minSize &&
                    partitionMemory[p] + agents[i].memory <= maxCapacity) {
                    minSize = partitionSizes[p];
                    bestPart = p;
                }
            }

            if (bestPart != -1) {
                partition[i] = bestPart;
                partitionSizes[bestPart]++;
                partitionSizes[currentPart]--;
                partitionMemory[bestPart] += agents[i].memory;
                partitionMemory[currentPart] -= agents[i].memory;
                improved = true;
            }
        }
        
        if (iterations >= 50) break;
    } while (improved);

    for (int p = 0; p < k; p++) {
        if (partitionSizes[p] == 0) {
            return {-1, partition};
        }
    }

    int totalCost = 0;
    for (const Team& team : teams) {
        totalCost += calculateCost(partition, team, k);
    }

    return {totalCost, partition};
}

int findOptimalK(const vector<Agent>& agents, const vector<Team>& teams, int maxCapacity, vector<int>& bestPartition) {
    int bestK = 2;
    int minCost = numeric_limits<int>::max();
    bool foundValidPartition = false;  
    
    for (int k = 2; k <= min(static_cast<int>(agents.size()), maxCapacity); ++k) {
        if (k > 2000) {
            break;
        }
        
        pair<int, vector<int>> result = kWayPartition(agents, teams, maxCapacity, k);
        int cost = result.first;
        vector<int> partition = result.second;
        
        if (cost != -1 && validateKWayPartition(partition, agents, teams, maxCapacity, k)) {
            if (cost < minCost) {
                minCost = cost;
                bestK = k;
                bestPartition = partition;
                foundValidPartition = true;  
            }
        }
    }
    
    return foundValidPartition ? bestK : -1;
}

bool validatePartition(const vector<int>& partition, const vector<Agent>& agents, 
                      const vector<Team>& teams, int maxCapacity, int k) {
    vector<int> partitionSizes(k, 0);
    vector<int> partitionMemory(k, 0);  

    for (size_t i = 0; i < partition.size(); i++) {
        int part = partition[i];
        if (part >= k || part < 0) {
            cout << "分區編號無效: " << part << endl;
            return false;
        }
        partitionSizes[part]++;
        partitionMemory[part] += agents[i].memory;  
    }

    for (int i = 0; i < k; i++) {
        if (partitionSizes[i] > maxCapacity) {
            cout << "分區 " << i << " 超過大小限制: " << partitionSizes[i] 
                 << " > " << maxCapacity << endl;
            return false;
        }
        if (partitionMemory[i] > maxCapacity) {  
            cout << "分區 " << i << " 超過memory限制: " << partitionMemory[i] 
                 << " > " << maxCapacity << endl;
            return false;
        }
        if (partitionSizes[i] == 0) {
            cout << "分區 " << i << " 是空的" << endl;
            return false;
        }
    }

    for (size_t i = 0; i < teams.size(); i++) {
        set<int> usedPartitions;
        for (int agent : teams[i].agents) {
            usedPartitions.insert(partition[agent]);
        }
        if (usedPartitions.size() > 2) {
            cout << "團隊 " << i << " 的成員分散在超過兩個分區中" << endl;
            return false;
        }
    }

    return true;
}

void outputResult(ofstream& outputFile, int cost, int k, const vector<int>& partition) {
    outputFile << cost << endl;
    outputFile << k << endl;
    for (int p : partition) {
        outputFile << p << endl;
    }
}

bool isValidPartition(const vector<int>& partition, const vector<Team>& teams) {
    for (const Team& team : teams) {
        set<int> teamPartitions;
        for (int agent : team.agents) {
            if (static_cast<size_t>(agent) >= partition.size()) {
                cerr << "Error: Agent index out of bounds" << endl;
                return false;
            }
            teamPartitions.insert(partition[agent]);
        }
        if (teamPartitions.size() > 2) {
            return false;
        }
    }
    return true;
}

bool validateKWayPartition(const vector<int>& partition, const vector<Agent>& agents, 
                          const vector<Team>& teams, int maxCapacity, int k) {
    vector<int> partitionSizes(k, 0);
    vector<int> partitionMemory(k, 0);

    for (size_t i = 0; i < partition.size(); i++) {
        int part = partition[i];
        if (part >= k || part < 0) {
            cout << "k-way分區: 分區編號無效: " << part << endl;
            return false;
        }
        partitionSizes[part]++;
        partitionMemory[part] += agents[i].memory;
    }

    for (int i = 0; i < k; i++) {
        if (partitionSizes[i] > maxCapacity) {
            cout << "k-way分區: 分區 " << i << " 超過大小限制: " << partitionSizes[i] 
                 << " > " << maxCapacity << endl;
            return false;
        }
        if (partitionMemory[i] > maxCapacity) {
            cout << "k-way分區: 分區 " << i << " 超過memory限制: " << partitionMemory[i] 
                 << " > " << maxCapacity << endl;
            return false;
        }
        if (partitionSizes[i] == 0) {
            cout << "k-way分區: 分區 " << i << " 是空的" << endl;
            return false;
        }
    }

    cout << "k-way分區驗證通過！分區資訊: " << endl;
    for (int i = 0; i < k; i++) {
        cout << "分區" << i << ": 大小=" << partitionSizes[i] 
             << ", Memory=" << partitionMemory[i] << endl;
    }

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./ap <input_file> <output_file>" << endl;
        return 1;
    }

    ifstream inputFile(argv[1]);
    ofstream outputFile(argv[2]);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        cerr << "Error opening file." << endl;
        return 1;
    }

    int maxCapacity;
    inputFile >> maxCapacity;

    string line;
    vector<Agent> agents;
    while (getline(inputFile, line) && line != ".agent");
    int n;
    inputFile >> n;
    agents.resize(n);
    for (int i = 0; i < n; ++i) {
        inputFile >> agents[i].memory;
    }

    while (getline(inputFile, line) && line != ".team");
    int m;
    inputFile >> m;
    vector<Team> teams(m);
    for (int i = 0; i < m; ++i) {
        int teamSize;
        inputFile >> teamSize;
        teams[i].agents.resize(teamSize);
        for (int j = 0; j < teamSize; ++j) {
            inputFile >> teams[i].agents[j];
        }
    }

    vector<int> partition2Way;
    int cost2Way = numeric_limits<int>::max();
    bool isValid2Way = false;
    
    if (n >= 2) {
        tie(cost2Way, partition2Way) = FMPartition(agents, teams, maxCapacity);
        isValid2Way = (cost2Way != -1) && validatePartition(partition2Way, agents, teams, maxCapacity, 2);
        cout << "2-way partition cost: " << cost2Way << (isValid2Way ? " (valid)" : " (invalid)") << endl;
    }

    if (isValid2Way) {
        bool memoryValid = true;
        vector<int> partitionMemory(2, 0);
        
        for (size_t i = 0; i < partition2Way.size(); i++) {
            partitionMemory[partition2Way[i]] += agents[i].memory;
            if (partitionMemory[partition2Way[i]] > maxCapacity) {
                memoryValid = false;
                break;
            }
        }
        
        if (!memoryValid) {
            cout << "2-way partition memory超出限制，嘗試k-way partition..." << endl;
            isValid2Way = false;
        }
    }

    if (!isValid2Way) {
        vector<int> bestPartition;
        int optimalK = findOptimalK(agents, teams, maxCapacity, bestPartition);
        
        if (optimalK == -1) {
            cout << "錯誤：無法找到有效的k-way分割方案" << endl;
            return 1;
        }
        
        bool memoryValid = true;
        vector<int> partitionMemory(optimalK, 0);
        
        for (size_t i = 0; i < bestPartition.size(); i++) {
            partitionMemory[bestPartition[i]] += agents[i].memory;
            if (partitionMemory[bestPartition[i]] > maxCapacity) {
                memoryValid = false;
                break;
            }
        }
        
        if (!memoryValid) {
            cout << "k-way partition memory超出限制" << endl;
            return 1;
        }
        
        int costKWay = 0;
        for (const Team& team : teams) {
            costKWay += calculateCost(bestPartition, team, optimalK);
        }
        
        outputResult(outputFile, costKWay, optimalK, bestPartition);
    } else {
        outputResult(outputFile, cost2Way, 2, partition2Way);
    }

    return 0;
}
