import sys
import ast
import os

graph = []

if len(sys.argv) < 2:
    print("missing filename argument")
    exit(1)

def insert_to_main():
    file_graph = open("graph", 'w')

    file_graph.write(f"{str(len(graph))} ")


    for node in graph:
        file_graph.write(f"{len(node['name'])} {node['name']}{str(len(node['edge']))} ")

        for edge in node['edge']:
            file_graph.write(f"{edge['indx_from']} {edge['indx_to']} {edge['cost']} ")

    file_graph.close()

# could be overrided
def read2files():
    arr = []
    try:
        file = open(f"Nodes {sys.argv[1]}.grf", 'r')
    except:
        print("node file wasn't found")
        exit(1)

    arr.append(file.readlines())
    file.close()

    try:
        file = open(f"{sys.argv[1]}.grf", 'r')
    except:
        print("main file wasn't found")
        exit(1)

    arr.append(file.readlines())
    file.close()

    return arr

def load():
    files = read2files()
    ret = []

    for node_name in files[0]:
        ret.append({'name': node_name.replace('\n', ''), 'edge': []})

    for edge in files[1]:
        splited = edge.split('\t')

        if int(splited[0]) < len(ret):
            ret[int(splited[0])]['edge'].append({'indx_from': int(splited[0]), 'indx_to': int(splited[1]), 'cost': float(splited[2])})

    return ret

graph = load()

insert_to_main()

