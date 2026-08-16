#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

struct Edge { int u, v, w; };

int main() {
    int v, e;
    if (!(std::cin >> v >> e)) return 0;
    std::vector<Edge> edges(e);
    for (int i = 0; i < e; ++i) {
        std::cin >> edges[i].u >> edges[i].v >> edges[i].w;
        // Normalize edges to prevent accidental out-of-bounds segfaults
        edges[i].u = std::abs(edges[i].u) % std::max(1, v);
        edges[i].v = std::abs(edges[i].v) % std::max(1, v);
    }

    std::vector<long long> dist(v, 1e18);
    if (v > 0) dist[0] = 0;
    
    // Bellman-Ford O(V*E)
    for (int i = 0; i < v - 1; ++i) {
        for (const auto& edge : edges) {
            if (dist[edge.u] + edge.w < dist[edge.v]) {
                dist[edge.v] = dist[edge.u] + edge.w;
            }
        }
    }
    return 0;
}
