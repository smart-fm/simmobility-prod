import pandas as pd
from collections import defaultdict

def check_sink_nodes():
    inFolder = 'Outputs/Test/simmobility/'
    nodes = pd.read_csv(inFolder + 'node-nodes.csv')
    links = pd.read_csv(inFolder + 'link-attributes.csv')
    print(links.columns)
    print(nodes.columns)
    outgoing = defaultdict(int)
    incoming = defaultdict(int)

    for indx, row in links.iterrows():
        outgoing[row.from_node] +=1
        incoming[row.to_node] +=1

    nodes['num_outgoing'] = nodes.apply(lambda row: outgoing[row.id], axis=1)
    print('num sink nodes ', len(nodes))
    nodes = nodes[nodes.num_outgoing == 0]
    print('\n\n')
    print(set(nodes['id'].tolist()))
    print('\n\n')
    print('num sink nodes ', len(nodes))
check_sink_nodes()
