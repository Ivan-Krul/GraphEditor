import sys
import ast
import os

if len(sys.argv) < 2:
    print("missing filename argument")
    exit(1)

def extract_graph():
    file_graph = open("graph", 'r')

    ret = ast.literal_eval(file_graph.readline())

    file_graph.close()

    return ret

# could be overrided
def save():
    file = open(f"{sys.argv[1]}.grf", 'w')
    file.write(str(graph))
    file.close()



graph = extract_graph()
save()
os.remove("graph")


