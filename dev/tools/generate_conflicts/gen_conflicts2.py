import psycopg2
import psycopg2.extras
import sys
import pprint

conflicts = [ {'first_turning':3,'second_turning':2} ]

# check point in the bound of t1,t2
def validate_cross_point(t1,t2,x,y):
  
def already_has_conflict(turning1,turning2) :
    # Generator Expressions
    res1 = next((item for item in conflicts if (item['first_turning'] == turning1['id'] and item['second_turning'] == turning2['id'])),None)
    res2 = next((item for item in conflicts if (item['first_turning'] == turning2['id'] and item['second_turning'] == turning1['id'])),None)
    
    #print res1,res2
    if res1 is None and res2 is None :
	#print res1,res2,'True'
	return False
    else :
	#print res1,res2,'False'
	return True
  
# calculate conflict point given tow turning section
# return point position
def calculate_conflict_point(turning1,turning2) :
    print ('t1 ',turning1)
    # get two points of t1
    t1_p1 = {}
    t1_p1['x'] = turning1['from_xpos'];
    t1_p1['y'] = turning1['from_ypos'];
    t1_p2 = {}
    t1_p2['x'] = turning1['to_xpos'];
    t1_p2['y'] = turning1['to_ypos'];
    
    print ('t2 ',turning2)
    t2_p1 = {}
    t2_p1['x'] = turning2['from_xpos'];
    t2_p1['y'] = turning2['from_ypos'];
    t2_p2 = {}
    t2_p2['x'] = turning2['to_xpos'];
    t2_p2['y'] = turning2['to_ypos'];
    # y = kx + b
    # 1.0 calculate k of line t1,t2
    k1 = (t1_p1['y'] - t1_p2['y'] )/ (t1_p1['x'] - t1_p2['x']);
    k2 = (t2_p1['y'] - t2_p2['y'] )/ (t2_p1['x'] - t2_p2['x']);
    print ('k1',k1)
    print ('k2',k2)
    # 2.0 calculate b of line t1,t2
    b1 = t1_p1['y'] - k1 * t1_p1['x'];
    b2 = t2_p1['y'] - k2 * t2_p1['x'];
    print ('b1',b1)
    print ('b2',b2)
    # 3.0 calculate cross point of two lines
    x = (b2-b1) / (k1-k2);
    y = k1 * x + b1;
    
    print ('x',x)
    print ('y',y)
    
    validate_cross_point(turning1,turning2,x,y)


    
def generate_conflict(ts,all_turning_section) :
	for other_ts in all_turning_section :
	    # filter itself
	    if other_ts['id'] == ts['id']:
		continue
	    else:
		# check conflict already exist
		is_has = already_has_conflict(ts,other_ts)
		if is_has == False :
		    calculate_conflict_point(ts,other_ts)
	
def main():
	#Define our connection string
	conn_string = "host='localhost' dbname='simmobility_db_2014' user='postgres' password='5M_S1mM0bility'"
 
	# print the connection string we will use to connect
	print "Connecting to database\n	->%s" % (conn_string)
	
	# get a connection, if a connect cannot be made an exception will be raised here
	conn = psycopg2.connect(conn_string)
 
	# conn.cursor will return a cursor object, you can use this query to perform queries
	# note that in this example we pass a cursor_factory argument that will
	# dictionary cursor so COLUMNS will be returned as a dictionary so we
	# can access columns by their name instead of index.
	cursor = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)
 
	cursor.execute('SELECT * FROM "TurningSection" ')
 
 
	# retrieve the records from the database
	records = cursor.fetchall()
	
	# print out the records using pretty print
	# note that the NAMES of the columns are not shown, instead just indexes.
	# for most people this isn't very useful so we'll show you how to return
	# columns as a dictionary (hash) in the next example.
	#pprint.pprint(records)
	
	#print records[0]['id'];
	#print records[0]['from_xpos'];
	
	#for key,value in records[0].iteritems() :
	    #print key,value
	    
	for turning_section in records :
	    generate_conflict(turning_section,records)
	    
if __name__ == "__main__":
	main()