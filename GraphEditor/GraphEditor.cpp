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
#include <array>
#include <map>
#include <variant>
#include <optional>


#define VERSION "1.4.0"
#define DISABLE_WARNINGS

#if defined(_WIN32) || defined(_WIN64) || \
  defined(__WIN32__) || defined(__TOS_WIN__) || \
  defined(__WINDOWS__)
#  define ENV "windows"
#  define Windows
#elif defined(linux) || defined(__linux) || \
  defined(__linux__) || defined(__gnu_linux__)
#  define ENV "linux"
#elif defined(macintosh) || defined(Macintosh) || \
  (defined(__APPLE__) && defined(__MACH__))
#  define ENV "macos"
#else
#  error Unsupported OS
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define ARC "x86_64"
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define ARC "x86_32"
#else
#define ARC "arm"
#endif

#ifdef DISABLE_WARNINGS
#pragma warning(disable : 4244)
#pragma warning(disable : 4267)
#endif



constexpr char c_mark_string = '"';
constexpr char c_mark_variable = '$';
constexpr char c_mark_temp_function = '!';
constexpr char c_mark_root_graph_name = '_';
constexpr char c_mark_root_separator = ':';
constexpr char c_mark_index_scope_op = '[';
constexpr char c_mark_index_scope_cl = ']';


struct Node;

struct Edge {
  unsigned int indx_to;
  unsigned int indx_from;
  float cost;
};

struct Node {
  std::string name;
  std::vector<Edge> edge;
};

struct Variable {
  union Value {
    int  whol;
    float  flot;
    size_t indx = 0;
  } value;
  enum : char {
    whol,
    flot,
    indx
  } type = indx;
};


inline int convertStrToRawInt(const std::string& str) {
  int orig = *(reinterpret_cast<const int*>(str.c_str()));
  return ((orig & 0xff000000) >> 24) | ((orig & 0xff0000) >> 8) | ((orig & 0xff00) << 8) | ((orig & 0xff) << 24);
}


inline int convertIntToRawStr(int orig) {
  return ((orig & 0xff000000) >> 24) | ((orig & 0xff0000) >> 8) | ((orig & 0xff00) << 8) | ((orig & 0xff) << 24);
}


std::vector<Node> nodes;
std::vector<Edge> edges;

std::unordered_map<std::string, size_t> name_map;
std::vector<Node> graph;;

size_t nod_origin_index = 0;
std::string inp = "";

struct {
  bool arg_mode : 1;
  bool debug_mode : 1;
  bool halt : 1;
  bool need_to_override_when_halts : 1;
  bool exit : 1;
} argument_flag;

std::unordered_map<std::string, Variable> variables;

std::unordered_map<std::string, std::queue<std::string>>      functions;
std::unordered_map<std::string, std::queue<std::string>>      functions_cache;
std::unordered_map<std::string, std::queue<std::string>> temp_functions;
std::unordered_map<std::string, std::queue<std::string>> temp_functions_cache;

std::queue<std::string> arg_input_queue;
Variable active_variable;

void executeIntCommand(int);
void loopArgumentInputQueue(const size_t arg_queue_size);
bool preProcressVariablesArg();
bool preProcressVariablesInp();
std::ostream& OutErr(std::ostream& out);
int enterManualMode();
const std::map<int, void (*)()>& getCommandList();

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


template<typename T>
inline T returnError(T returnsym, const std::string& message, std::ostream& out = std::cerr) {
  out << OutErr << message << std::endl;
  return returnsym;
}

inline void returnError(const std::string& message, std::ostream& out = std::cerr) {
  out << OutErr << message << std::endl;
}

template<typename T>
inline std::optional<T> returnError(const std::string& message, std::ostream& out = std::cerr) {
  out << message << std::endl;
  return std::nullopt;
}


inline bool checkVariableName(const std::unordered_map<std::string, Variable>::iterator& iter) {
  if (iter == variables.end()) return returnError(true, "the variable wasn't found");
  return false;
}


inline bool checkGraphName(const std::unordered_map<std::string, size_t>::iterator& iter) {
  if (iter == name_map.end()) return returnError(true, "not found by name");
  return false;
}


inline bool checkGraphRange(size_t index) {
  if (index >= graph.size()) returnError(true,"index is out of range");
  return false;
}


