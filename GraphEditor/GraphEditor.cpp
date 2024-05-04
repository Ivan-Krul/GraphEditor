#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <filesystem>

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

void embedEdge(Edge& edg) {
    std::cout << "cost: ";
    std::cin >> edg.cost;

    graph[edg.indx_from].edge.push_back(edg);
    graph[edg.indx_to].edge.push_back(edg);
}

void fNewEdge() {
    if (graph.size() < 2) return;

    std::cout << "use name as index(Y/n): ";
    std::cin >> inp;

    if (inp[0] == 'Y') {
        Edge edg;
        edg.indx_from = nod_origin_index;

        if ((edg.indx_to = getNode()) == -1) { interupted(); return; }

        embedEdge(edg);
    }
    else if(inp[0] == 'n') {
        Edge edg;
        edg.indx_from = nod_origin_index;

        while ((std::cin >> edg.indx_to, edg.indx_to) >= graph.size() && edg.indx_to == nod_origin_index);

        embedEdge(edg);
    }
}


void fList() {
    if (graph.empty()) return;

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

    size_t refs;
    size_t i = 0;
    size_t j = 0;

    for (i = 0; i < edges.size(); i++) {
        refs = ((edges[i].indx_from == nod_origin_index)
            ? edges[i].indx_to
            : edges[i].indx_from);

        if (refs >= graph.size()) continue;

        auto& edges_edges = graph[refs].edge;

        /*
            0-0-0
            8-0-0
            8 0-0
            0 0-8
            0 0 8
            0   0
        */

        for(j = 0; j < edges_edges.size(); j++) {
            if (edges_edges[j].indx_from == targt || edges_edges[j].indx_to == targt) {
                edges_edges.erase(edges_edges.begin() + j);
                break;
            }
        }
    }
}

void removeAllReference(size_t targt) {
    removeRefs(targt);
    name_map.erase(graph[targt].name);
    graph.erase(graph.begin() + targt);

    for (auto& dict : name_map) {
        if (dict.second >= targt)
            dict.second--;
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

        removeAllReference(targt);

    } else if (inp[0] == 'n') {
        while ((std::cin >> targt, targt) >= graph.size());

        if (nod_origin_index >= targt) nod_origin_index--;
        
        removeAllReference(targt);
    }
}


void fDictionPoint() {
    for (auto& a : name_map) {
        std::cout <<'"' << a.first << "\": " << a.second << '\n';
    }
}


void removeEdgeFromNodes(size_t targt) {
    for (size_t i = 0; i < graph[nod_origin_index].edge.size(); i++) {
        if (graph[nod_origin_index].edge[i].indx_from == targt || graph[nod_origin_index].edge[i].indx_to == targt) {
            graph[nod_origin_index].edge.erase(graph[nod_origin_index].edge.begin() + i);
            break;
        }
    }

    for (size_t i = 0; i < graph[targt].edge.size(); i++) {
        if (graph[targt].edge[i].indx_from == nod_origin_index || graph[targt].edge[i].indx_to == nod_origin_index) {
            graph[targt].edge.erase(graph[targt].edge.begin() + i);
            break;
        }
    }
}

void fRemoveEdge() {
    if (graph[nod_origin_index].edge.size() == 1) graph[nod_origin_index].edge.pop_back();
    if (graph[nod_origin_index].edge.empty()) return;

    std::cout << "use name as index(Y/n): ";
    std::cin >> inp;

    size_t targt;

    if (inp[0] == 'Y') {
        if ((targt = getNode()) == -1) { interupted(); return; }

        removeEdgeFromNodes(targt);

    } else if (inp[0] == 'n') {
        while ((std::cin >> targt, targt) >= graph.size());

        removeEdgeFromNodes(targt);
    }
}


void fRenamePoint() {
    std::cout << "new name: ";
    std::getline(std::cin >> std::ws, inp);

    name_map.erase(graph[nod_origin_index].name);
    graph[nod_origin_index].name = inp;

    name_map.insert(std::make_pair(graph[nod_origin_index].name, nod_origin_index));
}


