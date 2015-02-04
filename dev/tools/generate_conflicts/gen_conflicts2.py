import psycopg2
import psycopg2.extras
import sys
import pprint
import math

conflicts = [  ]

# calculate turning from point distance to conflict point
def calculate_conflict_distance(turning,x,y) :
    # 1.0 calculate turning polyline distance of from point to cross point
    t_dx = turning['from_xpos'] - x;
    t_dy = turning['from_ypos'] - y;
    t_distance = math.sqrt(t_dx*t_dx + t_dy*t_dy)
    return t_distance
  
# check point in the bound of t12
# x,y cross point position
# return False when not valid
def validate_cross_point(t1,t2,x,y):
    # 1.0 calculate line t1 distance of from/to points
    t1_dx = t1['from_xpos'] - t1['to_xpos'];
    t1_dy = t1['from_ypos'] - t1['to_ypos'];
    t1_distance = math.sqrt(t1_dx*t1_dx + t1_dy*t1_dy)
    
    # t2 distance of from/to points
    t2_dx = t2['from_xpos'] - t2['to_xpos'];
    t2_dy = t2['from_ypos'] - t2['to_ypos'];
    t2_distance = math.sqrt(t2_dx*t2_dx + t2_dy*t2_dy)
    
    # distance t1 from point to cross point
    t1_cross_dx = t1['from_xpos'] - x;
    t1_cross_dy = t1['from_ypos'] - y;
    t1_cross_distance = math.sqrt(t1_cross_dx*t1_cross_dx + t1_cross_dy*t1_cross_dy)
    
    offset = 1.0
    # cross point same with from point of turning,no valid
    if t1_cross_distance < offset :
	return False
    

    if t1_cross_distance > t1_distance + offset :
	# cross point outside of the t1
	return False
      
    # distance t2 from point to cross point
    t2_cross_dx = t2['from_xpos'] - x;
    t2_cross_dy = t2['from_ypos'] - y;
    t2_cross_distance = math.sqrt(t2_cross_dx*t2_cross_dx + t2_cross_dy*t2_cross_dy)
    
    if t2_cross_distance > t1_distance + offset:
	# cross point outside of the t1
	return False
    
    return True
  
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
    
    valid = validate_cross_point(turning1,turning2,x,y)
    if valid == True :
      # the cross point is valid,so create conflict
      first_cd = calculate_conflict_distance(turning1,x,y)
      second_cd = calculate_conflict_distance(turning2,x,y)
      cnf = {}
      cnf['first_turning'] = turning1['id']
      cnf['second_turning'] = turning2['id']
      cnf['first_cd'] = first_cd
      cnf['second_cd'] = second_cd
      
      # append to all 
      conflicts.append(cnf)


    
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
	
	print ('conflict:')
	pprint.pprint(conflicts)
	    
if __name__ == "__main__":
	main()