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

struct Function {
    std::queue<std::string> input_queue;
    bool require_cache = false;
};


inline int convertToInt(const std::string& str) {
    int orig = *(reinterpret_cast<const int*>(str.c_str()));
    return ((orig & 0xff000000) >> 24) | ((orig & 0xff0000) >> 8) | ((orig & 0xff00) << 8) | ((orig & 0xff) << 24);
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

std::unordered_map<std::string, Function> functions;
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
        Edge edg = { 0 };
        edg.indx_from = nod_origin_index;

        if ((edg.indx_to = getNode()) == -1) { interupted(); return; }

        embedEdge(edg);
    } else if (inp[0] == 'n') {
        Edge edg = { 0 };
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
    using pair_name_map = std::pair<std::string, size_t>;

    std::vector<pair_name_map> vec_name_map;
    vec_name_map.reserve(name_map.size());

    for (auto& elem : name_map) {
        vec_name_map.push_back(elem);
    }
    std::sort(vec_name_map.begin(), vec_name_map.end(), [=](pair_name_map& a, pair_name_map& b) {return a.second < b.second; });

    for (auto& a : vec_name_map) {
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

    size_t targt = 0;

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

    for (size_t n = 0; n < graph.size(); n++) {
        auto& node = graph[n];

        std::getline(fin, node.name);
        std::getline(fin, node.name);
        name_map.insert({ node.name, n });

        fin >> count;

        node.edge.resize(count);

        for (size_t e = 0; e < node.edge.size(); e++) {
            auto& edge = node.edge[e];
            fin >> edge.indx_from >> edge.indx_to >> edge.cost;
        }
    }

    fin.close();

#ifndef _DEBUG
    if (!debug_flag)
        std::remove("graph");
#endif
}

void generateTempCache() {
    std::ofstream fout;
    fout.open("graph");
    
    fout << graph.size();
    for (size_t n = 0; n < graph.size(); n++) {
        auto& node = graph[n];

        fout << '\n' << node.name << '\n' << node.edge.size();

        for (size_t e = 0; e < node.edge.size(); e++) {
            auto& edge = node.edge[e];
            fout << ' ' << edge.indx_from << ' ' << edge.indx_to << ' ' << edge.cost;
        }
    }

    fout.close();
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
    std::cout << 
"\
Graph Editor is a program, which was made for assisting making graphs\n\
Commands:\n\
\tnewp - create a node\n\
\tnewe - create an edge (from origin to...)\n\
\tseto - set origin node (to...)\n\
\tremp - remove a node and removing all references to the node (is...)\n\
\treme - remove an edge between origin and a node (from origin to...)\n\
\tremm - rename the current node\n\
\tlist - listing all nodes and costs which are adjacent to the origin node\n\
\tdict - shows a dictionary of names and it's translation to their indexes\n\
\tsave - saves graph in .grf using custom Python scripts\n\
\tload - loads graph from .grf using custom Python scripts\n\
\ttmpo - saves as cache for performance\n\
\ttmpi - loads cache from command 'tmpo' or from 'load' with debug flag\n\
\tclir - clears the screen\n\
\trset - resets graph to empty graph\n\
\texit - exit\n\
\n\
Also exists custom arguments:\n\
\t[--version | -v]  - get a version\n\
\t[-d]              - enter to debug mode (cache wouldn't be erased)\n\
\t[-h | --help]     - shows help for navigating the program\n\
\t[--argument | -a] - enter to argument mode (you can write all commands in arguments separated by space (for names as indexes you have to type 'n' and space before actual name))\n\
\n\
In argument mode:\n\
\t[-i] - alias to \"load\"\n\
\t[-o] - alias to \"save\"\n\
\t[-temp [i | o]] - alias to \"tmpi\" or \"tmpo\"\n";
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


void fSort() {
    using pair_name_map = std::pair<std::string, size_t>;

    std::vector<pair_name_map> vec_name_map;
    vec_name_map.reserve(name_map.size());
        
    for (auto& elem : name_map) {
        vec_name_map.push_back(elem);
    }
    std::sort(vec_name_map.begin(), vec_name_map.end(), [=](pair_name_map& a, pair_name_map& b) {return a.second < b.second; });

    name_map.clear();

    for (auto& elem : vec_name_map) {
        name_map.insert(elem);
    }
}


std::queue<std::string> pushInpToQueue(std::string input) {
    std::queue<std::string> queue;
    size_t crnt_space = input.find(' ');

    while (crnt_space != input.npos) {
        if (crnt_space != 1) {
            queue.push(input.substr(0, crnt_space));
            input.erase(input.begin(), input.begin() + crnt_space + 1);
        } else input.erase(input.begin());

        crnt_space = input.find(' ');
    }

    if (crnt_space != 1) {
        queue.push(input.substr(0, crnt_space));
    }

    return queue;
}

void fNewFunction() {
    Function func;
    std::string name;

    std::cout << "Choose a name for a function: ";
    std::getline(std::cin >> std::ws, name);

    std::cout << "Insert a logic for the function(as for argument mode): ";
    std::getline(std::cin >> std::ws, inp);
    func.input_queue = pushInpToQueue(inp);

    std::cout << "Does the function accepts and returns cache?(Y/n): ";
    std::cin >> inp;
    func.require_cache = (inp[0] == 'Y');

    functions.insert(std::make_pair(name, func));
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
        const char* arg = argv[arg_count];
        if ((strcmp(arg, "--version") | strcmp(arg, "-v")) == 0) {
            std::cout << "Graph Editor v" << VERSION << "." << ENV << "." << ARC << '\n';
            return 2;
        } else if (strcmp(arg, "-d") == 0) {
            debug_flag = true;
            arg = argv[++arg_count];
        } else if ((strcmp(arg, "-h") | strcmp(arg, "--help")) == 0) {
            fHelp();
            return 2;
        } else if ((strcmp(arg, "-a") | strcmp(arg, "--argument")) == 0) {
            argument_flag = true;
            arg = argv[++arg_count];
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
        case 'newp': fNewPoint();         break;
        case 'newe': fNewEdge();          break;
        case 'seto': fSetOrigin();        break;
        case 'remp': fRemovePoint();      break;
        case 'reme': fRemoveEdge();       break;
        case 'renm': fRenamePoint();      break;
        case 'list': fList();             break;
        case 'dict': fDictionPoint();     break;
        case 'save': fSaveGraph();        break;
        case 'load': fLoadGraph();        break;
        case 'clir': fClear();            break;
        case 'sort': fSort();             break;
        case 'tmpo': generateTempCache(); break;
        case 'tmpi': parseCache(); break;
        case '?':
        case 'help': fHelp();             break;
        case 'rset': fReset();            break;
        case 'lsta': fListAll();          break;

        default: std::cout << "invl\n"; [[fallthrough]];
        case 'exit': break;
        }
    } catch (const std::exception e) {
        std::cout << "excp: " << e.what() << '\n';
    }
}

void loopArgumentInputQueue(const size_t arg_queue_size) {
    while (arg_input_queue.size()) {
        std::cout << arg_queue_size - arg_input_queue.size() << ' ';
        if (arg_input_queue.front() == "-i") {
            arg_input_queue.pop();
            fLoadGraph();
        } else if (arg_input_queue.front() == "-o") {
            arg_input_queue.pop();
            fSaveGraph();
        } else if (arg_input_queue.front() == "--temp") {
            arg_input_queue.pop();
            if (arg_input_queue.front() == "o")
                generateTempCache();
            else if (arg_input_queue.front() == "i")
                parseCache();

            arg_input_queue.pop();
        } else {
            int num = convertToInt(arg_input_queue.front());
            arg_input_queue.pop();
            executeIntCommand(num);
        }
    }
}

int enterArgumentMode(const int args, const char* argv[], size_t arg_count) {
    for (size_t i = arg_count; i < args; i++) {
        if (debug_flag) {
            std::cout << '"' << argv[i] << "\"\n";
        }
        arg_input_queue.push(argv[i]);
    }

    loopArgumentInputQueue(args - arg_count);

    return 0;
}

int enterManualMode() {
    while (convertToInt(inp) != 'exit') {
        std::cout << "|";
        if (nod_origin_index < graph.size())
            std::cout << '"' << graph[nod_origin_index].name.c_str() << '"';
        else
            std::cout << "[null]";

        std::cout << " inp: ";
        std::getline(std::cin >> std::ws, inp);

        if(inp.find(' ') == inp.npos)
            executeIntCommand(convertToInt(inp));
        else {
            argument_flag = true;

            arg_input_queue = pushInpToQueue(inp);
            loopArgumentInputQueue(arg_input_queue.size());

            argument_flag = false;
            std::cout << '\n';
        }
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
