# -*- coding: utf-8 -*-
"""
Created on Wed May 16 14:28:05 2018
Tool: Find the sink/source and closed nodes (unreachable nodes)

Steps:
    1. Provides database connection. 
    2. studyArea = True or False:
        False (full network will be loaded, output: node_new.csv)
            - provides: table_node, Storedprocedure_link, Storedprocedure_turninggroup
        True (study area network will be loaded, output: blackisted_node.csv)
            - provides: Storedprocedure_StudyArea_node, Storedprocedure_StudyArea_link, Storedprocedure_StudyArea_turninggroup
            
"""

import psycopg2
import psycopg2.extras
import pandas as pd
#from tabulate import tabulate
import pandas.io.sql as sqlio

import sys, os
sys.path=os.getcwd()

studyArea = False

## Full Network
table_node = 'supply.node'
Storedprocedure_link = 'get_links()'
Storedprocedure_turninggroup = 'get_turning_groups()'
## Study Area
Storedprocedure_StudyArea_node = 'get_jurong_nodes()'
Storedprocedure_StudyArea_link = 'get_jurong_links()'
Storedprocedure_StudyArea_turninggroup = 'get_jurong_turning_groups()'


def main():
    
    #Define our connection string
    conn_string = "host='172.25.184.156' dbname='simmobility_l2nic3b' user='postgres' password='d84dmiN'"
    
    # print the connection string we will use to connect
    print ("Connecting to database\n	->%s" % (conn_string))
    
    # get a connection, if a connect cannot be made an exception will be raised here
    conn = psycopg2.connect(conn_string)
    
    if studyArea == False:
        input_nodes,input_link,input_turninggroup = loadtable_FullNetowork(conn)
    else:
        input_nodes,input_link,input_turninggroup = loadtable_StudyAreaNetowork(conn)

    # Find the sink / source nodes. which have no turning group
    sinksourceNode = input_nodes[~input_nodes.id.isin(input_turninggroup.node_id)].id
    print('SinkSourceNode :', len(sinksourceNode))
    print(sinksourceNode.tolist())
     
    ## removes sinksourceNode (node)
    print('\nAfter remove sink source node  \n-------------- ')
    nodes = input_nodes[~input_nodes.id.isin(sinksourceNode)]
    print('Node: ',len(nodes))
    ## removes sinksourceNode (link)
    links = input_link[input_link.from_node.isin(nodes.id) & input_link.to_node.isin(nodes.id)]
    print('Link: ',len(links))
    ## remove sinksourceNode (turning group)
    turninggroup = input_turninggroup[input_turninggroup.from_link.isin(links.id) & input_turninggroup.to_link.isin(links.id)]
    print('Turning group: ',len(turninggroup))
    

    closed_link = []
    newfound_closedlink = []
    i = 0
    
    print('\nRemoving closed link \n--------------  ')
    while i == 0 or len(newfound_closedlink) > 0:
        print('\nClosed link: ', closed_link)
        newfound_closedlink = []
        # removes closed link
        links = links[~links.id.isin(closed_link)]
        print('Link: ',len(links))
        # removes closed node
        nodes = nodes[nodes.id.isin(links.from_node) | nodes.id.isin(links.to_node)]
        print('Node: ',len(nodes))        
        # removes closed node (turninggroup)
        turninggroup = turninggroup[turninggroup.from_link.isin(links.id) & turninggroup.to_link.isin(links.id)]
        print('Turning group: ',len(turninggroup))
        
        # Find the closed link
        # removes closed link
        newfound_closedlink = links[~links.id.isin(turninggroup.from_link) | ~links.id.isin(turninggroup.to_link)].id

        for l in newfound_closedlink.tolist():
            closed_link.append(l)
        print('Found new closed link: ',len(newfound_closedlink))
        
        i +=1
    
    print('\nFinal, found \n--------------  ')
    # list closed node
    closenodelist = input_nodes[~input_nodes.id.isin(sinksourceNode) & ~input_nodes.id.isin(nodes.id)].id
    print('Closed node: ',len(closenodelist),' ',closenodelist.tolist())
    print('Closed link: ',len(closed_link),' ',closed_link)
    
    if studyArea == False:
        newNodefilesname = 'nodes_new.csv'
        input_nodes.loc[input_nodes.id.isin(sinksourceNode),'node_type'] =1
        input_nodes.loc[input_nodes.node_type == 1 & ~input_nodes.id.isin(sinksourceNode),'node_type'] =0              
        input_nodes.loc[input_nodes.id.isin(closenodelist),'node_type'] =9

        # export to csv file
        input_nodes.to_csv(newNodefilesname, index = False)
        print('\nNew node file was generated: ',newNodefilesname)
    else:
        blackListedNode = 'blacklisted_node.csv'
        closenodelist.to_csv(blackListedNode, index=False, header=True)
        print('\nFile was generated: ',blackListedNode)

def loadtable_StudyAreaNetowork (conn):
    
    # Load Table: Node
    sql_node = """SELECT * FROM """ + Storedprocedure_StudyArea_node + """ """
    input_nodes = sqlio.read_sql_query(sql_node, conn)
    
    # Load Table: Link
    sql_link = """SELECT * FROM """ + Storedprocedure_StudyArea_link + """ """
    input_link = sqlio.read_sql_query(sql_link, conn)
    
    # Load Table: Turning group
    sql_tg = """SELECT * FROM """ + Storedprocedure_StudyArea_turninggroup + """  """
    input_turninggroup = sqlio.read_sql_query(sql_tg, conn)
    
    return input_nodes,input_link,input_turninggroup

def loadtable_FullNetowork(conn):
    
    # Load Table: Node
    sql_node = """SELECT * FROM """ + table_node + """ """
    input_nodes = sqlio.read_sql_query(sql_node, conn)
    
    # Load Table: Link
    sql_link = """SELECT * FROM """ + Storedprocedure_link + """ """
    input_link = sqlio.read_sql_query(sql_link, conn)
    
    # Load Table: Turning group
    sql_tg = """SELECT * FROM """ + Storedprocedure_turninggroup + """  """
    input_turninggroup = sqlio.read_sql_query(sql_tg, conn)
    
    return input_nodes,input_link,input_turninggroup        

if __name__ == "__main__":
    main()
