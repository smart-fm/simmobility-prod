# -*- coding: utf-8 -*-
"""
Created on Wed Jul 18 17:19:21 2018
@author: chuyaw

Download and reproject coordinate system of sla_address (postcode)
1. Provide database connection, table_slaAddress, table_Taz
2. Set the reprojection. 
    - False. Download from database.
    - True. Download and convert coordinate system. Please define the input and output of the coordinate system. 
"""

import pandas as pd
from pyproj import Proj, transform
import time

import psycopg2
import psycopg2.extras
import pandas.io.sql as sqlio

import progressbar 

output_FileName = 'sla_address_transformed.csv'
table_slaAddress = 'synpop30.sla_addresses'
table_Taz = 'synpop30.taz'
reprojection = True

def main():
    #Define our connection string
    conn_string = "host='172.25.184.156' dbname='simmobility_l2nic2b' user='postgres' password='d84dmiN'"
    
    # print the connection string we will use to connect
    print ("Connecting to database\n	->%s" % (conn_string))
    
    # get a connection, if a connect cannot be made an exception will be raised here
    conn = psycopg2.connect(conn_string)
    df_postcode = loadtable_sla_address(conn)
    if reprojection is True:
        df_postcode_transformed = proj_transform(df_postcode)
    else:
        df_postcode_transformed = df_postcode
    df_postcode_transformed.to_csv(output_FileName, index = False)
    
    conn.close()

def proj_transform(df):
    # please define the in
    svy21 = 'epsg:3414'
    utm48n = 'epsg:32648'
    
    inPostcodeProj = Proj(init=svy21) 
    outPostcodeProj = Proj(init=utm48n)
    x_pos = pd.Series()
    y_pos = pd.Series()
    
    bar = progressbar.ProgressBar(maxval=len(df)).start()
    
    for idx, val in enumerate(df['x_coord']):
        #Coordinate conversions. please define the input projection (inPostcodeProj) and output projection (outPostcodeProj)
        x, y = transform(inPostcodeProj,outPostcodeProj,df['x_coord'][idx], df['y_coord'][idx])
        x_pos.set_value(idx, x)
        y_pos.set_value(idx, y)
        bar.update(idx)
    df['x_coord'] = x_pos
    df['y_coord'] = y_pos
    return df

def loadtable_sla_address (conn):
    # Load Table: Node
    sql_postcode = """
    SELECT DISTINCT sla_postcode, b.name as postcode_zone, x_coord, y_coord FROM
    """ + table_slaAddress + """ a LEFT JOIN 
    """ + table_Taz + """ 
    b ON a.taz_id = b.id WHERE x_coord != 0 and y_coord != 0
    """
    df_postcode = sqlio.read_sql_query(sql_postcode, conn)
    return df_postcode
    
if __name__ == "__main__":
    main()
    
