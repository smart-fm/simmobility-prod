# -*- coding: utf-8 -*-
"""
Created on Mon Jul 30 11:01:57 2018

@author: chuyaw

(Optional)
Assigning postcode to surround valid node (valid node, meant nodes except sink/source node, closed node, loop node, bus terminus node)
Why?
Noticed that some nodes having congestion (a lot of activities happenning at the node). Need to spread some postcodes to another nearby node. 

How?
- Based on the number of people living or working, estimating the number of activity of each postcode. 
- From the mapping of postcode to nearest node, estimate the total number of activity of each node.
- Reviews the estimated result, set the capacity of highactivityrate in a node. [Default Singapore = 100000]
- Prepare the mapping of postcode to near nodes [Default : 40 near nodes]
- (For each highactivityrate node). Reallocate postcode to surround nodes randomly. 
    -- check 1: not assigning to the high activity rate node.
    -- check 2: total estimated activity of a node that not more than MaxCapacity after assigning. [Default MaxCapacity  = 120000]

Steps:
1. Provide database connection
2. Provide tables name of table_individual_by_id, table_sla_addresses, table_NodeToZone, tabel_Node
3. Provide postcode_to_nearestnode, infilename_postcode_transformed, outfilename_postcode_to_node
4. Optional: Free to adjust 1) the capacity of highactivityrate of node, 2) number of near nodes to be mapping from postcode, 3) MaxCapacity of new assigned node afer assigning. 

Note: if you are getting error below:
'maximum recursion depth exceeded while calling a Python object'
Please adjust the 3 values mentioned above (in step 4) to higher value. 

"""
import pandas as pd
import numpy as np
import time
from sklearn.neighbors import KDTree
import random
import psycopg2
import psycopg2.extras
import pandas.io.sql as sqlio
import sys

# sql_get_totalactiveppl_in_postcode
table_individual_by_id = 'synpop30.table_individual_by_id_for_preday_wo_maid_construction_workers'
table_sla_addresses = 'synpop30.sla_addresses'
table_NodeToZone = 'demand.node_taz_map_testing'
tabel_Node = 'supply_jurong.node'

postcode_to_nearestnode = 'postcode_to_nereastnode.csv'
#postcode_to_nearestnode = 'postcode_to_node_spread.csv'
infilename_postcode_transformed = 'sla_address_transformed.csv'
outfilename_postcode_to_node = 'postcode_to_node_yeiiir.csv'


def main():
    random.seed(1)
    
    #Define our connection string
    conn_string = "host='172.25.184.156' dbname='simmobility_l2nic2b' user='postgres' password='d84dmiN'"
    
    # print the connection string we will use to connect
    print ("Connecting to database\n	->%s" % (conn_string))
    
    # get a connection, if a connect cannot be made an exception will be raised here
    conn = psycopg2.connect(conn_string)
    
    # loads valid node
    df_validnode = loadtable_node(conn)
    # Loading postcode
    df_postcode = pd.read_csv(infilename_postcode_transformed)
    # Loading postcode to existing nearest node
    df_postcode_to_1node = pd.read_csv(postcode_to_nearestnode)
    # Loads sql get total active people in each postcode
    df_pplcount_inpostcode = sql_total_activeppl_in_postcode(conn)
    
    
    # from postcode to nearest node, count the estimated number of acitivity of each node
    # find the high activity rate node and postcode
    highactivityrate, df_node_withhighactivityrate, df_postcode_mapped2_highactivityNode, df_node_estimatedactivity = find_highactivityrate_postcodeNode(df_postcode_to_1node, df_pplcount_inpostcode)
    
    '''
    #!!!!!!!!! run this to check the output file, change the input file (postcode_to_nearestnode), comment below script!!!!!!!!!
    print(df_node_withhighactivityrate)
    df_node_withhighactivityrate.to_csv('checking.csv')
    '''
    # postcode that need to be processed : assign to other node
    postcodeindex_tobeprocessed = []
    # postcode that do not need to re-assign node. keep the nearest node
    postcodeindex_keepthesame = []
    
    highActivity_NodeList = []
    
    # counting the sum of activity of each node until the capacity
    for index, row in df_node_withhighactivityrate.iterrows():
        highActivity_Node = int(row['nearest_node1'])
        highActivity_NodeList.append(highActivity_Node)
        dfp = df_postcode_mapped2_highactivityNode.loc[(df_postcode_mapped2_highactivityNode['nearest_node1'] == highActivity_Node) & df_postcode_mapped2_highactivityNode['total_ppl_x'] > 0]
        dfp_random = dfp.sample(frac=1,random_state=1)
        a = 0
        ActivityCap = highactivityrate
        
        for index, row in dfp_random.iterrows():
            count_activity = row['total_ppl_x']
            if a >= ActivityCap:
                postcodeindex_tobeprocessed.append(int(index))
                
            else:
                postcodeindex_keepthesame.append(int(index))
                a += count_activity
    df_postcode_tobeprocessed = df_postcode_mapped2_highactivityNode.loc[postcodeindex_tobeprocessed]
    df_postcode_tobeprocessed = pd.merge(df_postcode_tobeprocessed,df_postcode, left_on ='sla_postcode', right_on='sla_postcode', how='left')

    # find N nearest node from these postcode
    # adjust this
    N_nearest = 40
    tBegin = time.time()
    ProcessedPostcode_to_Nnodes = find_nearestN_node(df_validnode,df_postcode_tobeprocessed,N_nearest)
    tEnd = time.time()
    print 'Searched near nodes :', (tEnd - tBegin), ' seconds '
    
    tBegin = time.time()
    # adding new column, set default value first
    df_postcode_tobeprocessed['new_nearnode'] = 0
    df_postcode_tobeprocessed['new_nearDist'] = 0
    #df_postcode_tobeprocessed.to_csv('kgjg.csv')
    
    # random mapping postcode to near node, with avoiding high activity rate node
    highActivity_NodeSet =set(highActivity_NodeList)
    tBegin = time.time()
    df_postcode_tobeprocessed = postcode_mapTo_nearNode(highActivity_NodeSet, df_postcode_tobeprocessed, ProcessedPostcode_to_Nnodes, df_node_estimatedactivity, highactivityrate)
    tEnd = time.time()
    print 'Postcodes mapped to near nodes :', (tEnd - tBegin), ' seconds '
    
    
    
    # last, joining data (processed data and unprocessed data) and export 
    tBegin = time.time()          
    df_NewPostcodeToNode = pd.merge(df_postcode_to_1node, df_postcode_tobeprocessed, left_on ='sla_postcode', right_on='sla_postcode', how='left')
    # update new node id, postcode to new node distance
    dfp = df_NewPostcodeToNode.loc[df_NewPostcodeToNode.new_nearnode.notnull()]
    for index, row in dfp.iterrows():
        df_NewPostcodeToNode.nearest_node_x.iloc[index] = df_NewPostcodeToNode.new_nearnode.iloc[index]
        df_NewPostcodeToNode.nearest_node_dist_x.iloc[index] = df_NewPostcodeToNode.new_nearDist.iloc[index]
    # update new node zone
    df_NewPostcodeToNode = pd.merge(df_NewPostcodeToNode, df_validnode, left_on = 'nearest_node_x', right_on ='node_id', how='left')
    df_NewPostcodeToNode.node_zone_x = df_NewPostcodeToNode.node_zone
    cols_to_export = ['sla_postcode', 'postcode_zone', 'nearest_node_x', 'node_zone_x', 'nearest_node_dist_x']
    df_NewPostcodeToNode.to_csv(outfilename_postcode_to_node, columns = cols_to_export, index = False)
    tEnd = time.time()
    print 'Exported output file :', (tEnd - tBegin), ' seconds '
    
    conn.close()


