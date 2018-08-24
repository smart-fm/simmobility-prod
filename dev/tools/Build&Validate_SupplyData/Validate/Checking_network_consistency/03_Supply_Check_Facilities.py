# -*- coding: utf-8 -*-
"""
Created on Fri May 18 16:53:23 2018

@author: cyaot

Tool: Check supply's facilities
- bus stop
- taxi stand
- parking 

Steps:
    1. Provide the connection detail to database (e.g.: database name, host id, password, username)
    2. Input the table names. 
    
"""
import psycopg2
import psycopg2.extras
import pprint
import sys
from tabulate import tabulate

table_segment = 'supply.segment'
table_link = 'supply.link'
table_node = 'supply.node'
table_bs = 'supply.bus_stop'
table_taxistand = 'supply.taxi_stand_2030'
table_parking = 'supply.parking'

def main():
    
    #Define our connection string
    #conn_string = "host='172.25.184.11' dbname='simmobility_db_2014' user='postgres' password='5M_S1mM0bility'"
    #conn_string = "host='172.25.184.48' dbname='simmobcity' user='postgres' password='5M_S1mM0bility'"
    conn_string = "host='172.25.184.156' dbname='simmobility_l2nic2b' user='postgres' password='d84dmiN'"
    
    # print the connection string we will use to connect
    print ("Connecting to database\n	->%s" % (conn_string))
    
    # get a connection, if a connect cannot be made an exception will be raised here
    conn = psycopg2.connect(conn_string)
    
    # conn.cursor will return a cursor object, you can use this query to perform queries
    # note that in this example we pass a cursor_factory argument that will
    # dictionary cursor so COLUMNS will be returned as a dictionary so we
    # can access columns by their name instead of index.
    cursor = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
    
    # check records of bus stop
    cursor.execute("""SELECT * from """ + table_bs + """ LIMIT 1""")
    records_bs = cursor.fetchall()
    if records_bs:
        check_busstop(cursor)
        print('Checked bus stop.')   
    else:
        print('The bus stop table is empty.')
    
    # check records of taxi stand
    cursor.execute("""SELECT * from """ + table_taxistand + """ LIMIT 1""")
    records_tx = cursor.fetchall()
    if records_tx:
        check_taxistand(cursor)
        print('Checked taxi stand.') 
    else:
        print('The taxi stand table is empty.')
    
    # check records of parking
    cursor.execute("""SELECT * from """ + table_parking + """ LIMIT 1""")
    records_prk = cursor.fetchall()
    if records_prk:
        check_parking(cursor)
        print('Checked parking.') 
    else:
        print('The parking table is empty.')
        
        
