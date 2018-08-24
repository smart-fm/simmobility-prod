# -*- coding: utf-8 -*-
"""
Created on Tue Jul 24 14:45:20 2018

@author: chuyaw

Mapping postcode to nearest valid node (valid node, meant nodes except sink/source node, closed node, loop node, bus terminus node)
1. Provide database connection, table_NodeToZone, table_Node
2. Provide input postcode (infilename_postcode_transformed)
"""

import pandas as pd
import numpy as np
import time
from sklearn.neighbors import KDTree

import psycopg2
import psycopg2.extras
import pandas.io.sql as sqlio

table_NodeToZone = 'demand.node_taz_map_new'
table_Node = 'supply_jurong2.node'

infilename_postcode_transformed = 'sla_address_transformed.csv'
postcode_to_node_filesname = 'postcode_to_nereastnode.csv'

def main():
    #Define our connection string
    conn_string = "host='172.25.184.156' dbname='simmobility_l2nic2b' user='postgres' password='d84dmiN'"
    
    # print the connection string we will use to connect
    print ("Connecting to database\n	->%s" % (conn_string))
    
    # get a connection, if a connect cannot be made an exception will be raised here
    conn = psycopg2.connect(conn_string)
    
    # loading valid node
    df_validnode = loadtable_node(conn)
    # Loading postcode
    df_postcode = pd.read_csv(infilename_postcode_transformed)
    
    # find the nearest postcode from node
    tBegin = time.time()
    df_postcode = find_nearest_node(df_validnode,df_postcode)
    tEnd = time.time()
    print 'Completed mapping postcode to nearest node in ', (tEnd - tBegin), ' seconds '
    
    # export to csv file. 
    cols_to_keep = ['sla_postcode','postcode_zone','nearest_node','node_zone','nearest_node_dist']
    df_postcode[cols_to_keep].to_csv(postcode_to_node_filesname, index = False)
        
    conn.close()

def find_nearest_node(df_validnode,df_postcode):
    array_validnode = df_validnode[['x', 'y']].values
    # building valid node tree map
    print 'Shape of the numpy Array:', array_validnode.shape
    print 'Building the KD Tree of Node'
    tBegin = time.time()
    validnode_tree = KDTree(array_validnode, leaf_size= 2, metric='euclidean')
    tEnd = time.time()
    print 'K-D tree built in ', (tEnd - tBegin), ' seconds '
    
    nearNodeList = []
    nearNodeDistList = []
    
    for index, row in df_postcode.iterrows():
        x = row['x_coord']
        y = row['y_coord']
        # find nearest node from each postcode, output: distance from postcode to node, node's index
        [dist, ind] = validnode_tree.query([[x, y]], k=1)
        # Based on index, find the node
        nearNode = df_validnode.loc[int(ind[0][0])]['node_id']
        nearNode_zone = df_validnode.loc[int(ind[0][0])]['node_zone']
        nearDist = dist[0][0]
        nearNodeList.append(nearNode)
        nearNodeDistList.append(nearDist)
    df_postcode['nearest_node'] = nearNodeList
    df_postcode['node_zone'] = nearNode_zone
    df_postcode['nearest_node_dist'] = nearNodeDistList

    return df_postcode

def loadtable_node(conn):
    # Load Table: valid node to zone (valid: not sink, source, closed, loop node)
    sql_ValidNodeToZone = """SELECT node_id, taz as node_zone, x, y FROM """+table_NodeToZone+""" a
    LEFT JOIN """+table_Node+""" b ON a.node_id = b.id
    WHERE a.node_type NOT IN (1,8,9) and bus_terminus_node IS FALSE
    """
    df_validnode = sqlio.read_sql_query(sql_ValidNodeToZone, conn)
    
    return df_validnode
    
if __name__ == "__main__":
    main()
    
