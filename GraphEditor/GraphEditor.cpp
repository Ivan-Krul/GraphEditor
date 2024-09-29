#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <queue>
#include <cstring>
#include <algorithm>


#define VERSION "1.3.0"

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



constexpr char c_mark_string = '"';
constexpr char c_mark_variable = '$';
constexpr char c_mark_root_graph_name = '_';
constexpr char c_mark_root_separator = ':';
constexpr char c_mark_index_scope_op = '[';
constexpr char c_mark_index_scope_cl = ']';


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

struct Variable {
    union {
        int    whol;
        float  flot;
        size_t indx = 0;
    } value;
    enum : char {
        whol,
        flot,
        indx
    } type = indx;
};

struct Function {
    std::queue<std::string> input_queue;
    bool require_cache = false;
};


inline int convertToInt(const std::string& str) {
    int orig = *(reinterpret_cast<const int*>(str.c_str()));
    return ((orig & 0xff000000) >> 24) | ((orig & 0xff0000) >> 8) | ((orig & 0xff00) << 8) | ((orig & 0xff) << 24);
}

std::unordered_map<std::string, size_t> name_map;
std::vector<Node> graph;
size_t nod_origin_index = 0;
std::string inp = "";

struct {
    bool arg_mode : 1;
    bool debug_mode : 1;
} argument_flag;

std::unordered_map<std::string, Variable> variables;
std::unordered_map<std::string, Function> functions;
std::queue<std::string> arg_input_queue;
Variable active_variable;

void executeIntCommand(int);
void loopArgumentInputQueue(const size_t arg_queue_size);
bool preProcressVariablesArg();
bool preProcressVariablesInp();
std::ostream& OutErr(std::ostream& out);

template<typename T>
std::enable_if_t<std::is_arithmetic_v<T>, bool> fillWithVariable(T& number) {
    if (!(argument_flag.arg_mode ? preProcressVariablesArg() : preProcressVariablesInp())) return true;

    switch (active_variable.type) {
#pragma warning(suppress: 4244)
    case Variable::whol: number = active_variable.value.whol; break;
#pragma warning(suppress: 4244)
    case Variable::indx: number = active_variable.value.indx; break;
#pragma warning(suppress: 4244)
    case Variable::flot: number = active_variable.value.flot; break;
    }

    return false;
}


inline bool checkVariableName(const std::unordered_map<std::string, Variable>::iterator& iter) {
    if (iter == variables.end()) { std::cout << OutErr << "the variable wasn't found\n"; return true; }
    return false;
}


inline bool checkGraphName(const std::unordered_map<std::string, size_t>::iterator& iter) {
    if (iter == name_map.end()) { std::cout << OutErr << "not found by name\n"; return true; }
    return false;
}


inline bool checkGraphRange(size_t index) {
    if (index >= graph.size()) { std::cout << OutErr << "index is out of range\n"; return true; }
    return false;
}


inline bool checkForInsetingVariable() {
    return (argument_flag.arg_mode ? arg_input_queue.front() : inp)[0] == c_mark_variable;
}


