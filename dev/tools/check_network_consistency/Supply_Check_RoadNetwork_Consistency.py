# -*- coding: utf-8 -*-
"""
Created on Wed May 16 14:28:05 2018

@author: cyaot

Tool: Check supply road network consistency 

Steps:
    1. Provide the connection detail to database (e.g.: database name, host id, password, username)
    2. Input the table names. 
          
"""

import psycopg2
import psycopg2.extras
import sys
from tabulate import tabulate

table_node = 'supply.node'
table_segment = 'supply.segment'
table_lane = 'supply.lane'
table_link = 'supply.link'
table_tp = 'supply.turning_path'
table_tg = 'supply.turning_group'
table_connector = 'supply.connector'
table_lanepoly = 'supply.lane_polyline'
table_tppoly = 'supply.turning_path_polyline'

def main():
    
    #Define our connection string
    conn_string = "host='172.25.184.156' dbname='simmobility_virtualcity_sms_casestudy2017' user='postgres' password='d84dmiN'"
    
    # print the connection string we will use to connect
    print ("Connecting to database\n	->%s" % (conn_string))
    
    # get a connection, if a connect cannot be made an exception will be raised here
    conn = psycopg2.connect(conn_string)
    
    # conn.cursor will return a cursor object, you can use this query to perform queries
    # note that in this example we pass a cursor_factory argument that will
    # dictionary cursor so COLUMNS will be returned as a dictionary so we
    # can access columns by their name instead of index.
    cursor = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
    


    check_lane(cursor)
    print('Checked lane.')
    check_segment(cursor)
    print('Checked segment.')
    check_link(cursor)
    print('Checked link.')
    check_tp_tg(cursor)
    print('Checked turning path & turning group.')
    check_others(cursor)
    print('Checked others.')
    