inline void checkValidArgumentCount(size_t size) {
  if (size > arg_input_queue.size() && argument_flag.arg_mode)
    throw std::exception((std::string("esze: ") + std::to_string(size) + "! vs " + std::to_string(arg_input_queue.size())).c_str());
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
  checkValidArgumentCount(1);
  inputNodeName();

  Node nd;
  //name_array.emplace_back(std::make_unique<char>(inp.capacity()));

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
std::enable_if_t<std::is_floating_point_v<T>> inputNumber(T& number) {
  if (checkForValidVariables()) { if (fillWithVariable(number)) return; } else number = std::stof(getInputStringToConvert());
}
template<typename T>
std::enable_if_t<std::is_floating_point_v<T>> inputNumber(T& number, const std::string& ask) {
  if (checkForValidVariables(ask)) { if (fillWithVariable(number)) return; } else number = std::stof(getInputStringToConvert());
}

template<typename T>
std::enable_if_t<std::is_signed_v<T> && !std::is_floating_point_v<T>> inputNumber(T& number) {
  if (checkForValidVariables()) { if (fillWithVariable(number)) return; } else number = std::stoi(getInputStringToConvert());
}
template<typename T>
std::enable_if_t<std::is_signed_v<T> && !std::is_floating_point_v<T>> inputNumber(T& number, const std::string& ask) {
  if (checkForValidVariables(ask)) { if (fillWithVariable(number)) return; } else number = std::stoi(getInputStringToConvert());
}

template<typename T>
std::enable_if_t<std::is_unsigned_v<T> && !std::is_floating_point_v<T>> inputNumber(T& number) {
  if (checkForValidVariables()) { if (fillWithVariable(number)) return; } else number = std::stoull(getInputStringToConvert());
}
template<typename T>
std::enable_if_t<std::is_unsigned_v<T> && !std::is_floating_point_v<T>> inputNumber(T& number, const std::string& ask) {
  if (checkForValidVariables(ask)) { if (fillWithVariable(number)) return; } else number = std::stoull(getInputStringToConvert());
}

template<typename T>
std::enable_if_t<std::is_arithmetic_v<T>> setVariable(Variable& var, T number) {
  switch (var.type) {
  case Variable::whol: var.value.whol = number; break;
  case Variable::indx: var.value.indx = number; break;
  case Variable::flot: var.value.flot = number; break;
  }
}

inline std::ostream& OutErr(std::ostream& out) { 
  if(argument_flag.arg_mode) out << "eror: ";
  argument_flag.halt = true;
  return out;
}

void giveChoiceUseNameAsIndex() {
  if (argument_flag.arg_mode) {
    inp = isdigit(arg_input_queue.front()[0]) || arg_input_queue.front()[0] == c_mark_variable ? "n" : "Y";
  } else {
    std::cout << "use name as index(Y/n): ";
    std::cin >> inp;
  }
}

size_t handleEdgeIndexToInput() {
  size_t targt = 0;
  if (inp[0] == 'Y') {
    if ((targt = getNodeIndex()) == -1) return returnError(-1, "not found by name");
  } else if (inp[0] == 'n') {
    if (!argument_flag.arg_mode) std::cout << "index: ";
    inputNumber(targt);

    if (checkGraphRange(targt)) return -1;

    if (targt == nod_origin_index) return returnError(-1, "index is the same with origin");
  } else return returnError(-1, "way of input is undefined");
  return targt;
}

void fNewEdge() {
  if (graph.size() < 2) return;
  checkValidArgumentCount(2);
  giveChoiceUseNameAsIndex();

  Edge edg = { 0 };
  edg.indx_from = nod_origin_index;
  edg.cost = NAN;

  if ((edg.indx_to = handleEdgeIndexToInput()) == -1) return;

  graph[edg.indx_from].edge.push_back(edg);
  graph[edg.indx_to].edge.push_back(edg);
}


inline size_t getEdgeIndex(size_t indx_from, size_t indx_to) {
  for (size_t i = 0; i < graph[indx_from].edge.size(); i++) {
    if (graph[indx_from].edge[i].indx_to == indx_to) return i;
  }
  return -1;
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

    if (std::isnan(cur.edge[i].cost) && std::isnan(graph[nod_to].edge[getEdgeIndex(nod_to, nod_origin_index)].cost)) std::cout << "nous ";

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

  checkValidArgumentCount(1);
  giveChoiceUseNameAsIndex();

  if (inp[0] == 'Y') {
    if ((nod_origin_index = getNodeIndex()) == -1) {
      nod_origin_index = 0;
      return returnError("not found by name");
    }
  } else if (inp[0] == 'n') {
    inputNumber(nod_origin_index);

    if(checkGraphRange(nod_origin_index)) { nod_origin_index = 0; return; }
  }
}


void removeRefs(size_t targt) {
  auto& edges = graph[targt].edge;

  size_t refs;
  size_t i = 0;
  size_t j = 0;

  for (i = 0; i < edges.size(); i++) {
    refs = edges[i].indx_to;

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

    for (size_t i = 0; i < graph[dict.second].edge.size(); i++) {
      if (graph[dict.second].edge[i].indx_from >= targt)
        graph[dict.second].edge[i].indx_from--;
      if (graph[dict.second].edge[i].indx_to >= targt)
        graph[dict.second].edge[i].indx_to--;
    }
  }
}

void fRemovePoint() {
  if (graph.size() == 1) graph.pop_back();
  if (graph.empty()) return;

  checkValidArgumentCount(1);
  giveChoiceUseNameAsIndex();

  size_t targt;

  if (inp[0] == 'Y') {
    if ((targt = getNodeIndex()) == -1) return returnError("not found by name");

    if (nod_origin_index >= targt) nod_origin_index--;

    removeAllReference(targt);

  } else if (inp[0] == 'n') {
    inputNumber(targt);

    if (checkGraphRange(targt)) return;
    
    if (nod_origin_index >= targt) nod_origin_index--;

    removeAllReference(targt);
  }
}


int inputDirection() {
  std::string direct_str;
  if (argument_flag.arg_mode) {
    direct_str = arg_input_queue.front();
    arg_input_queue.pop();
  } else {
    std::cout << "The direction for applying the cost of the edge: ";
    std::cin >> direct_str;
  }

  if (direct_str.size() > 3 || direct_str.size() < 2) return returnError(-1, "Wrong format of directions");

  return (*(int*)(direct_str.c_str()) & (direct_str.size() == 3 ? 0x00FFFFFF : 0x0000FFFF));
}


inline void setEdgeIfPossible(Edge& from, Edge& to, float cost_from, float cost_to) {
  from.cost = cost_from;
  to.cost = cost_to;
}

inline void inputCost(float& cost) {
  if (!argument_flag.arg_mode) std::cout << "cost: ";
  inputNumber(cost);
}

void fSetEdge() {
// sete $target <-> 3
// sete $target <- 5
// sete $target -> 1
// sete $target <=> 2 5
// sete $target <>

  if (graph.size() < 2) return;
  checkValidArgumentCount(2);
  giveChoiceUseNameAsIndex();

  float cost = NAN;
  int dir_raw = 0;
  size_t targt_indx = handleEdgeIndexToInput();

  if (targt_indx == -1) return;

  size_t edge_indx_to   = getEdgeIndex(nod_origin_index, targt_indx);
  size_t edge_indx_from = getEdgeIndex(targt_indx, nod_origin_index);

  if (edge_indx_to == -1 || edge_indx_from == -1) return returnError("the edge target isn't connected to the index " + targt_indx);
  if ((dir_raw = inputDirection()) == -1) return;

  Edge& from = graph[nod_origin_index].edge[edge_indx_to];
  Edge& to   = graph[targt_indx].edge[edge_indx_from];

  if(dir_raw != '><') inputCost(cost);

  if (dir_raw == '><' || dir_raw == '>-<') { setEdgeIfPossible(from, to, cost, cost); return; }

  if (dir_raw != '>=<') {
    if (dir_raw == '-<') { setEdgeIfPossible(from, to, cost, NAN); return;  }
    else if (dir_raw == '>-') { setEdgeIfPossible(from, to, NAN, cost); return;  }
  }
  else {
    float cost_2;
    inputCost(cost);
    inputCost(cost_2);
    setEdgeIfPossible(from, to, cost, cost_2);
    return;
  }

  return returnError("the edge target isn't connected to the index " + targt_indx);
}

void fListUnusedEdges() {
  for (auto& node : graph) {
    for (auto& edge : node.edge) {
      if (!(std::isnan(edge.cost) && std::isnan(graph[edge.indx_to].edge[getEdgeIndex(edge.indx_to, edge.indx_from)].cost))) continue;

      std::cout << node.name << ":\n\t";

      if (edge.indx_to >= graph.size()) std::cout << "invl " << edge.indx_to;
      else std::cout << graph[edge.indx_to].name;

      std::cout << '\n';
    }
  }
}

void fRemoveUnusedEdges() {
  bool is_next_nan = false;
  auto& node = graph[nod_origin_index];
  for (size_t e = 0; e < node.edge.size(); e++) {
    auto& edge = node.edge[e];
    auto& next_edges = graph[edge.indx_to].edge;

    is_next_nan = std::isnan(next_edges[getEdgeIndex(edge.indx_to, edge.indx_from)].cost);
    if (!(std::isnan(edge.cost) && is_next_nan)) continue;

    node.edge.erase(node.edge.begin() + e);
    next_edges.erase(next_edges.begin() + getEdgeIndex(edge.indx_to, nod_origin_index));
  }
}


void fDictionPoint() {
  using pair_name_map = std::pair<std::string, size_t>;

  std::vector<pair_name_map> vec_name_map;
  vec_name_map.reserve(name_map.size());

  for (auto& elem : name_map) {
    vec_name_map.push_back(elem);
  }
  std::sort(vec_name_map.begin(), vec_name_map.end(), [=](pair_name_map& a, pair_name_map& b) { return a.second < b.second; });

  for (auto& a : vec_name_map) {
    std::cout << a.second << ": \"" << a.first << "\"" << '\n';
  }
}


void removeEdgeFromNodes(size_t targt_indx) {
  auto& orign = graph[nod_origin_index].edge;
  auto& targt = graph[targt_indx].edge;

  for (size_t i = 0; i < orign.size(); i++) {
    if (orign[i].indx_from == targt_indx || orign[i].indx_to == targt_indx) {
      orign.erase(orign.begin() + i);
      break;
    }
  }

  for (size_t i = 0; i < targt.size(); i++) {
    if (targt[i].indx_from == nod_origin_index || targt[i].indx_to == nod_origin_index) {
      targt.erase(targt.begin() + i);
      break;
    }
  }
}

void fRemoveEdge() {
  if (graph[nod_origin_index].edge.size() == 1) graph[nod_origin_index].edge.pop_back();
  if (graph[nod_origin_index].edge.empty()) return;

  checkValidArgumentCount(1);
  giveChoiceUseNameAsIndex();

  size_t targt = 0;

  if (inp[0] == 'Y') {
    if ((targt = getNodeIndex()) == -1) return returnError("not found by name");

    removeEdgeFromNodes(targt);

  } else if (inp[0] == 'n') {
    inputNumber(targt);

    if (checkGraphRange(nod_origin_index)) return;

    removeEdgeFromNodes(targt);
  }
}


void fRenamePoint() {
  checkValidArgumentCount(1);
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


  if (exitRet) std::cout << "something went wrong with python script\n";
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
  checkValidArgumentCount(2);
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
  checkValidArgumentCount(2);
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
\tsete - set edge two-sided, only sided and none sided with it's costs (<>, <-, ->, <->, <=>)\n\
\tlstu - list of unused edges (those edges what assigned with <>)\n\
\tremp - remove a node and removing all references to the node (is...)\n\
\treme - remove an edge between origin and a node (from origin to...)\n\
\tremu - remove unused edges in origin\n\
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
\tremf - remove a function\n\
\tseef - output the function body\n\
\tnewv - create a variable\n\
\tremv - remove the variable\n\
\tlstv - list all variables\n\
\trenv - rename a variable\n\
\tsetv - set variable's value with it's data type\n\
\toutv - output a single variable ($ before variable name)\n\
\tincv - increment a single variable as index ($ before variable name)\n\
\tdecv - decrement a single variable as index ($ before variable name)\n\
\tfndk - find node names with a certain keyword (it is sensitive to capitalisations)\n\
\tfnds - find node names with a similar writing (it is sensitive to capitalisations)\n\
\tersl - erase unsavable entries of functions and variables\n\
\tfile - extract commands from file and execute them\n\
\taddv - add between variables ($ before variable name)\n\
\tsubv - subtract between variables ($ before variable name)\n\
\tmulv - multiply between variables ($ before variable name)\n\
\tdivv - divide between variables ($ before variable name)\n\
\texit - exit\n\
\n\
Also exists custom arguments:\n\
\t[--version | -v]  - get a version\n\
\t[-d]        - enter to debug mode (cache wouldn't be erased)\n\
\t[-h | --help]   - shows help for navigating the program\n\
\t[--argument | -a] - enter to argument mode (you can write all commands in arguments separated by space (for names as indexes it recognise automatically))\n\
\t[-ssa]      - you can type your input using a single string argument (f.e. -ssa \"newp A newp B ...\")\n\
\n\
In argument mode:\n\
\t[-i] - alias to \"load\"\n\
\t[-o] - alias to \"save\"\n\
\t[-temp [i | o]] - alias to \"tmpi\" or \"tmpo\"\n\
\n\
Also for functions:\n\
\tFunctions can be with ! at the beginning to signature that they would not make an entry to .func\n\
\tThese functions are not savable\n\
Also for variables:\n\
\t[$(existing variable's name)] - insert variable's value right in to fields \n\
\tWhen you set the variable, you can set from graph values with $_\n\
\tThe variable can not be named with _ at the beginning\n\
\t$_:size - size of the graph\n\
\t$_:origin - origin from where the operation can be passed\n\
\tdefinition: (your index or your variable or the variable name with $) (abbr. IN) - indexing based on node index\n\
\tdefinition: (your index or your variable) (abbr. IE) - indexing based on edge index in a node\n\
\t$_[IN] - index to the node\n\
\t$_[IN][IN] - boolean is the edge between these 2 nodes exists\n\
\t$_[IN][IN]:[to | from | cost] - the property to the edge\n\
\t$_[IN]:edges - amount of edges in the node\n\
\t$_[IN]:edges[IE]:[to | from | cost] - the property to the edge in indexed edges in the node\n\
\n";
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


void fEraseLocalVariables() {
  temp_functions.clear();
  variables.clear();
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

  for (size_t i = 0; i < content.size(); i++) {
    inString ^= (content[i] == c_mark_string);

    if (inString) continue;

    if (isspace(content[i])) {
      if (wasSpace) {
        content.erase(content.begin() + i);
        i--;
      } else content[i] = ' ';
    }
    wasSpace = isspace(content[i]);
  }
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
        counter_beg = counter_end + 2;
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

constexpr std::unordered_map<std::string, std::queue<std::string>>& getFunctionMapC(bool cached = false, bool temporary = false) {
  if (cached) {
    if (temporary) return temp_functions_cache;
    else return functions_cache;
  } else {
    if (temporary) return temp_functions;
    else return functions;
  }
}

inline std::unordered_map<std::string, std::queue<std::string>>& getFunctionMap(bool cached = false, bool temporary = false) {
  if (cached) {
    if (temporary) return temp_functions_cache;
    else return functions_cache;
  }
  else {
    if (temporary) return temp_functions;
    else return functions;
  }
}


inline std::optional<std::pair<bool, std::queue<std::string>>> getQueue() {
  if (inp[0] == c_mark_temp_function) {
    auto iter = temp_functions.find(inp);
    if (iter == temp_functions.end()) {
      iter = temp_functions_cache.find(inp);
      if (iter == temp_functions_cache.end()) return std::make_pair(true, iter->second);
      else return returnError<std::pair<bool, std::queue<std::string>>>("can not find the required function in temporaries");
    } else return std::make_pair(false, iter->second);
  } else {
    auto iter = functions.find(inp);
    if (iter == functions.end()) {
      iter = functions_cache.find(inp);
      if (iter == functions_cache.end()) return std::make_pair(true, iter->second);
      else return returnError<std::pair<bool, std::queue<std::string>>>("can not find the required function");
    } else return std::make_pair(false, iter->second);
  }
}


void dumpToDotFunc() {
  std::ofstream fout;
  fout.open(".func");

  for (auto elem : functions) {
    fout << elem.first << '\t';
    
    const size_t queue_size = elem.second.size();
    auto& queue = elem.second;
    std::vector<std::string> vec_queue;
    vec_queue.reserve(queue_size);

    for (size_t i = 0; i < queue_size - 1; i++) {
      fout << queue.front() << ' ';
      queue.pop();
    }

    fout << queue.front() << "\tinstance\n";
  }
  for (auto elem : functions_cache) {
    fout << elem.first << '\t';

    const size_t queue_size = elem.second.size();
    auto& queue = elem.second;
    std::vector<std::string> vec_queue;
    vec_queue.reserve(queue_size);

    for (size_t i = 0; i < queue_size - 1; i++) {
      fout << queue.front() << ' ';
      queue.pop();
    }

    fout << queue.front() << "cache\n";
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

  while (fin) {
    indexTo = line.find('\t');
    name = line.substr(0, indexTo);
    line.erase(line.begin(), line.begin() + indexTo + 1);

    indexTo = line.find('\t');

    getFunctionMap(line == "cache").insert(std::make_pair(name, pushInpToQueue(line.substr(0, indexTo))));
    line.erase(line.begin(), line.begin() + indexTo + 1);

    std::getline(fin, line);
  }

  fin.close();
}

void fNewFunction() {
  checkValidArgumentCount(2);
  std::queue<std::string> q;
  std::string name;
  bool c = false;

  if (argument_flag.arg_mode) {
    name = arg_input_queue.front();
    arg_input_queue.pop();

    q = pushInpToQueue(arg_input_queue.front());
    arg_input_queue.pop();

    if (!arg_input_queue.empty()) {
      c = arg_input_queue.front() == "cache";
      if (c) arg_input_queue.pop();
    }
  }
  else {
    std::cout << "Choose a name for a function: ";
    std::getline(std::cin >> std::ws, name);

    std::cout << "Insert a logic for the function(as for argument mode): ";
    std::getline(std::cin >> std::ws, inp);
    q = pushInpToQueue(inp);

    std::cout << "Does the function accepts and returns cache?(Y/[n]): ";
    std::cin >> inp;
    c = (inp[0] == 'Y');
  }

  getFunctionMap(c, name[0] == c_mark_temp_function).insert(std::make_pair(name, q));

  if(name[0] != c_mark_temp_function)
    dumpToDotFunc();
}


void fListFunctions() {
  std::cout << "Functions:\n";
  for(auto& elem : getFunctionMapC()) {
    std::cout << "\t -> \"" << elem.first << "\"\n";
  }
  for (auto& elem : getFunctionMapC(false, true)) {
    std::cout << "\t -> \"" << elem.first << "\" (temporary)\n";
  }
  for (auto& elem : getFunctionMapC(true)) {
    std::cout << "\t -> \"" << elem.first << "\" (cached)\n";
  }
  for (auto& elem : getFunctionMapC(true, true)) {
    std::cout << "\t -> \"" << elem.first << "\" (cached, temporary)\n";
  }
}


void fCallFunction() {
  checkValidArgumentCount(1);
  if (argument_flag.arg_mode) {
    inp = arg_input_queue.front();
    arg_input_queue.pop();
  }
  else {
    std::cout << "Function: ";
    std::getline(std::cin >> std::ws, inp);
    argument_flag.arg_mode = true;
  }

  const auto orig_arg_input_queue = arg_input_queue;
  
  auto queue = getQueue();
  if (!queue.has_value()) return;
  arg_input_queue = queue->second;

  if (queue->first) {
    const auto orig_graph = graph;
    const auto orig_name_map = name_map;
    const auto orig_nod_origing_index = nod_origin_index;

    parseCache();
    loopArgumentInputQueue(arg_input_queue.size());
    generateTempCache();

    graph = orig_graph;
    name_map = orig_name_map;
    nod_origin_index = orig_nod_origing_index;
  } else loopArgumentInputQueue(arg_input_queue.size());

  arg_input_queue = orig_arg_input_queue;
  std::cout << '\n';
}


void fRemoveFunction() {
  checkValidArgumentCount(1);
  if (argument_flag.arg_mode) {
    inp = arg_input_queue.front();
    arg_input_queue.pop();
  } else {
    std::cout << "Function: ";
    std::getline(std::cin >> std::ws, inp);
  }

  if (inp[0] == c_mark_temp_function)
    temp_functions.erase(inp);
  else {
    functions.erase(inp);
    dumpToDotFunc();
  }
}


void fLookAtFunction() {
  checkValidArgumentCount(1);
  if (argument_flag.arg_mode) {
    inp = arg_input_queue.front();
    arg_input_queue.pop();
  } else {
    std::cout << "Function: ";
    std::getline(std::cin >> std::ws, inp);
  }

  auto queue = getQueue();
  if (!queue.has_value()) return;

  auto iter = getCommandList().begin();
  size_t var = 0;

  std::cout << "\nFunction body" << (queue->first ? "(work with cache)" : "") << ":";
  while(!queue->second.empty()) {
    iter = getCommandList().find(convertStrToRawInt(queue->second.front()));
    if (iter != getCommandList().end()) {
      var = convertIntToRawStr(iter->first);
      std::cout << "\n\t" << (char*)&var;
    }
    else std::cout << " " << queue->second.front();
    queue->second.pop();
  }
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
  checkValidArgumentCount(1);
  if (!argument_flag.arg_mode) {
    std::cout << "Enter a variable name: ";
    std::getline(std::cin >> std::ws, inp);
  }
  else {
    inp = arg_input_queue.front();
    arg_input_queue.pop();
  }

  if (inp[0] == c_mark_root_graph_name) returnError(std::string("variable is being named like root ('") + c_mark_root_graph_name + "' at beginning of the name) is not supposed to exist");

  variables.insert(std::make_pair(inp, Variable{}));
}


void fRemoveVariable() {
  checkValidArgumentCount(1);
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
  checkValidArgumentCount(2);
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
      //   iter_from
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
      //         if 'iter' side matches with value,then it would be casted approximately,
      //         otherwise it would be casted directly how are they represent
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

size_t getInsideScopeGeneric(size_t& openScopeIndexInp, size_t& closScopeIndexInp) {
  closScopeIndexInp = inp.find_first_of(c_mark_index_scope_cl, ++openScopeIndexInp);
  size_t index = 0;

  if (closScopeIndexInp == -1) return -1;

  if (inp[openScopeIndexInp] == c_mark_variable) { // variable
    openScopeIndexInp++;
    auto iter_from = variables.find(inp.substr(openScopeIndexInp, closScopeIndexInp - openScopeIndexInp));
    if (checkVariableName(iter_from)) return -1;

    index = iter_from->second.value.indx;
  } else if (isdigit(inp[openScopeIndexInp]) != 0) // digit
    index = std::stoull(inp.substr(openScopeIndexInp, closScopeIndexInp - openScopeIndexInp));
  else return -2; // there should be an external value

  openScopeIndexInp = closScopeIndexInp;

  return index;
}

size_t getGraphIndexInsideScopes(size_t& sepFirst) {
  size_t sepSecond;
  size_t index = 0;
  index = getInsideScopeGeneric(sepFirst, sepSecond);

  if (index == -1) return -1;
  if (index != -2) return index;

  sepFirst++;
  auto iter_from = name_map.find(inp.substr(sepFirst, sepSecond - sepFirst));
  if (checkGraphName(iter_from)) return -1;

  sepFirst = sepSecond;

  return index;
}

decltype(variables.begin()) getInputForSetVariable() {
  auto iter = variables.end();
  if (argument_flag.arg_mode) {
    iter = variables.find(arg_input_queue.front());
    arg_input_queue.pop();
    if (checkVariableName(iter)) return returnError(variables.end(), "the variable has no match with the name");

    if (convertStrToRawInt(arg_input_queue.front()) == 'whol') iter->second.type = Variable::whol;
    else if (convertStrToRawInt(arg_input_queue.front()) == 'flot') iter->second.type = Variable::flot;
    else if (convertStrToRawInt(arg_input_queue.front()) == 'indx') iter->second.type = Variable::indx;
    else { arg_input_queue.pop(); return returnError(variables.end(), "the value type wasn't settled"); }
    arg_input_queue.pop();

    inp = arg_input_queue.front();
    arg_input_queue.pop();
  } else {
    std::cout << "Variable name: ";
    std::getline(std::cin >> std::ws, inp);

    iter = variables.find(inp);
    if (checkVariableName(iter)) return returnError(variables.end(), "the variable has no match with the name");

    std::cout << "Type [whol(eq. int) / flot(eq. float) / indx]: ";
    std::cin >> inp;

    if (convertStrToRawInt(inp) == 'whol') iter->second.type = Variable::whol;
    else if (convertStrToRawInt(inp) == 'flot') iter->second.type = Variable::flot;
    else if (convertStrToRawInt(inp) == 'indx') iter->second.type = Variable::indx;
    else return returnError(variables.end(), "the value type wasn't settled");

    std::cout << "Value: ";
    std::cin >> inp;
  }
  return iter;
}

void setVariableFromEdge(size_t& sepFirst, decltype(variables.begin())& iter, size_t indexNod, size_t indexEdg) {
  if (inp[++sepFirst] != c_mark_root_separator) {
    setVariable<size_t>(iter->second, 1); // it can be as the condition
    //std::cout << OutErr << "there's no separator after index scopes\n";
    return;
  }

  constexpr const char* c_str_from = "from";
  constexpr const char* c_str_to = "to";
  constexpr const char* c_str_cost = "cost";

  if (strcmp(inp.data() + sepFirst + 1, c_str_from) == 0)      setVariable(iter->second, graph[indexNod].edge[indexEdg].indx_from);
  else if (strcmp(inp.data() + sepFirst + 1, c_str_to) == 0)   setVariable(iter->second, graph[indexNod].edge[indexEdg].indx_to);
  else if (strcmp(inp.data() + sepFirst + 1, c_str_cost) == 0) setVariable(iter->second, graph[indexNod].edge[indexEdg].cost);
  else return returnError("no variable type isn't matching");
}

void setVariableFromNode(size_t& sepFirst, decltype(variables.begin())& iter, size_t indexNod) {
  // the user wants to get the property of the node
  constexpr const char* c_str_edges = "edges";

  if (strcmp(inp.substr(sepFirst + 1, strlen(c_str_edges)).c_str(), c_str_edges) == 0) {
    sepFirst = inp.find_first_of(c_mark_index_scope_op, sepFirst + 1); // we could be at [
    if (sepFirst == -1) {
      setVariable(iter->second, graph[indexNod].edge.size());
      return;
    }

    size_t s = 0;
    size_t indexEdg = getInsideScopeGeneric(sepFirst, s);

    if (indexEdg == -1 || indexEdg == -2) return returnError("the index isn't in the range of the current edge index");

    setVariableFromEdge(sepFirst, iter, indexNod, indexEdg);
  } else return returnError("the demanding property from the node is unclear");

  return;
}

void setVariableFromGraphProperty(size_t& sepFirst, decltype(variables.begin())& iter) {
  constexpr const char* c_str_origin = "origin";
  constexpr const char* c_str_size = "size";

  sepFirst = inp.find_first_of(c_mark_root_separator, 2);
  if (sepFirst == -1) return returnError("there's no separator after root variable");

  sepFirst++;

  if (strcmp(inp.substr(sepFirst, strlen(c_str_origin)).c_str(), c_str_origin) == 0) setVariable(iter->second, nod_origin_index);
  else if (strcmp(inp.substr(sepFirst, strlen(c_str_size)).c_str(), c_str_size) == 0) setVariable(iter->second, graph.size());
  else return returnError("the demanding property is unclear to get from the graph itself");
}

size_t chechIndexInputFromBracket(size_t& sepFirst) {
  size_t index = getGraphIndexInsideScopes(sepFirst); // sepFirst should be at last ]

  if (index == -1) return returnError(-1, "the index wasn't found or is illegal to set");
  if (checkGraphRange(index)) return returnError(-1, "the index isn't in the range of the graph");
  return index;
}

size_t getIndexToFromBracket(size_t& sepFirst, size_t indexNod) {
  size_t indexTo = chechIndexInputFromBracket(sepFirst); // sepFirst should be at last ]
  if (indexTo == -1) return -1;

  bool is_connected = false;
  size_t indexEdg = -1;

  for (size_t i = 0; i < graph[indexNod].edge.size(); i++) {
    if (graph[indexNod].edge[i].indx_to == indexTo) {
      is_connected = true;
      indexEdg = i;
      break;
    }
  }

  if (!is_connected) {
    if (inp.find_first_of(sepFirst, c_mark_root_separator) != -1) // it can be as the condition
      return returnError(-1, "the index isn't connected to the graph");
    return -2;
  }

  return indexEdg;
}

void setVariableWithBrackets(size_t sepFirst, decltype(variables.begin())& iter) {
  // indexing with various methods
  size_t indexNod = chechIndexInputFromBracket(sepFirst); // sepFirst is at ]
  if (indexNod == -1) return;

  // we need to sure that we get into the properties of the node
  sepFirst = inp.find_first_of(c_mark_root_separator, sepFirst + 1);

  if (sepFirst != -1) {
    setVariableFromNode(sepFirst, iter, indexNod);
    return;
  }

  sepFirst = inp.find_first_of(c_mark_index_scope_op, 4);
  if (sepFirst == -1) { // default: get the index of the node
    setVariable(iter->second, indexNod);
    return;
  }

  // if we went there, that means, that we need edge
  size_t indexEdg = getIndexToFromBracket(sepFirst, indexNod);
  if (indexEdg == -1) return;
  if (indexEdg == -2) {
    setVariable<size_t>(iter->second, 0);
    return;
  }

  setVariableFromEdge(sepFirst, iter, indexNod, indexEdg);
  return;
}

void fSetVariable() {
  checkValidArgumentCount(3);
  auto iter = getInputForSetVariable();
  if (iter == variables.end()) return;

  if(inp[0] != c_mark_variable) {
    switch (iter->second.type) {
    case Variable::whol: iter->second.value.whol = std::stoi(inp);  break;
    case Variable::flot: iter->second.value.flot = std::stof(inp);  break;
    case Variable::indx: iter->second.value.indx = std::stoull(inp); break;
    }
    return;
  }

  // set variable from existing value
  if (inp[1] != c_mark_root_graph_name) {
    setVariableFromVariable(iter->second, variables.find(inp.substr(1))->second);
    return;
  }

  // variable manipulation
  size_t sepFirst = inp.find_first_of(c_mark_index_scope_op, 2);

  if (sepFirst != -1) {
    setVariableWithBrackets(sepFirst, iter);
    return;
  }
  
  // there has to be a separator to interact with graph properties
  setVariableFromGraphProperty(sepFirst, iter);
  return;
}


void fOutputVariable() {
  checkValidArgumentCount(1);
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
  checkValidArgumentCount(1);
  auto iter = getVariableInstance();
  if (checkVariableName(iter)) return;

  iter->second.value.indx++;
}

void fDecrementVariable() {
  checkValidArgumentCount(1);
  auto iter = getVariableInstance();
  if (checkVariableName(iter)) return;

  iter->second.value.indx--;
}


std::optional<std::pair<decltype(variables.begin()), decltype(variables.begin())>> inputItersForOperations(const char* arg_to_operator) {
  if (!argument_flag.arg_mode) std::cout << "Variable to be " << arg_to_operator << ": ";
  auto iterin = getVariableInstance();
  if (checkVariableName(iterin)) return;

  if (!argument_flag.arg_mode) std::cout << "Variable from: ";
  auto iterfrom = getVariableInstance();
  if (checkVariableName(iterfrom)) return;

  return std::make_pair(iterin, iterfrom);
}

void fAddVariable() {
  checkValidArgumentCount(2);
  auto opers = inputItersForOperations("added");
  if (!opers.has_value()) return;

  auto& var = opers->first->second;
  const auto& varFrom = opers->second->second;

  switch (var.type) {
  case Variable::whol:
    switch (varFrom.type) {
    case Variable::whol: var.value.whol += varFrom.value.whol; break;
    case Variable::flot: var.value.whol += varFrom.value.flot; break;
    case Variable::indx: var.value.whol += varFrom.value.indx; break;
    } break;
  case Variable::flot:
    switch (varFrom.type) {
    case Variable::whol: var.value.flot += varFrom.value.whol; break;
    case Variable::flot: var.value.flot += varFrom.value.flot; break;
    case Variable::indx: var.value.flot += varFrom.value.indx; break;
    } break;
  case Variable::indx:
    switch (varFrom.type) {
    case Variable::whol: var.value.indx += varFrom.value.whol; break;
    case Variable::flot: var.value.indx += varFrom.value.flot; break;
    case Variable::indx: var.value.indx += varFrom.value.indx; break;
    } break;
  }
}

void fSubVariable() {
  checkValidArgumentCount(2);
  auto opers = inputItersForOperations("subtracted");
  if (!opers.has_value()) return;

  auto& var = opers->first->second;
  const auto& varFrom = opers->second->second;

  switch (var.type) {
  case Variable::whol:
    switch (varFrom.type) {
    case Variable::whol: var.value.whol -= varFrom.value.whol; break;
    case Variable::flot: var.value.whol -= varFrom.value.flot; break;
    case Variable::indx: var.value.whol -= varFrom.value.indx; break;
    } break;
  case Variable::flot:
    switch (varFrom.type) {
    case Variable::whol: var.value.flot -= varFrom.value.whol; break;
    case Variable::flot: var.value.flot -= varFrom.value.flot; break;
    case Variable::indx: var.value.flot -= varFrom.value.indx; break;
    } break;
  case Variable::indx:
    switch (varFrom.type) {
    case Variable::whol: var.value.indx -= varFrom.value.whol; break;
    case Variable::flot: var.value.indx -= varFrom.value.flot; break;
    case Variable::indx: var.value.indx -= varFrom.value.indx; break;
    } break;
  }
}

void fMulVariable() {
  checkValidArgumentCount(2);
  auto opers = inputItersForOperations("multiply");
  if (!opers.has_value()) return;

  auto& var = opers->first->second;
  const auto& varFrom = opers->second->second;

  switch (var.type) {
  case Variable::whol:
    switch (varFrom.type) {
    case Variable::whol: var.value.whol *= varFrom.value.whol; break;
    case Variable::flot: var.value.whol *= varFrom.value.flot; break;
    case Variable::indx: var.value.whol *= varFrom.value.indx; break;
    } break;
  case Variable::flot:
    switch (varFrom.type) {
    case Variable::whol: var.value.flot *= varFrom.value.whol; break;
    case Variable::flot: var.value.flot *= varFrom.value.flot; break;
    case Variable::indx: var.value.flot *= varFrom.value.indx; break;
    } break;
  case Variable::indx:
    switch (varFrom.type) {
    case Variable::whol: var.value.indx *= varFrom.value.whol; break;
    case Variable::flot: var.value.indx *= varFrom.value.flot; break;
    case Variable::indx: var.value.indx *= varFrom.value.indx; break;
    } break;
  }
}

void fDivVariable() {
  checkValidArgumentCount(2);
  auto opers = inputItersForOperations("divided");
  if (!opers.has_value()) return;

  auto& var = opers->first->second;
  const auto& varFrom = opers->second->second;

  switch (var.type) {
  case Variable::whol:
    switch (varFrom.type) {
    case Variable::whol: var.value.whol /= varFrom.value.whol; break;
    case Variable::flot: var.value.whol /= varFrom.value.flot; break;
    case Variable::indx: var.value.whol /= varFrom.value.indx; break;
    } break;
  case Variable::flot:
    switch (varFrom.type) {
    case Variable::whol: var.value.flot /= varFrom.value.whol; break;
    case Variable::flot: var.value.flot /= varFrom.value.flot; break;
    case Variable::indx: var.value.flot /= varFrom.value.indx; break;
    } break;
  case Variable::indx:
    switch (varFrom.type) {
    case Variable::whol: var.value.indx /= varFrom.value.whol; break;
    case Variable::flot: var.value.indx /= varFrom.value.flot; break;
    case Variable::indx: var.value.indx /= varFrom.value.indx; break;
    } break;
  }
}


void updateMatrixSizeLev(std::vector<size_t>& matrix, size_t& width, size_t& height, const size_t origSize, const size_t checkSize) {
  size_t i = 0;
  
  if (width < origSize || height < checkSize) {
    if (width < origSize) width = origSize;
    if (height < checkSize) height = checkSize;

    matrix.resize((width + 1) * (height + 1));
    for (i = 0; i < width + 1; i++) // fill by width
      matrix[i] = i;
    for (i = 0; i < height + 1; i++) // fill by height
      matrix[width * i] = i;
  }
}

// Wagner–Fischer algorithm
size_t countErrorsInStringLev(const std::string& orig, const std::string& check) {
  // all static to not spend time on pushing temporary variables in stack and then poping, dynamic programming anyway
  static std::vector<size_t> matrix;
  static size_t width = 0;
  static size_t height = 0;
  static size_t x;
  static size_t y;
  static bool subst;

  updateMatrixSizeLev(matrix, width, height, orig.size(), check.size());

  for (x = 0; x < orig.size(); x++) {
    for (y = 0; y < check.size(); y++) {
      subst = orig[x] != check[y];
      matrix[(x + 1) + (y + 1) * width] = std::min(matrix[(x) + (y + 1) * width] + 1, std::min(matrix[(x + 1) + (y) * width] + 1, matrix[x + y * width] + subst));
    }
  }
  return matrix[x + y * width];
}

auto searchNearest() {
  inputNodeName();

  auto low = name_map.end();

  size_t currentCount;

  for (auto elem = name_map.begin(); elem != name_map.end(); elem++) {
    currentCount = countErrorsInStringLev(elem->first, inp);
    if (currentCount == 0) return elem;
    if (low->second > currentCount) low = elem;
  }

  return low;
}

void outputSearchMap(size_t(*comp)(const std::string& orig, const std::string& check)) {
  inputNodeName();

  std::vector<size_t> ratelist;
  std::vector<decltype(name_map.begin())> indxlist;

  ratelist.reserve(name_map.size());
  indxlist.reserve(name_map.size());

  size_t currentCount;
  size_t target = 0;

  for (auto elem = name_map.begin(); elem != name_map.end(); elem++) {
    currentCount = comp(elem->first, inp);

    // part of insertion sort
    for (target = 0; target < ratelist.size(); target++) {
      if (currentCount > ratelist[target]) {
        ratelist.insert(ratelist.begin() + target, currentCount);
        indxlist.insert(indxlist.begin() + target, elem);
        break;
      }
    }

    if (target >= ratelist.size()) {
      ratelist.push_back(currentCount);
      indxlist.push_back(elem);
    }
  }

  if (ratelist.size() == 0) {
    std::cout << "empt\n";
    return;
  }
  std::cout << '\n';
  for (size_t i = 0; i < std::min<size_t>(ratelist.size(), 20); i++) {
    if (ratelist[ratelist.size() - i - 1] == -1) return;
    std::cout << indxlist[ratelist.size() - i - 1]->first << '(' << ratelist[ratelist.size() - i - 1] << "): " << indxlist[ratelist.size() - i - 1]->second << '\n';
  }
}

size_t findKeyword(const std::string& orig, const std::string& check) {
  return orig.find(check);
}

void fSearchKeyword() {
  checkValidArgumentCount(1);
  outputSearchMap(findKeyword);
}

void fSearchSimilarMap() {
  checkValidArgumentCount(1);
  outputSearchMap(countErrorsInStringLev);
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


const std::map<int,void (*)()> c_command_list = {
  {'newp',fNewPoint           },
  {'newe',fNewEdge            },
  {'seto',fSetOrigin          },
  {'sete',fSetEdge            },
  {'remp',fRemovePoint        },
  {'reme',fRemoveEdge         },
  {'lstu',fListUnusedEdges    },
  {'remu',fRemoveUnusedEdges  },
  {'renm',fRenamePoint        },
  {'list',fList               },
  {'dict',fDictionPoint       },
  {'save',fSaveGraph          },
  {'load',fLoadGraph          },
  {'clir',fClear              },
  {'help',fHelp               },
  {'rset',fReset              },
  {'lsta',fListAll            },
  {'tmpi',parseCache          },
  {'tmpo',generateTempCache   },
  {'newf',fNewFunction        },
  {'lstf',fListFunctions      },
  {'call',fCallFunction       },
  {'reff',refreshFromDotFunc  },
  {'remf',fRemoveFunction     },
  {'seef',fLookAtFunction     },
  {'newv',fNewVariable        },
  {'remv',fRemoveVariable     },
  {'lstv',fListVariables      },
  {'renv',fRenameVariable     },
  {'setv',fSetVariable        },
  {'incv',fIncrementVariable  },
  {'decv',fDecrementVariable  },
  {'outv',fOutputVariable     },
  {'ersl',fEraseLocalVariables},
  {'fndk',fSearchKeyword      },
  {'fnds',fSearchSimilarMap   },
  {'file',fFile               },
  {'addv',fAddVariable        },
  {'subv',fSubVariable        },
  {'mulv',fMulVariable        },
  {'divv',fDivVariable        },
  {'exit',nullptr             }
};

decltype(c_command_list)& getCommandList() { return c_command_list; }


int init(const int args, const char* argv[], size_t& arg_count) {
  argument_flag.need_to_override_when_halts = false;
  argument_flag.halt = false;
  argument_flag.exit = false;

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
      argument_flag.need_to_override_when_halts = true;
      arg = argv[++arg_count];
    } else if (strcmp(arg, "-f") == 0 || strcmp(arg, "--file") == 0) {
      extractFromFileToQueue(arg_count);
      argument_flag.arg_mode = true;
      arg = argv[++arg_count];
    }
  }


  if (isPythonMissing()) return returnError(1, "python is missing in the enviroument");
  refreshFromDotFunc();

  return 0;
}

void executeIntCommand(int input) {
  try {
    auto command = c_command_list.find(input);
    if (command == c_command_list.end()) {
      std::cout << "invl\n";
      return;
    }
    if (command->second) command->second();
    else argument_flag.exit = true;
  } catch (const std::exception e) {
    std::cout << "excp: " << e.what() << '\n';
  }
}

void loopArgumentInputQueue(size_t arg_queue_size) {
  auto was = arg_input_queue.size();
  while (arg_input_queue.size() && !argument_flag.halt && !argument_flag.exit) {
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
      const int num = convertStrToRawInt(arg_input_queue.front());
      arg_input_queue.pop();
      executeIntCommand(num);
    }
  }
  if (argument_flag.halt) {
    if (argument_flag.need_to_override_when_halts) enterManualMode();
    else argument_flag.arg_mode = false;
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
  while (!argument_flag.exit) {
    argument_flag.halt = false;
    std::cout << "|";
    if (nod_origin_index < graph.size())
      std::cout << '"' << graph[nod_origin_index].name.c_str() << '"';
    else
      std::cout << "[null]";

    std::cout << " inp: ";
    std::getline(std::cin >> std::ws, inp);

    if(inp.find(' ') == inp.npos)
      executeIntCommand(convertStrToRawInt(inp));
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
