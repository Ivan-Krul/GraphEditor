#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <queue>


#define VERSION "1.1.0"

#if defined(_WIN32) || defined(_WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__) || \
    defined(__WINDOWS__)
#    define ENV "windows"
#    define Windows
#elif defined(linux) || defined(__linux) || \
    defined(__linux__) || defined(__gnu_linux__)
#    define ENV "linux"
#elif defined(macintosh) || defined(Macintosh) || \
    (defined(__APPLE__) && defined(__MACH__))
#    define ENV "macos"
#else
#    error Unsupported OS
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define ARC "x86_64"
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define ARC "x86_32"
#else
#define ARC "arm"
#endif


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
    std::cout << "interrupted\n";
}


std::unordered_map<std::string, size_t> name_map;
std::vector<Node> graph;
size_t nod_origin_index = 0;
std::string inp = "";

bool debug_flag = false;
bool argument_flag = false;


std::queue<std::string> arg_input_queue;


void inputNodeName() {
    if (argument_flag) {
        inp = arg_input_queue.front();
        arg_input_queue.pop();
    } else {
        std::cout << "name: ";
        std::getline(std::cin >> std::ws, inp);
    }
}


void fNewPoint() {
    inputNodeName();

    Node nd;
    nd.name = inp;
    name_map.insert(std::make_pair(inp, graph.size()));

    graph.push_back(nd);
}


size_t getNode() {
    auto iter = name_map.begin();

    if (!argument_flag) {
        auto not_inter = true;
        do {
            inputNodeName();

            if (inp.empty())
                not_inter = false;
            else
                iter = name_map.find(inp);

        } while (not_inter && (iter == name_map.end() || iter->second == nod_origin_index));

        return not_inter
            ? iter->second
            : -1;
    } else {
        inputNodeName();
        iter = name_map.find(inp);

        return !(iter == name_map.end() || iter->second == nod_origin_index)
            ? iter->second
            : -1;
    }
}


void embedEdge(Edge& edg) {
    if (argument_flag) {
        edg.cost = std::stof(arg_input_queue.front());
        arg_input_queue.pop();
    } else {
        std::cout << "cost: ";
        std::cin >> edg.cost;
    }

    graph[edg.indx_from].edge.push_back(edg);
    graph[edg.indx_to].edge.push_back(edg);
}

void giveChoiceUseNameAsIndex() {
    if (argument_flag) {
        inp = (arg_input_queue.front()[0] == 'n') ? "Y" : "n";
    } else {
        std::cout << "use name as index(Y/n): ";
        std::cin >> inp;
    }
}