def check_lane(cursor):
    # 1) check no of lane in segment. num_lanes VS real count no of lane
    cursor.execute("""
    SELECT b.id, b.num_lanes, a.count_lane FROM
    (SELECT segment_id, count(id) as count_lane FROM """ + table_lane + """ GROUP BY segment_id) a 
    LEFT JOIN """ + table_segment + """ b ON a.segment_id = b.id 
    WHERE b.num_lanes != a.count_lane
    """)
    colnames = [desc[0] for desc in cursor.description]
    error_numoflanes = cursor.fetchall()
    if error_numoflanes:
        print('\n Error! Discrepancy of num_lanes value and real count of lanes in the segment.')
        print (tabulate(error_numoflanes, headers=colnames))
        sys.exit()
    else:
        pass

    
    # 2) Check the lane ordering
    cursor.execute("""
    SELECT segment_id, min(id - segment_id*100) as min_laneorder FROM """ + table_lane +""" 
    GROUP BY segment_id HAVING min(id - segment_id*100) != 0
    """)
    colnames = [desc[0] for desc in cursor.description]
    error_laneorder2 = cursor.fetchall()
    if error_laneorder2:
        print('\n Error! Wrong Lane Ordering. Must starting from 0.')
        print (tabulate(error_laneorder2, headers=colnames))
        sys.exit()
    else:
        pass
    
    
    # 3) Check the lane ordering
    cursor.execute("""
    with temp1 as( 
    SELECT a.segment_id, (a.max_laneorder -(num_lanes -1)) as dif FROM
    (
    SELECT segment_id, (max(id) - segment_id*100)  as max_laneorder
    FROM """ + table_lane +""" GROUP BY segment_id
    ) a, """ + table_segment + """ WHERE a.segment_id = segment.id
    ) SELECT * FROM temp1 where dif != 0
    """)
    colnames = [desc[0] for desc in cursor.description]
    error_laneorder3 = cursor.fetchall()
    if error_laneorder3:
        print('\n Error! Wrong Lane Ordering. Not in sequence order.')
        print (tabulate(error_laneorder3, headers=colnames))
        sys.exit()
    else:
        pass
    
    
    # 4) Check discrepancy of lane and segment 1
    cursor.execute("""
    SELECT DISTINCT segment_id FROM """ + table_lane +""" 
    WHERE segment_id NOT IN (SELECT id FROM """ + table_segment + """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    unknownsegment = cursor.fetchall()
    if unknownsegment:
        print('\n Error! Unknown segment_id in table lane')
        print (tabulate(unknownsegment, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 5) Check discrepancy of lane and segment 2
    cursor.execute("""
    SELECT id FROM """ + table_segment + """
    WHERE id NOT IN (SELECT DISTINCT segment_id FROM """ + table_lane +""")
    """)
    colnames = [desc[0] for desc in cursor.description]
    segment_xlanes = cursor.fetchall()
    if segment_xlanes:
        print('\n Error! This segment dont have lane')
        print (tabulate(segment_xlanes, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 6) Check lane id
    cursor.execute("""
    SELECT id, ROUND(id /100,0) as round, segment_id FROM """ + table_lane +"""
    WHERE ROUND(id /100,0) != segment_id
    """)
    colnames = [desc[0] for desc in cursor.description]
    wrong_laneid = cursor.fetchall()
    if wrong_laneid:
        print('\n Error! Wrong lane id. lane id = segment id * 100 + lane order')
        print (tabulate(wrong_laneid, headers=colnames))
        sys.exit()
    else:
        pass

def check_segment(cursor):
    # 1) Check segment's link sequence 
    cursor.execute("""
    SELECT link_id ,sequence_num, count(link_id) as count
    FROM """ + table_segment + """
    GROUP BY link_id, sequence_num HAVING count(link_id) != 1
    """)
    colnames = [desc[0] for desc in cursor.description]
    dup_linkseq = cursor.fetchall()
    if dup_linkseq:
        print('\n Error! Duplicate link sequence in table segment.')
        print (tabulate(dup_linkseq, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 2) Check discrepancy of segment and link 1
    cursor.execute("""
    SELECT DISTINCT link_id FROM """ + table_segment + """
    WHERE link_id NOT IN (SELECT id FROM """ + table_link+ """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    unknownlink = cursor.fetchall()
    if unknownlink:
        print('\n Error! Unknown link in table segment.')
        print (tabulate(unknownlink, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 3) Check discrepancy of segment and link 2
    cursor.execute("""
    SELECT id FROM """ + table_link+ """ 
    WHERE id NOT IN (SELECT DISTINCT link_id FROM """ + table_segment + """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    link_xsegment = cursor.fetchall()
    if link_xsegment:
        print('\n Error! The link dont have segment')
        print (tabulate(link_xsegment, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 4)
    cursor.execute("""
    SELECT link_id, count (id) as countseg, (MAX(sequence_num) - MIN(sequence_num) + 1) as diff
    FROM """ + table_segment + """
    GROUP BY link_id
    HAVING count (id) != (MAX(sequence_num) - MIN(sequence_num) + 1) 
    """)
    colnames = [desc[0] for desc in cursor.description]
    wrong_segment_linkseq1 = cursor.fetchall()
    if wrong_segment_linkseq1:
        print('\n Error! The link_sequence not in sequence ordered')
        print (tabulate(wrong_segment_linkseq1, headers=colnames))
        sys.exit()
    else:
        pass

def check_link(cursor):
    # 1) Check discrepancy of link and node 1
    cursor.execute("""
    SELECT id, from_node, to_node 
    FROM """ + table_link + """
    WHERE from_node NOT IN (SELECT id FROM """ + table_node + """) 
    or to_node NOT IN (SELECT id FROM """ + table_node + """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    unknown_nodeid = cursor.fetchall()
    if unknown_nodeid:
        print('\n Error! The unknown from_node / to_node in link table.')
        print (tabulate(unknown_nodeid, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 2) Check discrepancy of link and node 2
    cursor.execute("""
    SELECT id FROM """ + table_node + """
    WHERE id NOT IN (SELECT from_node FROM """ + table_link + """) and 
    id NOT IN (SELECT to_node FROM """ + table_link + """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    node_xlink = cursor.fetchall()
    if node_xlink:
        print('\n Error! The node not found the link table, from_node and to_node.')
        print (tabulate(node_xlink, headers=colnames))
        sys.exit()
    else:
        pass
    
    
    # 3) Check from/to_node of link. No loop node. 
    cursor.execute("""
    SELECT id, from_node, to_node
    FROM """ + table_link + """ 
    WHERE from_node = to_node
    """)
    colnames = [desc[0] for desc in cursor.description]
    loop_node = cursor.fetchall()
    if loop_node:
        print('\n Error! Loop node, from_node same to to_node, which is not allowed.')
        print (tabulate(loop_node, headers=colnames))
        #sys.exit()
    else:
        pass
    
    

def check_tp_tg(cursor):
    # 1) Check turning path connectivity
    cursor.execute("""
    with max_lane as
    (
    SELECT a.id, b.x, b.y, a.seq FROM
    (SELECT id, max(seq_id) AS seq FROM """ + table_lanepoly + """ GROUP BY id) a
    LEFT JOIN """ + table_lanepoly + """ b
    ON a.id = b.id and a.seq = b.seq_id
    ),
    min_lane as
    (
    SELECT id, x, y, seq_id as seq
    FROM """ + table_lanepoly + """
    WHERE seq_id = 0
    ),
    max_tp as
    (
    SELECT a.id, b.x, b.y, a.seq FROM
    (SELECT id, max(seq_id) AS seq FROM """ + table_tppoly + """ GROUP BY id) a
    LEFT JOIN """ + table_tppoly + """ b
    ON a.id = b.id and a.seq = b.seq_id
    ),
    min_tp as
    (
    SELECT id, x, y, seq_id as seq
    FROM """ + table_tppoly + """
    WHERE seq_id = 0
    ),
    temp1 as
    (
    SELECT a.id, a.from_lane, a.to_lane, b.x as mintp_x, b.y as mintp_y, c.x as maxtp_x, c.y as maxtp_y, 
    d.x as minlane_x, d.y as minlane_y, e.x as maxlane_x, e.y as maxlane_y
    FROM """ + table_tp + """ a
    LEFT JOIN min_tp b ON a.id = b.id
    LEFT JOIN max_tp c ON a.id = c.id
    LEFT JOIN min_lane d ON c.x = d.x AND c.y = d.y
    LEFT JOIN max_lane e ON b.x = e.x AND b.y = e.y
    )
    SELECT id, from_lane, to_lane FROM """ + table_tp + """
    WHERE id NOT IN (SELECT id FROM temp1)
    """)
    colnames = [desc[0] for desc in cursor.description]
    misconnected_tp = cursor.fetchall()
    if misconnected_tp:
        print('\n Error! Check the connectivity of the turning path (Tips: Snap to lane polyline).')
        print (tabulate(misconnected_tp, headers=colnames))
        sys.exit()
    else:
        pass

    # 2) Check turning path max_speed. Get the lower speed between upstream and downstream segments.
    cursor.execute("""
    with temp1 as
    (SELECT a.id, a.max_speed, a.from_lane, b.segment_id as from_seg, a.to_lane, c.segment_id as to_seg,
    CASE WHEN d.max_speed <= e.max_speed THEN d.max_speed
    WHEN d.max_speed > e.max_speed THEN e.max_speed END AS tp_max_speed
    FROM """ + table_tp + """ a
    LEFT JOIN """ + table_lane + """ b ON a.from_lane = b.id
    LEFT JOIN """ + table_lane + """ c ON a.to_lane = c.id
    LEFT JOIN """ + table_segment + """ d ON b.segment_id = d.id
    LEFT JOIN """ + table_segment + """ e ON c.segment_id = e.id
    )SELECT id, max_speed, tp_max_speed FROM temp1 WHERE max_speed != tp_max_speed
    """)
    colnames = [desc[0] for desc in cursor.description]
    wrong_tpspeed = cursor.fetchall()
    if wrong_tpspeed:
        print('\n Error! Wrong turning path speed. Please get the lower speed between upstream and downstream segments.')
        print (tabulate(wrong_tpspeed, headers=colnames))
        #sys.exit()
    else:
        pass
    
    #3) Check turning group 1. Check multiple turning group id in single turning (link pair)
    cursor.execute("""
    with temp1 as
    (
    SELECT DISTINCT a.group_id, b.segment_id as from_segment, c.segment_id as to_segment
    FROM """ + table_tp + """ a
    LEFT JOIN """ + table_lane + """ b ON a.from_lane = b.id
    LEFT JOIN """ + table_lane + """ c ON a.to_lane  = c.id
    )SELECT from_segment, to_segment, count(group_id) as duplicate
    FROM temp1 GROUP BY from_segment, to_segment HAVING count(group_id) >  1
    """)
    colnames = [desc[0] for desc in cursor.description]
    multi_tg_oneturning = cursor.fetchall()
    if multi_tg_oneturning:
        print('\n Error! More than one turning group id in the single turning. (Tips: link A to link B, one unique turning group only.')
        print (tabulate(multi_tg_oneturning, headers=colnames))
        sys.exit()
    else:
        pass
    
    #4) Check turning group 2. One turning group id found in multiple turnings
    cursor.execute("""
    with temp1 as
    (
    SELECT DISTINCT a.group_id, b.segment_id as from_segment, c.segment_id as to_segment
    FROM """ + table_tp + """ a
    LEFT JOIN """ + table_lane + """ b ON a.from_lane = b.id
    LEFT JOIN """ + table_lane + """ c ON a.to_lane  = c.id
    )SELECT group_id, count(from_segment) as duplicate
    FROM temp1 GROUP BY group_id HAVING count(from_segment) >  1
    """)
    colnames = [desc[0] for desc in cursor.description]
    one_tg_multiturning = cursor.fetchall()
    if one_tg_multiturning:
        print('\n Error! Turning group found in multiple turnings (Tips: link A to link B, one unique turning group only.')
        print (tabulate(one_tg_multiturning, headers=colnames))
        sys.exit()
    else:
        pass
    
    #5) Check turning group (link to link connectivity)
    cursor.execute("""
    SELECT DISTINCT a.group_id, f.to_node as node1, g.from_node as node2
    FROM """ + table_tp + """ a
    LEFT JOIN """ + table_lane + """ b ON a.from_lane = b.id
    LEFT JOIN """ + table_lane + """ c ON a.to_lane = c.id
    LEFT JOIN """ + table_segment + """ d ON b.segment_id = d.id
    LEFT JOIN """ + table_segment + """ e ON c.segment_id = e.id
    LEFT JOIN """ + table_link + """ f ON d.link_id = f.id
    LEFT JOIN """ + table_link + """ g ON e.link_id = g.id
    WHERE f.to_node != g.from_node
    """)
    colnames = [desc[0] for desc in cursor.description]
    misconnect_tg_link2link = cursor.fetchall()
    if misconnect_tg_link2link:
        print('\n Error! Turning group connection between links. Please check from_node & to_node of the upstream and downstream links')
        print (tabulate(misconnect_tg_link2link, headers=colnames))
        sys.exit()
    else:
        pass
    
    #6) Check discrepancy of turning group and turning path 1
    cursor.execute("""
    SELECT group_id FROM """ + table_tp + """
    WHERE group_id NOT IN (SELECT id FROM """ + table_tg + """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    unknown_tg_id = cursor.fetchall()
    if unknown_tg_id:
        print('\n Error! Unknown turning group_id found in turning_path table.')
        print (tabulate(unknown_tg_id, headers=colnames))
        sys.exit()
    else:
        pass
    
    #7) Check discrepancy of turning group and turning path 2
    cursor.execute("""
    SELECT id FROM """ + table_tg + """
    WHERE id NOT IN (SELECT group_id FROM """ + table_tp + """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    tg_xtp = cursor.fetchall()
    if tg_xtp:
        print('\n Error! Turning group that not found in table turning_path, group_id.')
        print (tabulate(tg_xtp, headers=colnames))
        sys.exit()
    else:
        pass
    
    
    #8) No turnings at the node (except sink/source node)
    cursor.execute("""
    SELECT id FROM """ + table_node + """
    WHERE node_type != 1 AND id NOT IN (SELECT node_id FROM """ + table_tg + """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    xturning_atnode = cursor.fetchall()
    if xturning_atnode:
        print('\n Error! No turning at the below nodes, except for sink/source node.')
        print (tabulate(xturning_atnode, headers=colnames))
        #sys.exit()
    else:
        pass
    
    
    #9) Check downstream missing turning path
    cursor.execute("""
    with temp1 as
    (
    SELECT id FROM """ + table_lane + """
    WHERE segment_id IN (
    SELECT segment_id
    FROM """ + table_tp + """ a
    LEFT JOIN """ + table_lane + """ b ON a.from_lane = b.id)
    )
    SELECT id FROM temp1
    WHERE id NOT IN (SELECT DISTINCT from_lane FROM """ + table_tp + """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    missing_downstream_tp = cursor.fetchall()
    if missing_downstream_tp:
        print('\n Error! Missing downstream turning paths.')
        print (tabulate(missing_downstream_tp, headers=colnames))
        sys.exit()
    else:
        pass

def check_others(cursor):
    
    # 1) check segmente's link sequence
    cursor.execute("""
    with temp1 as
    (
    SELECT DISTINCT to_lane , b.segment_id, c.link_id, c.sequence_num FROM 
    """ + table_tp + """ a
    LEFT JOIN """ + table_lane + """ b ON a.to_lane = b.id
    LEFT JOIN """ + table_segment + """ c ON b.segment_id = c.id
    ),
    temp2 as
    (
    SELECT link_id, min(sequence_num) as min_seg_seq
    FROM """ + table_segment + """
    GROUP BY link_id
    )
    SELECT segment_id, temp1.link_id, sequence_num 
    FROM temp1, temp2
    WHERE temp1.link_id = temp2.link_id
    and sequence_num !=   min_seg_seq
    """)
    colnames = [desc[0] for desc in cursor.description]
    wrong_segment_linkseq2 = cursor.fetchall()
    if wrong_segment_linkseq2:
        print('\n Error! The link sequence was wrong.')
        print (tabulate(wrong_segment_linkseq2, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 2) check missing lane connector
    cursor.execute("""
    with temp1 as
    (
    SELECT link_id, count(id) as count_seg
    FROM """ + table_segment + """
    GROUP BY link_id
    HAVING count(id) > 1
    ), 
    temp2_1 as
    (
    SELECT id, link_id, sequence_num
    FROM """ + table_segment + """
    WHERE link_id IN (SELECT link_id FROM temp1)
    ),
    temp2_2 as
    (
    SELECT id, link_id, sequence_num
    FROM """ + table_segment + """
    WHERE link_id IN (SELECT link_id FROM temp1)
    ),
    temp3 as
    (
    SELECT temp2_1.id as id1, temp2_1.link_id, temp2_1.sequence_num, temp2_2.id as id2, temp2_2.link_id, temp2_2.sequence_num
    FROM temp2_1, temp2_2
    WHERE temp2_1.link_id = temp2_2.link_id and temp2_2.sequence_num - temp2_1.sequence_num = 1
    ORDER BY temp2_1.link_id, temp2_1.sequence_num
    )
    SELECT DISTINCT id1 as from_segment, id2 as to_segment
    FROM temp3
    WHERE (id1, id2) NOT IN (SELECT DISTINCT from_segment, to_segment FROM """ + table_connector + """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    missing_connector = cursor.fetchall()
    if missing_connector:
        print('\n Error! Missing connector between segments.')
        print (tabulate(missing_connector, headers=colnames))
        #sys.exit()
    else:
        pass


    
    
if __name__ == "__main__":
    main()