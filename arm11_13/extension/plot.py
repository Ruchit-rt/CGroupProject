#Requirement - pip install networkx[default]
import networkx as nx
import matplotlib
import matplotlib.pyplot as plot
matplotlib.use('Agg')

lst1 = []
lst2 = []

with open('out', 'r') as f:
    serialisable = f.readline().upper()
    for line in f:
        line = line.strip().split(',') 
        lst1.append(line[0])
        lst2.append(line[1])

tuple_list = list(zip(lst1, lst2))
if (len(tuple_list) == 0):
    serialisable += " (no conflicts found)"

G = nx.DiGraph()
G.add_edges_from(tuple_list)
plot.figure(figsize =(5, 5))
nx.draw_networkx(G, with_labels = True, node_size = 1000, node_color ='yellow', width = 3)
plot.title(serialisable, fontsize=12)
plot.savefig('graph.pdf')
plot.show()