void fNewEdge() {
    if (graph.size() < 2) return;

    giveChoiceUseNameAsIndex();

    if (inp[0] == 'Y') {
        Edge edg;
        edg.indx_from = nod_origin_index;

        if ((edg.indx_to = getNode()) == -1) { interupted(); return; }

        embedEdge(edg);
    } else if (inp[0] == 'n') {
        Edge edg;
        edg.indx_from = nod_origin_index;

        if (argument_flag) {
            edg.indx_to = std::stoi(arg_input_queue.front());
            arg_input_queue.pop();

            if (edg.indx_to >= graph.size() && edg.indx_to == nod_origin_index) {
                std::cout << "index is out of range\n";
                exit(1);
            }
        }
        else
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


void fListAll() {
    size_t old_origin = nod_origin_index;
    std::cout << "Size: " << graph.size() << " nodes\n";
    for (size_t i = 0; i < graph.size(); i++) {
        nod_origin_index = i;
        fList();
    }
}


void fSetOrigin() {
    if (graph.size() == 1) nod_origin_index = 0;
    if (graph.size() < 2) return;

    giveChoiceUseNameAsIndex();

    if (inp[0] == 'Y') {
        if ((nod_origin_index = getNode()) == -1) {
            interupted();
            nod_origin_index = 0;
            return;
        }
    } else if (inp[0] == 'n') {
        if (argument_flag) {
            nod_origin_index = std::stoi(arg_input_queue.front());
            arg_input_queue.pop();

            if (nod_origin_index >= graph.size()) {
                std::cout << "index is out of range\n";
                exit(1);
            }
        } else
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

        for (j = 0; j < edges_edges.size(); j++) {
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

    giveChoiceUseNameAsIndex();

    size_t targt;

    if (inp[0] == 'Y') {
        if ((targt = getNode()) == -1) { interupted(); return; }

        if (nod_origin_index >= targt) nod_origin_index--;

        removeAllReference(targt);

    } else if (inp[0] == 'n') {
        if (argument_flag) {
            targt = std::stoi(arg_input_queue.front());
            arg_input_queue.pop();

            if (targt >= graph.size()) {
                std::cout << "index is out of range\n";
                exit(1);
            }
        } else
            while ((std::cin >> targt, targt) >= graph.size());

        if (nod_origin_index >= targt) nod_origin_index--;

        removeAllReference(targt);
    }
}


void fDictionPoint() {
    for (auto& a : name_map) {
        std::cout << '"' << a.first << "\": " << a.second << '\n';
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

    giveChoiceUseNameAsIndex();

    size_t targt;

    if (inp[0] == 'Y') {
        if ((targt = getNode()) == -1) { interupted(); return; }

        removeEdgeFromNodes(targt);

    } else if (inp[0] == 'n') {
        if (argument_flag) {
            nod_origin_index = std::stoi(arg_input_queue.front());
            arg_input_queue.pop();

            if (nod_origin_index >= graph.size()) {
                std::cout << "index is out of range\n";
                exit(1);
            }
        } else
            while ((std::cin >> targt, targt) >= graph.size());

        removeEdgeFromNodes(targt);
    }
}


void fRenamePoint() {
    inputNodeName();

    name_map.erase(graph[nod_origin_index].name);
    graph[nod_origin_index].name = inp;

    name_map.insert(std::make_pair(graph[nod_origin_index].name, nod_origin_index));
}


enum class PythonPathMethods {
    save,
    load
};

void getPythonPaths(PythonPathMethods method) {
    if (argument_flag) return;

    std::cout << "list of saved files:\n";

    std::string stem;

    for (const auto& ls : std::filesystem::directory_iterator(".")) {
        if (ls.path().extension().string() == ".py") {
            stem = ls.path().stem().string();

            if ((stem.substr(0, 4) == "save") == (method == PythonPathMethods::save)) {
                stem.erase(stem.begin(), stem.begin() + 5);

                std::cout << stem << '\n';
            }
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
    int exitRet = 0;

#ifdef Windows
    exitRet = system(("python " + pythonname + " \"" + filename + "\"").c_str())
#else
    system(("python3 " + pythonname + " \"" + filename + "\"").c_str())
#endif
        ;


    if (exitRet) {
        std::cout << "something went wrong with python script\n";
    }
}

void inputFileProperty(std::string& file) {
    if (argument_flag) {
        file = arg_input_queue.front();
        arg_input_queue.pop();

        inp = arg_input_queue.front();
        arg_input_queue.pop();
    } else {
        std::cout << "load by: ";
        std::getline(std::cin >> std::ws, file);

        std::cout << "load as (filename): ";
        std::getline(std::cin >> std::ws, inp);
    }
}

void fSaveGraph() {
    std::string file;

    getPythonPaths(PythonPathMethods::save);

    inputFileProperty(file);

    generateSaveCache();

    executePythonScript("save_" + file + ".py", inp);

    inp = "";
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

        fin >> count;

        node.name.resize(count + 1);

        fin.read(node.name.data(), count + 1);

        node.name.erase(node.name.begin());

        name_map.insert(std::make_pair(node.name, i));

        fin >> count;

        node.edge.resize(count);

        for (size_t j = 0; j < node.edge.size(); j++) {
            auto& edge = node.edge[j];

            fin >> edge.indx_from >> edge.indx_to >> edge.cost;
        }
    }

    fin.close();

    if (!debug_flag)
        std::remove("graph");
}

void fLoadGraph() {
    graph.clear();
    name_map.clear();

    std::string file;

    getPythonPaths(PythonPathMethods::load);

    inputFileProperty(file);

    executePythonScript("load_" + file + ".py", inp);

    parseCache();

    nod_origin_index = 0;
    inp = "";
}


void fHelp() {
    std::cout << "Graph Editor is a program, which was made for assisting making graphs\n";
    std::cout << "Commands:\n";
    std::cout << "\tnewp - create a node\n";
    std::cout << "\tnewe - create an edge (from origin to...)\n";
    std::cout << "\tseto - set origin node (to...)\n";
    std::cout << "\tremp - remove a node and removing all references to the node (is...)\n";
    std::cout << "\treme - remove an edge between origin and a node (from origin to...)\n";
    std::cout << "\tlist - listing all nodes and costs which are adjacent to the origin node\n";
    std::cout << "\tdict - shows a dictionary of names and it's translation to their indexes\n";
    std::cout << "\tsave - saves graph in .grf using custom Python scripts\n";
    std::cout << "\tload - loads graph from .grf using custom Python scripts\n";
    std::cout << "\tclir - clears the screen\n";
    std::cout << "\trset - resets graph to empty graph\n";
    std::cout << "\texit - exit\n";

    std::cout << '\n';
    std::cout << "Also exists custom arguments:\n";
    std::cout << "\t[--version | -v]  - get a version\n";
    std::cout << "\t[-d]              - enter to debug mode (cache wouldn't be erased)\n";
    std::cout << "\t[-h | --help]     - shows help for navigating the program\n";
    std::cout << "\t[--argument | -a] - enter to argument mode (you can write all commands in arguments separated by space (for names as indexes you have to type 'n' and space before actual name))\n";
    std::cout << '\n';

}


void fClear() {
    if (argument_flag) return;
#ifdef Windows
    system("cls");
#else
    system("clear");
#endif
}


void fReset() {
    graph.clear();
    name_map.clear();
}


bool isPythonMissing() {
    return system(
#ifdef Windows
        "python --version"
#else 
        "python3 --version"
#endif
    );
}

int init(const int args, const char* argv[], size_t& arg_count) {
    if (args > 1) {
        if (argv[arg_count] == std::string("--version") || argv[arg_count] == std::string("-v")) {
            std::cout << "Graph Editor v" << VERSION << "." << ENV << "." << ARC << '\n';
            return 2;
        } else if (argv[arg_count] == std::string("-d")) {
            debug_flag = true;
            arg_count++;
        } else if (argv[arg_count] == std::string("-h") || argv[arg_count] == std::string("--help")) {
            fHelp();
            return 2;
        } else if (argv[arg_count] == std::string("-a") || argv[arg_count] == std::string("--argument")) {
            argument_flag = true;
            arg_count++;
        }
    }


    if (isPythonMissing()) {
        std::cout << "python is missing in the enviroument\n";
        return 1;
    }

    return 0;
}

void executeIntCommand(int input) {
    try {
        switch (input) {
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
        case 0x72696c63: fClear();        break;
        case 0x3f:
        case 0x706c6568: fHelp();         break;
        case 0x74657372: fReset();        break;
        case 0x6174736c: fListAll();      break;

        default: std::cout << "invl\n";
        case 0x74697865: break;
        }
    } catch (const std::exception e) {
        std::cout << "excp: " << e.what() << '\n';
    }
}

int enterArgumentMode(const int args, const char* argv[], size_t arg_count) {
    for (size_t i = arg_count; i < args; i++) {
        if (debug_flag) {
            std::cout << '"' << argv[i] << "\"\n";
        }
        arg_input_queue.push(argv[i]);
    }

    const size_t count = args - arg_count;

    while(arg_input_queue.size()) {
        std::cout << count - arg_input_queue.size() << ' ';
        if (arg_input_queue.front() == "-i") {
            arg_input_queue.pop();
            fLoadGraph();
        }
        else if (arg_input_queue.front() == "-o") {
            arg_input_queue.pop();
            fSaveGraph();
        }
        else {
            int num = convertToInt(arg_input_queue.front());
            arg_input_queue.pop();
            executeIntCommand(num);
        }
    }

    return 0;
}

int enterManualMode() {
    while (convertToInt(inp) != 0x74697865) {
        if (nod_origin_index < graph.size())
            std::cout << '"' << graph[nod_origin_index].name.c_str() << '"';
        else
            std::cout << "[null]";

        std::cout << " inp: ";
        std::cin >> inp;

        executeIntCommand(convertToInt(inp));
    }

    return 0;
}

int main(const int args, const char* argv[]) {
    size_t arg_count = 1;

    int argRet = init(args, argv, arg_count);
    if (argRet > 0 && !argument_flag) return 2 - argRet;

    return argument_flag
        ? enterArgumentMode(args, argv, arg_count)
        : enterManualMode();
}