void inputNodeName() {
    if (argument_flag.arg_mode) {
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


size_t getNodeIndex() {
    auto iter = name_map.begin();

    if (!argument_flag.arg_mode) {
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

inline bool checkForValidVariables() {
    if (!argument_flag.arg_mode) std::cin >> inp;
    return checkForInsetingVariable();
}
inline bool checkForValidVariables(const std::string& ask) {
    if (!argument_flag.arg_mode) {
        std::cout << ask << ": ";
        std::cin >> inp;
    }
    return checkForInsetingVariable();
}

inline std::string& getInputStringToConvert() {
    return argument_flag.arg_mode ? (inp = arg_input_queue.front(), arg_input_queue.pop(), inp) : inp;
}

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>> inputNumberFromVariable(T& number) {
    if (checkForValidVariables()) { if (fillWithVariable(number)) return; } else number = std::stof(getInputStringToConvert());
}
template<typename T>
std::enable_if_t<std::is_floating_point_v<T>> inputNumberFromVariable(T& number, const std::string& ask) {
    if (checkForValidVariables(ask)) { if (fillWithVariable(number)) return; } else number = std::stof(getInputStringToConvert());
}

template<typename T>
std::enable_if_t<std::is_signed_v<T> && !std::is_floating_point_v<T>> inputNumberFromVariable(T& number) {
    if (checkForValidVariables()) { if (fillWithVariable(number)) return; } else number = std::stoi(getInputStringToConvert());
}
template<typename T>
std::enable_if_t<std::is_signed_v<T> && !std::is_floating_point_v<T>> inputNumberFromVariable(T& number, const std::string& ask) {
    if (checkForValidVariables(ask)) { if (fillWithVariable(number)) return; } else number = std::stoi(getInputStringToConvert());
}

template<typename T>
std::enable_if_t<std::is_unsigned_v<T> && !std::is_floating_point_v<T>> inputNumberFromVariable(T& number) {
    if (checkForValidVariables()) { if (fillWithVariable(number)) return; } else number = std::stoull(getInputStringToConvert());
}
template<typename T>
std::enable_if_t<std::is_unsigned_v<T> && !std::is_floating_point_v<T>> inputNumberFromVariable(T& number, const std::string& ask) {
    if (checkForValidVariables(ask)) { if (fillWithVariable(number)) return; } else number = std::stoull(getInputStringToConvert());
}

template<typename T>
std::enable_if_t<std::is_arithmetic_v<T>> setVariable(Variable& var, T number) {
    switch (var.type) {
#pragma warning(suppress: 4244)
    case Variable::whol: active_variable.value.whol = number; break;
#pragma warning(suppress: 4244)
    case Variable::indx: active_variable.value.indx = number; break;
#pragma warning(suppress: 4244)
    case Variable::flot: active_variable.value.flot = number; break;
    }
}

inline std::ostream& OutErr(std::ostream& out) { if(argument_flag.arg_mode) out << "eror: "; return out; }

void embedEdge(Edge& edg) {
    inputNumberFromVariable(edg.cost, "cost");

    graph[edg.indx_from].edge.push_back(edg);
    graph[edg.indx_to].edge.push_back(edg);
}

void giveChoiceUseNameAsIndex() {
    if (argument_flag.arg_mode) {
        inp = isdigit(arg_input_queue.front()[0]) || arg_input_queue.front()[0] == c_mark_variable ? "n" : "Y";
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

        if ((edg.indx_to = getNodeIndex()) == -1) { std::cout << OutErr << "not found by name\n"; return; }

        embedEdge(edg);
    } else if (inp[0] == 'n') {
        Edge edg = { 0 };
        edg.indx_from = nod_origin_index;

        inputNumberFromVariable(edg.indx_to);

        if (checkGraphRange(edg.indx_to)) return;

        if (edg.indx_to == nod_origin_index) {
            std::cout << OutErr << "index is the same with origin\n";
            return;
        }

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
        if ((nod_origin_index = getNodeIndex()) == -1) {
            std::cout << OutErr << "not found by name\n";
            nod_origin_index = 0;
            return;
        }
    } else if (inp[0] == 'n') {
        inputNumberFromVariable(nod_origin_index);

        if(checkGraphRange(nod_origin_index)) { nod_origin_index = 0; return; }
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
        if ((targt = getNodeIndex()) == -1) { std::cout << OutErr << "not found by name\n"; return; }

        if (nod_origin_index >= targt) nod_origin_index--;

        removeAllReference(targt);

    } else if (inp[0] == 'n') {
        inputNumberFromVariable(targt);

        if (checkGraphRange(targt)) return;
        
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
        std::cout << a.second << ": \"" << a.first << "\"" << '\n';
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
        if ((targt = getNodeIndex()) == -1) { std::cout << OutErr << "not found by name\n"; return; }

        removeEdgeFromNodes(targt);

    } else if (inp[0] == 'n') {
        inputNumberFromVariable(targt);

        if (checkGraphRange(nod_origin_index)) return;

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
    if (argument_flag.arg_mode) return;

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
    if (argument_flag.arg_mode) {
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
    if (!argument_flag.debug_mode)
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
\trenm - rename the current node\n\
\tlist - list all nodes and costs which are adjacent to the origin node\n\
\tdict - shows a dictionary of names and it's translation to their indexes\n\
\tsave - saves graph in .grf using custom Python scripts\n\
\tload - loads graph from .grf using custom Python scripts\n\
\tclir - clears the screen\n\
\thelp - shows help for navigating the program\n\
\trset - resets graph to empty graph\n\
\tlsta - list whole graph with all edges of nodes\n\
\ttmpi - saves as cache for performance\n\
\ttmpo - loads cache from command 'tmpo' or from 'load' with debug flag\n\
\tnewf - creates a function was for argument list and saves in .func\n\
\tlstf - list loadedd functions\n\
\tcall - call a function, which execute all from argument list\n\
\treff - refresh functions from .func\n\
\tnewv - create a variable\n\
\tremv - remove the variable\n\
\tlstv - list all variables\n\
\trenv - rename a variable\n\
\tsetv - set variable's value with it's data type\n\
\toutv - output a single variable ($ before variable name)\n\
\tincv - increment a single variable as index ($ before variable name)\n\
\tdecv - decrement a single variable as index ($ before variable name)\n\
\tfile - extract commands from file and execute them\n\
\texit - exit\n\
\n\
Also exists custom arguments:\n\
\t[--version | -v]  - get a version\n\
\t[-d]              - enter to debug mode (cache wouldn't be erased)\n\
\t[-h | --help]     - shows help for navigating the program\n\
\t[--argument | -a] - enter to argument mode (you can write all commands in arguments separated by space (for names as indexes it recognise automatically))\n\
\t[-ssa]            - you can type your input using a single string argument (f.e. -ssa \"newp A newp B ...\")\n\
\n\
In argument mode:\n\
\t[-i] - alias to \"load\"\n\
\t[-o] - alias to \"save\"\n\
\t[-temp [i | o]] - alias to \"tmpi\" or \"tmpo\"\n\
\n\
Also for variables:\n\
\t[$(existing variable's name)] - insert variable's value right in to fields \n";
}


void fClear() {
    if (argument_flag.arg_mode) return;
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


void deleteSpaces(std::string& content) {
    bool inString = false;
    bool wasSpace = false;

    std::string result;
    result.reserve(content.size());

    for (size_t i = 0; i < content.size(); i++) {
        if (isspace(content[i])) {
            if (inString)
                result.push_back(content[i]);
            else if (!wasSpace)
                result.push_back(' ');

            wasSpace = true;
        } else if (content[i] == c_mark_string) {
            inString = !inString;
            result.push_back(content[i]);
            wasSpace = false;
        } else {
            result.push_back(content[i]);
            wasSpace = false;
        }
    }

    if (isspace(result[result.size() - 1]))
        result.erase(result.begin() + result.size() - 1);

    result.shrink_to_fit();

    content = result;
}


std::queue<std::string> pushInpToQueue(std::string input) {
    std::queue<std::string> queue;
    deleteSpaces(input);

    size_t crnt_space = input.find(' ');

    size_t counter_beg = 0;
    size_t counter_end = 0;

    while (crnt_space != input.npos) {
        if (crnt_space) {
            counter_end = crnt_space;
            if (input[counter_beg] == c_mark_string) {
                counter_beg++;
                counter_end = input.find_first_of(c_mark_string, counter_beg);
                queue.push(input.substr(counter_beg, counter_end - counter_beg));
                counter_beg = counter_end + 1;
            } else {
                queue.push(input.substr(counter_beg, counter_end - counter_beg));
                counter_beg = crnt_space + 1;
            }
        } else counter_beg++;

        crnt_space = input.find_first_of(' ', counter_beg);
    }

    if (counter_end < (input.size() - 1)) {
        counter_end = input[input.size() - 1] == input.size();
        if (input[counter_beg] == c_mark_string) {
            counter_beg++;
            counter_end = input.find_first_of(c_mark_string, counter_beg);
            queue.push(input.substr(counter_beg, counter_end - counter_beg));
        } else {
            queue.push(input.substr(counter_beg, counter_end - counter_beg));
        }
    }

    return queue;
}

void dumpToDotFunc() {
    std::ofstream fout;
    fout.open(".func");

    for (auto elem : functions) {
        fout << elem.first << '\t';
        
        const size_t queue_size = elem.second.input_queue.size();
        auto& queue = elem.second.input_queue;
        std::vector<std::string> vec_queue;
        vec_queue.reserve(queue_size);

        for (size_t i = 0; i < queue_size - 1; i++) {
            fout << queue.front() << ' ';
            queue.pop();
        }

        fout << queue.front() << '\t';
        fout << (elem.second.require_cache ? "cache" : "instance") << '\n';
    }

    fout.close();
}

void refreshFromDotFunc() {
    std::ifstream fin;
    fin.open(".func");

    std::string line;
    std::getline(fin, line);

    std::string name;
    size_t indexTo = 0;

    Function func;

    while (fin) {
        indexTo = line.find('\t');
        name = line.substr(0, indexTo);
        line.erase(line.begin(), line.begin() + indexTo + 1);

        indexTo = line.find('\t');
        func.input_queue = pushInpToQueue(line.substr(0, indexTo));
        line.erase(line.begin(), line.begin() + indexTo + 1);

        func.require_cache = line == "cache";

        functions.insert(std::make_pair(name, func));

        std::getline(fin, line);
    }

    fin.close();
}

void fNewFunction() {
    Function func;
    std::string name;

    if (argument_flag.arg_mode) {
        name = arg_input_queue.front();
        arg_input_queue.pop();

        func.input_queue = pushInpToQueue(arg_input_queue.front());
        arg_input_queue.pop();

        func.require_cache = arg_input_queue.front() == "cache";
    }
    else {
        std::cout << "Choose a name for a function: ";
        std::getline(std::cin >> std::ws, name);

        std::cout << "Insert a logic for the function(as for argument mode): ";
        std::getline(std::cin >> std::ws, inp);
        func.input_queue = pushInpToQueue(inp);

        std::cout << "Does the function accepts and returns cache?(Y/n): ";
        std::cin >> inp;
        func.require_cache = (inp[0] == 'Y');
    }

    functions.insert(std::make_pair(name, func));

    dumpToDotFunc();
}


void fListFunctions() {
    std::cout << "Functions:\n";
    for(auto& elem : functions) {
        std::cout << "\t -> \"" << elem.first << "\"\n";
    }
}


void fCallFunction() {
    if (argument_flag.arg_mode) {
        inp = arg_input_queue.front();
        arg_input_queue.pop();
    }
    else {
        std::cout << "Function: ";
        std::getline(std::cin >> std::ws, inp);
    }

    auto& func = functions.at(inp);
    auto orig_arg_input_queue = arg_input_queue;

    arg_input_queue = func.input_queue;
    argument_flag.arg_mode = true;

    if (func.require_cache) {
        auto orig_graph = graph;
        auto orig_name_map = name_map;
        auto orig_nod_origing_index = nod_origin_index;
        
        parseCache();
        loopArgumentInputQueue(arg_input_queue.size());
        generateTempCache();

        graph = orig_graph;
        name_map = orig_name_map;
        nod_origin_index = orig_nod_origing_index;
    }
    else {
        arg_input_queue = func.input_queue;
        loopArgumentInputQueue(arg_input_queue.size());
    }

    arg_input_queue = orig_arg_input_queue;
    argument_flag.arg_mode = false;
    std::cout << '\n';
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

template<typename T>
std::queue<T> insertQueueToFrontQueue(std::queue<T> base, std::queue<T> field) {
    while (!base.empty()) {
        field.push(base.front());
        base.pop();
    }

    return field;
}

void extractFromFileToQueue(size_t& arg_queue_size) {
    std::string content;
    {
        std::ifstream fin;
        fin.open(arg_input_queue.front());
        arg_input_queue.pop();

        std::getline(fin, content, '\0');

        fin.close();
    }

    arg_input_queue = insertQueueToFrontQueue(arg_input_queue, pushInpToQueue(content));

    argument_flag.arg_mode = true;
    arg_queue_size = arg_input_queue.size();
}


void fFile() {
    size_t queue_size;
    if (!argument_flag.arg_mode) {
        std::cout << "File to fetch commands: ";
        std::string str;
        std::getline(std::cin >> std::ws, str);
        arg_input_queue.push(str);
    }
    extractFromFileToQueue(queue_size);
    loopArgumentInputQueue(queue_size);
}


void fNewVariable() {
    if (!argument_flag.arg_mode) {
        std::cout << "Enter a variable name: ";
        std::getline(std::cin >> std::ws, inp);
    }
    else {
        inp = arg_input_queue.front();
        arg_input_queue.pop();
    }

    if (inp[0] == c_mark_root_graph_name) {
        std::cout << OutErr << "variable is being named like root ('"<<c_mark_root_graph_name<<"' at beginning of the name) is not supposed to exist\n";
        return;
    }

    variables.insert(std::make_pair(inp, Variable{}));
}


void fRemoveVariable() {
    if (argument_flag.arg_mode) {
        inp = arg_input_queue.front();
        arg_input_queue.pop();
    }
    else {
        std::cout << "Variable name for removal: ";
        std::getline(std::cin >> std::ws, inp);
    }

    auto iter = variables.find(inp);
    if (checkVariableName(iter)) return;

    variables.erase(iter);
}


void fListVariables() {
    std::cout << "Variables:\n";
    for (auto& pair : variables) {
        std::cout << "\t" << pair.first << "\t = ";
        switch (pair.second.type) {
        case Variable::whol:
            std::cout << "[whol]" << pair.second.value.whol << "\n"; break;
        case Variable::flot:
            std::cout << "[flot]" << pair.second.value.flot << "\n"; break;
        case Variable::indx:
            std::cout << "[indx]" << pair.second.value.indx << "\n"; break;
        }
    }
}


void fRenameVariable() {
    std::string from;
    auto iter = variables.begin();

    if (argument_flag.arg_mode) {
        from = arg_input_queue.front();
        arg_input_queue.pop();

        iter = variables.find(from);
        if (checkVariableName(iter)) return;

        inp = arg_input_queue.front();
        arg_input_queue.pop();
    }
    else {
        std::cout << "Variable name to be renamed: ";
        std::getline(std::cin >> std::ws, from);

        iter = variables.find(from);
        if (checkVariableName(iter)) return;

        std::cout << "New variable name: ";
        std::getline(std::cin >> std::ws, inp);
    }

    auto var = iter->second;
    variables.erase(iter);
    variables.insert({ inp, var });
}


void setVariableFromVariable(Variable& var, const Variable& varFrom) {
    // so we got an interaction table
            //
            //     iter_from
            //   v| i f n
            //   -+-------
            // i i| = f i 
            // t  |
            // e f| i = n
            // r  |
            //   n| n n =
            //
            // when:
            //   = - sets variable without changes
            //   [i|f|n] - sets variable for type,
            //               if 'iter' side matches with value,then it would be casted approximately,
            //               otherwise it would be casted directly how are they represent
            //

    switch (var.type) {
    case Variable::whol:
        switch (varFrom.type) {
        case Variable::whol: var.value.whol = varFrom.value.whol; break;
        case Variable::flot: var.value.whol = varFrom.value.flot; break;
        case Variable::indx: var.value.whol = varFrom.value.whol; break;
        } break;
    case Variable::flot:
        switch (varFrom.type) {
        case Variable::whol: var.value.flot = varFrom.value.whol; break;
        case Variable::flot: var.value.flot = varFrom.value.flot; break;
        case Variable::indx: var.value.flot = varFrom.value.indx; break;
        } break;
    case Variable::indx:
        switch (varFrom.type) {
        case Variable::whol: var.value.flot = varFrom.value.indx; break;
        case Variable::flot: var.value.flot = varFrom.value.indx; break;
        case Variable::indx: var.value.flot = varFrom.value.indx; break;
        } break;
    }
} 


size_t getIndexInsideScopes(size_t& sepFirst) {
    size_t sepSecond = inp.find_first_of(c_mark_index_scope_cl, ++sepFirst);
    size_t index = 0;

    if (inp[sepFirst] == c_mark_variable) {
        auto iter_from = variables.find(inp.substr(++sepFirst, sepSecond - sepFirst));
        if (checkVariableName(iter_from)) return -1;

        index = iter_from->second.value.indx;
    } else if (isdigit(inp[sepFirst]) != 0)
        index = std::stoull(inp.substr(sepFirst, sepSecond - sepFirst));
    else {
        auto iter_from = name_map.find(inp.substr(++sepFirst, sepSecond - sepFirst));
        if (checkGraphName(iter_from)) return -1;

        index = iter_from->second;
    }

    sepFirst = sepSecond;

    return index;
}

void fSetVariable() {
    auto iter = variables.begin();

    if (argument_flag.arg_mode) {
        iter = variables.find(arg_input_queue.front());
        arg_input_queue.pop();
        if (checkVariableName(iter)) return;

             if (convertToInt(arg_input_queue.front()) == 'whol') iter->second.type = Variable::whol;
        else if (convertToInt(arg_input_queue.front()) == 'flot') iter->second.type = Variable::flot;
        else if (convertToInt(arg_input_queue.front()) == 'indx') iter->second.type = Variable::indx;
        else    { std::cout << OutErr << "the value type wasn't settled\n"; arg_input_queue.pop(); return; }
        arg_input_queue.pop();

        inp = arg_input_queue.front();
        arg_input_queue.pop();
    } else {
        std::cout << "Variable name: ";
        std::getline(std::cin >> std::ws, inp);

        iter = variables.find(inp);
        if (checkVariableName(iter)) return;

        std::cout << "Type [whol(eq. int) / flot(eq. float) / indx]: ";
        std::cin >> inp;

             if (convertToInt(inp) == 'whol') iter->second.type = Variable::whol;
        else if (convertToInt(inp) == 'flot') iter->second.type = Variable::flot;
        else if (convertToInt(inp) == 'indx') iter->second.type = Variable::indx;
        else    { std::cout << OutErr << "the value type wasn't settled\n"; return; }

        std::cout << "Value: ";
        std::cin >> inp;
    }

    if(inp[0] != c_mark_variable) {
        switch (iter->second.type) {
        case Variable::whol: iter->second.value.whol = std::stoi(inp);  break;
        case Variable::flot: iter->second.value.flot = std::stof(inp);  break;
        case Variable::indx: iter->second.value.indx = std::stoull(inp); break;
        }
    } else {
        // set variable from existing value
        if (inp[1] == c_mark_root_graph_name) {
            // if there's a root, then all the string with gibrish would be ignored till c_mark_root_separator
            // \$_\.\[($\w+|\d+|[^_]\w*)\]\[($\w+|\d+|[^_]\w*)\]\.(from|to|cost)

            constexpr const char* c_str_origin = "origin";

            size_t sepFirst = inp.find_first_of(c_mark_root_separator, 2) + 1;

            if (inp[sepFirst] == c_mark_index_scope_op) {
                // indexing with various methods
                size_t indexNod = getIndexInsideScopes(sepFirst);

                if (indexNod == -1) return;
                if (checkGraphRange(indexNod)) return;

                if (inp[++sepFirst] != c_mark_index_scope_op) {
                    std::cout << OutErr << "there's no edge index scope after node index scope\n";
                    return;
                }

                size_t indexEdg = getIndexInsideScopes(sepFirst);
                if (indexEdg == -1) return;
                if (indexEdg >= graph[indexNod].edge.size()) {
                    std::cout << OutErr << "direct index of graph edges is out of range\n";
                    return;
                }

                if (inp[++sepFirst] != c_mark_root_separator) {
                    std::cout << OutErr << "there's no separator after index scopes\n";
                    return;
                }

                constexpr const char* c_str_from = "from";
                constexpr const char* c_str_to   = "to";
                constexpr const char* c_str_cost = "cost";

                if (strcmp(inp.data() + sepFirst + 1, c_str_from) == 0)
                    setVariable(iter->second, graph[indexNod].edge[indexEdg].indx_from);
                else if (strcmp(inp.data() + sepFirst + 1, c_str_to) == 0)
                    setVariable(iter->second, graph[indexNod].edge[indexEdg].indx_to);
                else if (strcmp(inp.data() + sepFirst + 1, c_str_cost) == 0)
                    setVariable(iter->second, graph[indexNod].edge[indexEdg].cost);
                else {
                    std::cout << OutErr << "no variable type isn't matching\n";
                    return;
                }
            }
            else if (inp.substr(sepFirst, sepFirst + strlen(c_str_origin)) == c_str_origin) setVariable(iter->second, nod_origin_index);
        }
        else setVariableFromVariable(iter->second, variables.find(inp.substr(1))->second);

    }
}


void fOutputVariable() {
    if (!argument_flag.arg_mode) std::cin >> inp;
    if (!(argument_flag.arg_mode ? preProcressVariablesArg() : preProcressVariablesInp())) return;

    std::cout << "[-]" << "\t = ";
    switch (active_variable.type) {
    case Variable::whol:
        std::cout << "[whol]" << active_variable.value.whol << "\n"; break;
    case Variable::flot:
        std::cout << "[flot]" << active_variable.value.flot << "\n"; break;
    case Variable::indx:
        std::cout << "[indx]" << active_variable.value.indx << "\n"; break;
    }
}


decltype(variables.begin()) getVariableInstance() {
    auto iter = variables.begin();
    if (!argument_flag.arg_mode) {
        std::cin >> inp;
        iter = variables.find(inp.substr(1));
    } else {
        iter = variables.find(arg_input_queue.front().substr(1));
        arg_input_queue.pop();
    }

    return iter;
}


void fIncrementVariable() {
    auto iter = getVariableInstance();
    if (checkVariableName(iter)) return;

    iter->second.value.indx++;
}

void fDecrementVariable() {
    auto iter = getVariableInstance();
    if (checkVariableName(iter)) return;

    iter->second.value.indx--;
}


bool preProcressVariablesInp() {
    auto iter = variables.find(inp.substr(1));
    if (checkVariableName(iter)) return false;

    active_variable = iter->second;

    return true;
}
bool preProcressVariablesArg() {
    auto iter = variables.find(arg_input_queue.front().substr(1));
    arg_input_queue.pop();
    if (checkVariableName(iter)) return false;

    active_variable = iter->second;

    return true;
}


int init(const int args, const char* argv[], size_t& arg_count) {
    if (args > 1) {
        const char* arg = argv[arg_count];
        if (strcmp(arg, "--version") == 0 || strcmp(arg, "-v") == 0) {
            std::cout << "Graph Editor v" << VERSION << "." << ENV << "." << ARC << '\n';
            return 2;
        } else if (strcmp(arg, "-d") == 0) {
            argument_flag.debug_mode = true;
            std::cout << "Debug\n";
            arg = argv[++arg_count];
        } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            fHelp();
            return 2;
        } else if (strcmp(arg, "-a") == 0 || strcmp(arg, "--argument") == 0) {
            argument_flag.arg_mode = true;
            arg = argv[++arg_count];
        } else if (strcmp(arg, "-f") == 0 || strcmp(arg, "--file") == 0) {
            extractFromFileToQueue(arg_count);
            argument_flag.arg_mode = true;
            arg = argv[++arg_count];
        }
    }


    if (isPythonMissing()) {
        std::cout << OutErr << "python is missing in the enviroument\n";
        return 1;
    }

    refreshFromDotFunc();

    return 0;
}

void executeIntCommand(int input) {
    try {
        switch (input) {
        case 'newp': fNewPoint();          break;
        case 'newe': fNewEdge();           break;
        case 'seto': fSetOrigin();         break;
        case 'remp': fRemovePoint();       break;
        case 'reme': fRemoveEdge();        break;
        case 'renm': fRenamePoint();       break;
        case 'list': fList();              break;
        case 'dict': fDictionPoint();      break;
        case 'save': fSaveGraph();         break;
        case 'load': fLoadGraph();         break;
        case 'clir': fClear();             break;
        case 'help': fHelp();              break;
        case 'rset': fReset();             break;
        case 'lsta': fListAll();           break;
        case 'tmpi': parseCache();         break;
        case 'tmpo': generateTempCache();  break;
        case 'newf': fNewFunction();       break;
        case 'lstf': fListFunctions();     break;
        case 'call': fCallFunction();      break;
        case 'reff': refreshFromDotFunc(); break;
        case 'newv': fNewVariable();       break;
        case 'remv': fRemoveVariable();    break;
        case 'lstv': fListVariables();     break;
        case 'renv': fRenameVariable();    break;
        case 'setv': fSetVariable();       break;
        case 'incv': fIncrementVariable(); break;
        case 'decv': fDecrementVariable(); break;
        case 'outv': fOutputVariable();    break;
        case 'file': fFile();              break;

        default: std::cout << "invl\n"; [[fallthrough]];
        case 'exit': break;
        }
    } catch (const std::exception e) {
        std::cout << "excp: " << e.what() << '\n';
    }
}

void loopArgumentInputQueue(size_t arg_queue_size) {
    auto was = arg_input_queue.size();
    while (arg_input_queue.size()) {
        std::cout << was - arg_input_queue.size() << ' ';
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
        } else if (arg_input_queue.front() == "-f" || arg_input_queue.front() == "--file") {
            arg_input_queue.pop();
            extractFromFileToQueue(was);
        } else if (arg_input_queue.front() == "-ssa") {
            arg_input_queue.pop();
            const auto str = arg_input_queue.front();
            arg_input_queue.pop();
            arg_input_queue = insertQueueToFrontQueue(arg_input_queue, pushInpToQueue(str));
            was = arg_input_queue.size();
        } else {
            const int num = convertToInt(arg_input_queue.front());
            arg_input_queue.pop();
            executeIntCommand(num);
        }
    }
}

int enterArgumentMode(const int args, const char* argv[], size_t arg_count) {
    for (size_t i = arg_count; i < args; i++) {
        if (argument_flag.debug_mode) {
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
            argument_flag.arg_mode = true;

            arg_input_queue = pushInpToQueue(inp);
            loopArgumentInputQueue(arg_input_queue.size());

            argument_flag.arg_mode = false;
            std::cout << '\n';
        }
    }

    return 0;
}

int main(const int args, const char* argv[]) {
    size_t arg_count = 1;

    int argRet = init(args, argv, arg_count);
    if (argRet > 0 && !argument_flag.arg_mode) return 2 - argRet;

    return argument_flag.arg_mode
        ? enterArgumentMode(args, argv, arg_count)
        : enterManualMode();
}