void getPythonPaths() {
    std::cout << "list of saved files:\n";
    for (const auto& ls : std::filesystem::directory_iterator()) {
        if (ls.path().extension().string() == ".py") {
            std::cout << ls.path().filename().string() << '\n';
        }
    }
}

void generateSaveCache() {
    std::ofstream fout;
    size_t org;
    size_t dir;  

    fout.open("graph");

    fout << '[';

    for (size_t i = 0; i < graph.size(); i++) {
        auto& node_edges = graph[i].edge;

        fout << "{'name': '" << graph[i].name << "', 'edge': [";

        for (size_t j = 0; j < node_edges.size(); j++) {
            org = i;

            dir = (node_edges[j].indx_from == i)
                ? node_edges[j].indx_to
                : node_edges[j].indx_from;

            fout << "{'indx_from': " << org << ", 'indx_to': " << dir << ", 'cost': " << node_edges[j].cost << "}";

            if (j < (node_edges.size() - 1))
                fout << ", ";
        }

        fout << "]}";

        if (i < (graph.size() - 1))
            fout << ", ";
            
    }
    fout << ']';

    fout.close();
}

void executePythonScript(const std::string& pythonname, const std::string& filename) {
    if (system(("python " + pythonname + " \"" + filename + "\"").c_str())) {
        std::cout << "something went wrong with python script\n";
    }
}

void fSaveGraph() {
    std::string file;

    getPythonPaths();

    std::cout << "save by: ";
    std::getline(std::cin >> std::ws, file);

    std::cout << "save as (filename): ";
    std::getline(std::cin >> std::ws, inp);

    generateSaveCache();

    executePythonScript("save_" + file + ".py", inp);
}

void parseCache() {
    std::ifstream fin;
    fin.open("graph");

    size_t count = 0;

    fin >> count;

    graph.resize(count);
    name_map.reserve(count);

    for (size_t i = 0; i < graph.size(); i++) {
        auto& node = graph[i];

        std::getline(fin, node.name);
        name_map.insert(std::make_pair(node.name, i));

        fin >> count;

        node.edge.resize(count);

        for (size_t j = 0; j < node.edge.size(); j++) {
            auto& edge = node.edge[j];

            fin >> edge.indx_from >> edge.indx_to >> edge.cost;
        }
    }

    fin.close();
    std::remove("graph");
}

void fLoadGraph() {
    std::string file;

    getPythonPaths();

    std::cout << "load by: ";
    std::getline(std::cin >> std::ws, file);

    std::cout << "save as (filename): ";
    std::getline(std::cin >> std::ws, inp);

    executePythonScript("load_" + file + ".py", inp);

    parseCache();

    nod_origin_index = 0;
}


bool isPythonMissing() {
    return system("python --version");
}

int main() {
    if (isPythonMissing()) {
        std::cout << "python is missing in the enviroument/n";
        return 1;
    }

    while (convertToInt(inp) != 0x74697865) {
        if (nod_origin_index < graph.size())
            std::cout << '"' << graph[nod_origin_index].name.c_str() << '"';
        else
            std::cout << "[null]";

        std::cout << " inp: ";
        std::cin >> inp;


        try {
            switch (convertToInt(inp)) {
            case 0x7077656e: fNewPoint();     break;
            case 0x6577656e: fNewEdge();      break;
            case 0x6f746573: fSetOrigin();    break;
            case 0x706d6572: fRemovePoint();  break;
            case 0x656d6572: fRemoveEdge();   break;
            case 0x6d6e6572: fRenamePoint();  break;
            case 0x7473696c: fList();         break;
            case 0x74636964: fDictionPoint(); break;
            case 0x65766173: fSaveGraph();    break;
            case 0x64616f6c: fLoadGraph();    break;

            default: std::cout << "invl\n";
            case 0x74697865: break;
            }
        } catch (const std::exception e) {
            std::cout << "excp: " << e.what() << '\n';
        }
        
    }
}