def postcode_mapTo_nearNode (highActivity_NodeSet, df_postcode_tobeprocessed, ProcessedPostcode_to_Nnodes, df_node_estimatedactivity, highactivityrate):
    node_estimatedactivity_dict = {}
    for index, row in df_node_estimatedactivity.iterrows():
        nodeid = index
        totalppl = row['total_ppl']
        node_estimatedactivity_dict[nodeid] = totalppl
        
    for i in highActivity_NodeSet:
        highActivity_Node = i
        dfp = df_postcode_tobeprocessed.loc[(df_postcode_tobeprocessed['nearest_node1'] == highActivity_Node)]
        
        for index, row in dfp.iterrows():
            postcode = row['sla_postcode']
            postcode_activity = row['total_ppl_x']
            
            new_postcode_and_dist = random_nextnode (ProcessedPostcode_to_Nnodes[postcode], highActivity_NodeSet, postcode_activity, node_estimatedactivity_dict, highactivityrate)
            #print(new_postcode_and_dist)
            newnode = new_postcode_and_dist[0]
            new_Dist = new_postcode_and_dist[1]
            df_postcode_tobeprocessed.new_nearnode.iloc[index] = newnode
            df_postcode_tobeprocessed.new_nearDist.iloc[index] = new_Dist
    
    return df_postcode_tobeprocessed

def random_nextnode(ps_to_node_dict,highActivity_NodeSet, postcode_activity, node_estimatedactivity_dict, highactivityrate):
    
    # adjust this
    MaxCapacity = 120000
    random_node = random.choice(ps_to_node_dict)
    if random_node[0] not in node_estimatedactivity_dict:
        node_estimatedactivity_dict[int(random_node[0])] = postcode_activity
    else:
        node_estimatedactivity_dict[random_node[0]] = int(node_estimatedactivity_dict[random_node[0]]) + postcode_activity
    
    # adjust this
    # checks: 1) not assigning to high activity node  2) Total of estimated activity in the node not more than MaxCapacity after assigning.
    if (random_node[0] in highActivity_NodeSet) or (node_estimatedactivity_dict[random_node[0]] > MaxCapacity):
        node_estimatedactivity_dict[random_node[0]] = node_estimatedactivity_dict[random_node[0]] - postcode_activity
        return random_nextnode(ps_to_node_dict,highActivity_NodeSet, postcode_activity, node_estimatedactivity_dict, highactivityrate)
    else:
        return random_node
        
