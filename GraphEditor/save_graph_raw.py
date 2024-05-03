import sys

if len(sys.argv) < 2:
    print("not [filename argument]")
    exit(1)

file_graph = open("graph", 'r')

state = 0
graph = []

graph_size = int(file_graph.readline())
for i in range(graph_size):
    node = {"name": file_graph.readline(), "edge":[]}
    
    node["name"] = node["name"].rstrip(node["name"][-1])

    edges_size = int(file_graph.readline())

    for j in range(edges_size):
        nod_to = int(file_graph.readline())
        cost = float(file_graph.readline())

        edgie = {"indx_from": i, "indx_to": nod_to, "cost": cost}

        node["edge"].append(edgie)

    graph.append(node)

print(graph)

