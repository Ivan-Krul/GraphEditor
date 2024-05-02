#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

struct Node;

struct Edge {
    size_t indx_to;
    size_t indx_from;
    float cost;
};

struct Node {
    std::string name;
    std::vector<Edge> edge;
};


inline int convertToInt(const std::string& str) {
    return *(reinterpret_cast<const int*>(str.c_str()));
}

inline void interupted() {
    std::cout << "intr\n";
}


std::unordered_map<std::string, size_t> name_map;
std::vector<Node> graph;
size_t nod_origin_index = 0;
std::string inp = "";


void fNewPoint() {
    std::cout << "name: ";
    std::getline(std::cin >> std::ws, inp);

    Node nd;
    nd.name = inp;
    name_map.insert(std::make_pair(inp, graph.size()));

    graph.push_back(nd);
}


size_t getNode() {
    auto iter = name_map.begin();
    auto not_inter = true;

    do {

        std::cout << "name: ";
        std::getline(std::cin >> std::ws, inp);

        if (inp.empty())
            not_inter = false;
        else
            iter = name_map.find(inp);

        /*
            a b c    x
            0 0 0 -> 0 c
            1 0 0 -> 0 c
            0 1 0 -> 0 c
            1 1 0 -> 0 c
            0 0 1 -> 0
            1 0 1 -> 1 (a | b)
            0 1 1 -> 1 (a | b)
            1 1 1 -> 1 (a | b)

            c & (a | b)

        */

    } while (not_inter && (iter == name_map.end() || iter->second == nod_origin_index));
    
    return not_inter
        ? iter->second
        : -1;
}

void fNewEdge() {
    if (graph.size() < 2) return;

    std::cout << "use name as index(Y/n): ";
    std::cin >> inp;

    if (inp[0] == 'Y') {
        Edge edg;
        edg.indx_from = nod_origin_index;

        if ((edg.indx_to = getNode()) == -1) { interupted(); return; }

        std::cout << "cost: ";
        std::cin >> edg.cost;

        std::cout << edg.indx_from << " " << edg.indx_to;

        graph[edg.indx_from].edge.push_back(edg);
        graph[edg.indx_to  ].edge.push_back(edg);
    }
    else if(inp[0] == 'n') {
        Edge edg;
        edg.indx_from = nod_origin_index;

        while ((std::cin >> edg.indx_to, edg.indx_to) >= graph.size() && edg.indx_to == nod_origin_index);

        std::cout << "cost: ";
        std::cin >> edg.cost;

        graph[edg.indx_from].edge.push_back(edg);
        graph[edg.indx_to  ].edge.push_back(edg);
    }
}


void fList() {
    auto& cur = graph[nod_origin_index];

    size_t nod_to;

    std::cout << cur.name << ":\n";
    for (size_t i = 0; i < cur.edge.size(); i++) {
        nod_to = ((cur.edge[i].indx_from == nod_origin_index)
            ? cur.edge[i].indx_to
            : cur.edge[i].indx_from);

        std::cout << '\t';

        if (nod_to >= graph.size()) std::cout << "invl " << nod_to;
        else std::cout << graph[nod_to].name;

        std::cout << " -> " << cur.edge[i].cost << '\n';
    }

}


void fSetOrigin() {
    if (graph.size() == 1) nod_origin_index = 0;
    if (graph.size() < 2) return;

    std::cout << "use name as index(Y/n): ";
    std::cin >> inp;

    if (inp[0] == 'Y') {
        if ((nod_origin_index = getNode()) == -1) {
            interupted();
            nod_origin_index = 0;
            return;
        }
    } else if (inp[0] == 'n') {
        while ((std::cin >> nod_origin_index, nod_origin_index) >= graph.size());
    }
}


void removeRefs(size_t targt) {
    auto& edges = graph[targt].edge;
    auto& edges_edges = graph[targt].edge;

    size_t refs;
    size_t i = 0;
    size_t j = 0;

    for (i = 0; i < edges.size(); i++) {
        refs = ((edges[i].indx_from == nod_origin_index)
            ? edges[i].indx_to
            : edges[i].indx_from);

        if (refs >= graph.size()) continue;

        edges_edges = graph[refs].edge;

        for(j = 0; j < edges_edges.size(); j++) {
            if (edges_edges[j].indx_from == targt || edges_edges[j].indx_to == targt) {
                edges_edges.erase(edges_edges.begin() + j);
                break;
            }
        }
    }
}

void fRemovePoint() {
    if (graph.size() == 1) graph.pop_back();
    if (graph.empty()) return;

    std::cout << "use name as index(Y/n): ";
    std::cin >> inp;

    size_t targt;

    if (inp[0] == 'Y') {
        if ((targt = getNode()) == -1) { interupted(); return; }

        if (nod_origin_index >= targt) nod_origin_index--;

        removeRefs(targt);
        name_map.erase(graph[targt].name);
        graph.erase(graph.begin() + targt);

    } else if (inp[0] == 'n') {
        while ((std::cin >> targt, targt) >= graph.size());

        if (nod_origin_index >= targt) nod_origin_index--;
        
        removeRefs(targt);
        name_map.erase(graph[targt].name);
        graph.erase(graph.begin() + targt);
        
        
    }
}


int main() {
    while (convertToInt(inp) != 0x74697865) {
        if (nod_origin_index < graph.size())
            std::cout << '"' << graph[nod_origin_index].name.c_str() << '"';
        else
            std::cout << "[null]";

        std::cout << " inp: ";
        std::cin >> inp;


        try {
            switch (convertToInt(inp)) {
            case 0x7077656e: fNewPoint();    break;
            case 0x6577656e: fNewEdge();     break;
            case 0x6f746573: fSetOrigin();   break;
            case 0x656d6572:                 break; // reme
            case 0x706d6572: fRemovePoint(); break; // remp
            case 0x6d6e6572:                 break; // renm
            case 0x7473696c: fList();        break;

            default: std::cout << "invl\n";
            case 0x74697865: break;
            }
        } catch (const std::exception e) {
            std::cout << "excp: " << e.what() << '\n';
        }
        
    }
}
