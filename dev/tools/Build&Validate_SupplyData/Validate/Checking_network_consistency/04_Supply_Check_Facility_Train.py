# -*- coding: utf-8 -*-
"""
Created on Wed May 23 11:38:00 2018

@author: cyaot

Tool: Check supply's facility
- Train:
    :- train stop
    :- train_access_segment
    :- pt_train_block, pt_train_route, pt_train_route_platform (pending)

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
table_trainstop = 'supply.train_stop'
table_train_platform = 'supply.train_platform'
table_train_segment = 'supply.train_access_segment'

def check_train_stop(cursor):
    # 1) check NULL platform_name
    cursor.execute("""
    SELECT id, platform_name, station_name
    FROM """ + table_trainstop + """
    WHERE platform_name IS NULL
    """)
    colnames = [desc[0] for desc in cursor.description]
    null_platform_name = cursor.fetchall()
    if null_platform_name:
        print('\n Error! NULL platform_name in table train stop.')
        print (tabulate(null_platform_name, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 2) check unique platform_name
    cursor.execute("""
    SELECT platform_name
    FROM """ + table_trainstop + """
    GROUP BY platform_name HAVING count(platform_name) > 1
    """)
    colnames = [desc[0] for desc in cursor.description]
    duplicate_platform_name = cursor.fetchall()
    if duplicate_platform_name:
        print('\n Error! Duplicate platform_name in table train_stop.')
        print (tabulate(duplicate_platform_name, headers=colnames))
        sys.exit()
    else:
        pass

def check_train_access_segment(cursor):
     # 1) check train stop with no access segment
    cursor.execute("""
    SELECT id, platform_name
    FROM """ + table_trainstop + """
    WHERE id NOT IN (SELECT mrt_stop_id FROM """ + table_train_segment + """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    trainstop_noaccess = cursor.fetchall()
    if trainstop_noaccess:
        print('\n Error! Train stop with no access segment.')
        print (tabulate(trainstop_noaccess, headers=colnames))
        sys.exit()
    else:
        pass
    
    # 2) check unknown train stop
    cursor.execute("""
    SELECT mrt_stop_id FROM """ + table_train_segment + """
    WHERE mrt_stop_id NOT IN (SELECT id FROM """ + table_trainstop + """)
    """)
    colnames = [desc[0] for desc in cursor.description]
    unknowntrainstop_accessseg = cursor.fetchall()
    if unknowntrainstop_accessseg:
        print('\n Error! Unknown mrt stop id in table train access segment.')
        print (tabulate(unknowntrainstop_accessseg, headers=colnames))
        sys.exit()
    else:
        pass

def check_train_platform(cursor):
    pass
    


def main():
    
    #Define our connection string
    #conn_string = "host='172.25.184.11' dbname='simmobility_db_2014' user='postgres' password='5M_S1mM0bility'"
    conn_string = "host='172.25.184.48' dbname='simmobcity' user='postgres' password='5M_S1mM0bility'"
    #conn_string = "host='172.25.184.156' dbname='simmobility_l2nic2b' user='postgres' password='d84dmiN'"
    
    # print the connection string we will use to connect
    print ("Connecting to database\n	->%s" % (conn_string))
    
    # get a connection, if a connect cannot be made an exception will be raised here
    conn = psycopg2.connect(conn_string)
    
    # conn.cursor will return a cursor object, you can use this query to perform queries
    # note that in this example we pass a cursor_factory argument that will
    # dictionary cursor so COLUMNS will be returned as a dictionary so we
    # can access columns by their name instead of index.
    cursor = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
    
    # check records of train stop
    cursor.execute("""SELECT * from """ + table_trainstop + """ LIMIT 1""")
    records_trainstop = cursor.fetchall()
    if records_trainstop:
        check_train_stop(cursor)
        print('Checked train platform.')
        check_train_access_segment(cursor)
        print('Checked train access segment.')
    else:
        print('The train stop table is empty.')
    

if __name__ == "__main__":
    main()