def check_busstop(cursor):
    # 1) check bus stop's column. 
    cursor.execute("""
    SELECT code, status, terminal, length, section_offset
    FROM """ + table_bs + """
    WHERE code IS NULL or status IS NULL or terminal IS NULL or length IS NULL or section_offset IS NULL
    """)
    colnames = [desc[0] for desc in cursor.description]
    missingdata_bs = cursor.fetchall()
    if missingdata_bs:
        print('\n Error! Missing data in bus stop table.')
        print (tabulate(missingdata_bs, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 2) check existance of segment
    cursor.execute("""
    SELECT code
    FROM """ + table_bs + """
    WHERE section_id NOT IN (SELECT id FROM """ + table_segment + """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    unknown_sectionid = cursor.fetchall()
    if unknown_sectionid:
        print('\n Error! Unknown section_id which does not exist in table segment.')
        print (tabulate(unknown_sectionid, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 3) check unique bus stop code
    cursor.execute("""
    SELECT code, count(id)
    FROM """ + table_bs + """
    GROUP BY code
    HAVING count(id) > 1
    """)
    colnames = [desc[0] for desc in cursor.description]
    duplicate_busstop_code = cursor.fetchall()
    if duplicate_busstop_code:
        print('\n Error! Duplicated bus stop code.')
        print (tabulate(duplicate_busstop_code, headers=colnames))
        #sys.exit()
    else:
        pass
    
    # 4) check bus terminal 1. all available data
    cursor.execute("""
    SELECT code, terminal, section_id, reverse_section, terminal_node
    FROM """ + table_bs + """
    WHERE terminal = 1 and (reverse_section IS NULL or terminal_node IS NULL 
    or reverse_section = 0 or terminal_node = 0)
    """)
    colnames = [desc[0] for desc in cursor.description]
    emptydata_busterminal = cursor.fetchall()
    if emptydata_busterminal:
        print('\n Error! Please fill in reserve section and terminal node information, for bus terminal only.')
        print (tabulate(emptydata_busterminal, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 5) check bus terminal node. 
    cursor.execute("""
    with temp1 as
    (
    SELECT code, section_id, terminal_node,
    CASE WHEN terminal_node NOT IN (c.from_node, c.to_node, e.from_node, e.to_node) THEN 'wrong terminal node'
     WHEN terminal_node = c.from_node and terminal_node = e.to_node THEN 'located at downstream link'
     WHEN terminal_node = c.to_node and terminal_node= e.from_node THEN 'located at upstream link'
    ELSE 'NIL'
    END AS validate_terminal_node
    FROM """ + table_bs + """ a
    LEFT JOIN """ + table_segment + """ b ON a.section_id = b.id
    LEFT JOIN """ + table_link + """ c ON b.link_id = c.id
    LEFT JOIN """ + table_segment + """ d ON a.reverse_section = d.id
    LEFT JOIN """ + table_link + """ e ON d.link_id = e.id
    WHERE a.terminal = 1
    )
    SELECT * FROM temp1 WHERE validate_terminal_node IN ('wrong terminal node', 'NIL')
    """)
    colnames = [desc[0] for desc in cursor.description]
    wrong_terminalnode = cursor.fetchall()
    if wrong_terminalnode:
        print('\n Error! Check the bus terminal node.')
        print (tabulate(wrong_terminalnode, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 6) check bus stop offset. can't more than segment's length
    cursor.execute("""
    SELECT code, section_offset, b.length
    FROM """ + table_bs + """ a
    LEFT JOIN supply.segment b ON a.section_id = b.id
    WHERE section_offset > b.length
    """)
    colnames = [desc[0] for desc in cursor.description]
    longer_sectionoffset = cursor.fetchall()
    if longer_sectionoffset:
        print('\n Error! The section offset is longer than segment length.')
        print (tabulate(longer_sectionoffset, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 7) check location of bus stop. cannot duplicate (same location)
    cursor.execute("""
    SELECT a.code as busstop1_code, a.section_id as busstop1_section_id, a.section_offset as busstop1_section_offset, 
    b.code as busstop2_code, b.section_id as busstop2_section_id, b.section_offset as busstop2_section_offset
    FROM """ + table_bs + """ a
    INNER JOIN """ + table_bs + """ b
    ON a.section_id = b.section_id and a.section_offset = b.section_offset
    WHERE a.code != b.code
    """)
    colnames = [desc[0] for desc in cursor.description]
    bs_samelocation = cursor.fetchall()
    if bs_samelocation:
        print('\n Error! Bus stops are located at the exact same location.')
        print (tabulate(bs_samelocation, headers=colnames))
        sys.exit()
    else:
        pass
    
def check_taxistand(cursor):
    # 1) check taxi stand columns.
    cursor.execute("""
    SELECT id, segment_id, length, section_offset
    FROM """ + table_taxistand + """
    WHERE segment_id IS NULL or length IS NULL or section_offset IS NULL
    """)
    colnames = [desc[0] for desc in cursor.description]
    missing_taxistand_data = cursor.fetchall()
    if missing_taxistand_data:
        print('\n Error! Some data are missing. Please fill up.')
        print (tabulate(missing_taxistand_data, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 2) check existance of segmente id
    cursor.execute("""
    SELECT id
    FROM """ + table_taxistand + """
    WHERE segment_id NOT IN (SELECT id FROM """ + table_segment + """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    unknown_segmentid = cursor.fetchall()
    if unknown_segmentid:
        print('\n Error! Unknown segment id which does not exist in table segment.')
        print (tabulate(unknown_segmentid, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 3) discrepancy of length and segment length
    cursor.execute("""
    SELECT a.id, a.segment_id, a.length, b.length as segment_length
    FROM """ + table_taxistand + """ a
    LEFT JOIN """ + table_segment + """ b ON a.segment_id =  b.id
    WHERE a.length != b.length
    """)
    colnames = [desc[0] for desc in cursor.description]
    diff_btw_segmentlength = cursor.fetchall()
    if diff_btw_segmentlength:
        print('\n Error! Discrepancy segment length data.')
        print (tabulate(diff_btw_segmentlength, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 4) check taxi stand offset. can't more than segment's length
    cursor.execute("""
    SELECT id, segment_id, length, section_offset
    FROM """ + table_taxistand + """ 
    WHERE section_offset > length
    """)
    colnames = [desc[0] for desc in cursor.description]
    longer_taxistd_sectionoffset = cursor.fetchall()
    if longer_taxistd_sectionoffset:
        print('\n Error! Taxi stand section offset is longer than segment length.')
        print (tabulate(longer_taxistd_sectionoffset, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 5) check location of taxi stand. cannot duplicate (same location)
    cursor.execute("""
    SELECT b.id as taxistand1_id, b.segment_id as taxistand1_segment_id, b.section_offset as taxistand1_section_offset,
    b.id as taxistand2_id, b.segment_id as taxistand2_segment_id, b.section_offset as taxistand2_section_offset
    FROM """ + table_taxistand + """  a
    INNER JOIN """ + table_taxistand + """  b
    ON a.segment_id = b.segment_id and a.section_offset = b.section_offset
    WHERE a.id != b.id
    """)
    colnames = [desc[0] for desc in cursor.description]
    taxistand_samelocation = cursor.fetchall()
    if taxistand_samelocation:
        print('\n Error! Taxi stands located at the exact same location.')
        print (tabulate(taxistand_samelocation, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 6) check taxi stand and bus stop segment id and segment offset. They cannot located at the exact same location. 
    cursor.execute("""
    SELECT a.code as busstop_code, a.section_id as busstop_section_id, a.section_offset as busstop_section_offset, 
    b.id as taxistand_id, b.segment_id as taxistand_segment_id, b.section_offset as taxistand_section_offset
    FROM """ + table_bs + """ a
    INNER JOIN """ + table_taxistand + """ b
    ON a.section_id = b.segment_id and a.section_offset = b.section_offset
    """)
    colnames = [desc[0] for desc in cursor.description]
    bs_taxistd_samelocation = cursor.fetchall()
    if bs_taxistd_samelocation:
        print('\n Error! Bus stop and taxi stand are located at the exact same location.')
        print (tabulate(bs_taxistd_samelocation, headers=colnames))
        sys.exit()
    else:
        pass

def check_parking(cursor):
    # 1) check unique parking id
    cursor.execute("""
    SELECT parking_id FROM """ + table_parking + """ GROUP BY parking_id
    HAVING count(parking_id) > 1
    """)
    colnames = [desc[0] for desc in cursor.description]
    duplicate_parkingid = cursor.fetchall()
    if duplicate_parkingid:
        print('\n Error! Duplicate parking id.')
        print (tabulate(duplicate_parkingid, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 2) check existance of segmente id
    cursor.execute("""
    SELECT parking_id
    FROM """ + table_parking + """
    WHERE segment_id NOT IN (SELECT id FROM """ + table_segment + """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    unknown_segmentid_prk = cursor.fetchall()
    if unknown_segmentid_prk:
        print('\n Error! Unknown segment id which does not exist in table segment.')
        print (tabulate(unknown_segmentid_prk, headers=colnames))
        sys.exit()
    else:
        pass 
    
    # 3) check existance of node , access_node and egress_node
    cursor.execute("""
    SELECT parking_id, access_node, egress_node
    FROM """ + table_parking + """
    WHERE access_node NOT IN (SELECT id FROM """ + table_node + """) 
    or egress_node NOT IN (SELECT id FROM """ + table_node + """) 
    """)
    colnames = [desc[0] for desc in cursor.description]
    unknown_nodeid_prk = cursor.fetchall()
    if unknown_nodeid_prk:
        print('\n Error! Unknown node id which does not exist in table node.')
        print (tabulate(unknown_nodeid_prk, headers=colnames))
        sys.exit()
    else:
        pass 



if __name__ == "__main__":
    main()