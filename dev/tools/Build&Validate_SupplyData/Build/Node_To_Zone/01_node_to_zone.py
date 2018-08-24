# -*- coding: utf-8 -*-
"""
Created on Wed Aug  1 15:24:27 2018

@author: chuyaw

Mapping node to zone
1. Provide database connection, table_Node, table_Link, table_BS
2. Give the file directory to access zone.shp (zoneshapefile)
3. Necessary for inputs (node and zone) to have the same coordinate system. (E.g.: Singapore, pls use UTM48N epsg:32648)
4. When reading the node table to geodataframe, please define its coordinate system (E.g.: crs_utm48n)


"""

import geopandas as gpd
from shapely.geometry import Point
import pandas as pd
import time
import psycopg2
import psycopg2.extras
import pandas.io.sql as sqlio

table_Node = 'supply_2bmerge.node_6types'
table_Link = 'supply_2bmerge.link'
table_BS = 'supply_2bmerge.bus_stop'
output_node2zone = 'node_to_zone.csv'
zoneshapefile = 'D:/Chuyaw SMART/CYT/Shapefile-SimMobility/Shapefile/TAZ zone/MTZ1169_UMT48N.shp'
gpolygon_zone = gpd.GeoDataFrame.from_file(zoneshapefile)


def main():
    #Define our connection string
    conn_string = "host='172.25.184.156' dbname='simmobility_l2nic2b' user='postgres' password='d84dmiN'"
    
    # print the connection string we will use to connect
    print ("Connecting to database\n	->%s" % (conn_string))
    
    # get a connection, if a connect cannot be made an exception will be raised here
    conn = psycopg2.connect(conn_string)
    
    tBegin = time.time()
    # loads valid node
    df_node = loadtable_node(conn)
    tEnd = time.time()
    print 'Loaded node table: ', (tEnd - tBegin), ' seconds'

        
    geometry = [Point(xy) for xy in zip(df_node.x, df_node.y)]
    df_node = df_node.drop(['x', 'y'], axis=1)
    # please define node coordinate system correctly
    crs_utm48n = {'init': 'epsg:32648'}
    gdf_node = gpd.GeoDataFrame(df_node, crs=crs_utm48n, geometry=geometry)
    
    node2zone_list =[]
    nodeWOzone_list = []
    
    tBegin = time.time()
    for Nindex, node in gdf_node.iterrows():
        node_id = node.id
        node_type = node.node_type
        traffic_light = node.traffic_light
        source = node.source
        sink = node.sink
        expressway = node.expressway
        intersection = node.intersection
        bus_terminus_node = node.bus_terminus_node
        intersect_zone = 0
        for Zindex, zone in gpolygon_zone.iterrows():
            if node['geometry'].intersects(zone['geometry']):
                intersect_zone = 1
                node2zone_list.append([int(node_id),int(node_type),traffic_light,source,sink,expressway,intersection,bus_terminus_node,int(zone.MTZ_1169)])
                break
        if intersect_zone == 0:
            nodeWOzone_list.append(int(node_id))
            node2zone_list.append([int(node_id),int(node_type),traffic_light,source,sink,expressway,intersection,bus_terminus_node,0])
            
    col_labels = ['node_id','node_type','traffic_light','source','sink','expressway','intersection','bus_terminus_node','node_zone']
    #list to dataframe
    df_node2zone = pd.DataFrame.from_records(node2zone_list, columns = col_labels)
    df_node2zone.to_csv(output_node2zone, index = False)
    tEnd = time.time()
    print 'Mapped Node to Zone. Exported output file :', (tEnd - tBegin), ' seconds'
    print'Number of Node without mapping of zone: ' , len(nodeWOzone_list) , '  ', nodeWOzone_list
    
    conn.close()
    
def loadtable_node(conn):
    
    # Load Table: valid node to zone (valid: not sink, source, closed, loop node)
    sql_Node = """
    with temp1 as
    (
    -- ss_node . sink or source node
    SELECT id as ss_node FROM """+table_Node+""" WHERE node_type = 1
    ),
    -- sink node
    temp_sinknode as
    (
    SELECT to_node FROM """+table_Link+""" WHERE to_node IN (SELECT ss_node FROM temp1)
    ),
    -- source node
    temp_sourcenode as
    (
    SELECT from_node FROM """+table_Link+""" WHERE from_node IN (SELECT ss_node FROM temp1)
    ),
    -- expressay node
    temp_expresswaynode as
    (
    (SELECT from_node as expresswaynode FROM """+table_Link+"""
    WHERE category = 1)
    UNION
    (SELECT to_node as expresswaynode FROM """+table_Link+"""
    WHERE category = 1)
    )
    SELECT id, x, y, node_type, 
    CASE WHEN traffic_light_id = 0 THEN FALSE ElSE TRUE END as traffic_light,
    CASE WHEN id IN (SELECT from_node FROM temp_sourcenode) THEN TRUE ELSE FALSE END as source,
    CASE WHEN id IN (SELECT to_node FROM temp_sinknode) THEN TRUE ELSE FALSE END as sink,
    CASE WHEN id IN (SELECT expresswaynode FROM temp_expresswaynode) THEN TRUE ELSE FALSE END as expressway,
    CASE WHEN node_type = 2 THEN TRUE ELSE FALSE END as intersection,
    CASE WHEN id IN (SELECT terminal_node FROM """+table_BS+""" WHERE terminal_node != 0) THEN TRUE ELSE FALSE END AS bus_terminus_node
    FROM """+table_Node+"""

    """
    df_node = sqlio.read_sql_query(sql_Node, conn)
    
    return df_node

if __name__ == "__main__":
    main()