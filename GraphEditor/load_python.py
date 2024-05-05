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
def load():
    try:
        file = open(f"{sys.argv[1]}.grf", 'r')
    except:
        print("file wasn't found")
        exit(1)

    ret = ast.literal_eval(file.readline())
    file.close()

    return ret



graph = load()

insert_to_main()