def find_highactivityrate_postcodeNode(df_postcode_to_1node, df_pplcount_inpostcode):
    
    df_postcode_to_1node['sla_postcode']=df_postcode_to_1node['sla_postcode'].apply(int)
    df_pplcount_inpostcode['sla_postcode']=df_pplcount_inpostcode['sla_postcode'].apply(int)
    df_postcode_to_1node = pd.merge(df_postcode_to_1node,df_pplcount_inpostcode, left_on ='sla_postcode', right_on='sla_postcode')
        
    df_node_estimatedactivity = df_postcode_to_1node.groupby(['nearest_node'])[['total_ppl']].sum()
    df_node_estimatedactivity = df_node_estimatedactivity.sort_values(by=['total_ppl'],ascending = False)
    df_node_estimatedactivity['nearest_node1'] = df_node_estimatedactivity.index
    
    # adjust this
    highactivityrate = 100000
    df_node_withhighactivityrate = df_node_estimatedactivity.loc[df_node_estimatedactivity['total_ppl'] > highactivityrate]
    
    df_postcode_mapped2_highactivityNode = pd.merge(left=df_postcode_to_1node, right=df_node_withhighactivityrate, how='inner', left_on = 'nearest_node', right_on = 'nearest_node1')
    
    return highactivityrate, df_node_withhighactivityrate, df_postcode_mapped2_highactivityNode, df_node_estimatedactivity
    
def find_nearestN_node(df_validnode,df_postcode,N_nearest):
    
    postcode_to_Nnode = {}
    array_validnode = df_validnode[['x', 'y']].values
    # building valid node tree map
    tBegin = time.time()
    validnode_tree = KDTree(array_validnode, leaf_size= 2, metric='euclidean')
    tEnd = time.time()
    print 'Node K-D tree built in ', (tEnd - tBegin), ' seconds '
    
    for index, row in df_postcode.iterrows():
        sla_postcode = row['sla_postcode']
        x = row['x_coord']
        y = row['y_coord']
        # find N nearest node from each postcode, output: distance from postcode to node, node's index
        [dist, ind] = validnode_tree.query([[x, y]], k=N_nearest)
        # Based on index, find the node
        nearNode = df_validnode.loc[int(ind[0][0])]['node_id']
        #print([dist, ind])
        #nearNode_zone = df_validnode.loc[int(ind[0][0])]['node_zone']
        Nnodelist = []
        for i in range (len([dist, ind][0][0])):
            nearNodeIndex = ([dist, ind][1][0][i])
            nearNodeDist = ([dist, ind][0][0][i])
            nearNode = df_validnode.loc[nearNodeIndex]['node_id']
            Nnodelist.append([nearNode,nearNodeDist])
        postcode_to_Nnode[sla_postcode] = Nnodelist
        
    return postcode_to_Nnode
    
def loadtable_node(conn):
    # Load Table: valid node to zone (valid: not sink, source, closed, loop node)
    sql_ValidNodeToZone = """SELECT node_id, taz as node_zone, x, y FROM """+table_NodeToZone+""" a
    LEFT JOIN """+tabel_Node+""" b ON a.node_id = b.id
    WHERE a.node_type NOT IN (1,8,9) and bus_terminus_node IS FALSE
    """
    df_validnode = sqlio.read_sql_query(sql_ValidNodeToZone, conn)
    
    return df_validnode


def sql_total_activeppl_in_postcode(conn):
    
    sql_get_totalactiveppl_in_postcode = """
    with activepeople_activityaddress as
    (
    SELECT work_address, count(individual_id) as number_pplactive
    FROM """+table_individual_by_id+"""
    WHERE age_category_id != 0 --- young children, 0-3 yr old
    GROUP BY work_address
    ),
    activepeople_homeaddress as
    (
    SELECT home_address, count(individual_id) as number_pplliving
    FROM """+table_individual_by_id+"""
    WHERE age_category_id != 0 --- young children, 0-3 yr old
    GROUP BY home_address
    ),
    ppl_counting as 
    (
    SELECT a.address_id, a.sla_postcode, 
    CASE WHEN number_pplactive is NULL THEN 0 ELSE number_pplactive END AS number_pplactive, 
    CASE WHEN number_pplliving is NULL THEN 0 ELSE number_pplliving END AS number_pplliving
    FROM """+table_sla_addresses+""" a
    LEFT JOIN activepeople_activityaddress b ON a.address_id = b.work_address
    LEFT JOIN activepeople_homeaddress c ON a.address_id = c.number_pplliving
    ),
    address_total_counting as
    (
    SELECT address_id, sla_postcode, number_pplactive, number_pplliving, number_pplactive + number_pplliving as total_ppl 
    FROM ppl_counting
    )
    SELECT sla_postcode, sum(number_pplactive) as number_pplactive, sum(number_pplliving) as number_pplliving, sum(total_ppl) as total_ppl
    FROM address_total_counting
    GROUP BY sla_postcode
    """
    
    df_pplcount_inpostcode = sqlio.read_sql_query(sql_get_totalactiveppl_in_postcode, conn)
    return df_pplcount_inpostcode

if __name__ == "__main__":
    main()