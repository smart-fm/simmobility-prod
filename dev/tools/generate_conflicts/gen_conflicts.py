import psycopg2
import sys
import pprint
 
def main():
	#Define our connection string
	conn_string = "host='localhost' dbname='simmobility_db_2014' user='postgres' password='5M_S1mM0bility'"
 
	# print the connection string we will use to connect
	print "Connecting to database\n	->%s" % (conn_string)
 
	# get a connection, if a connect cannot be made an exception will be raised here
	conn = psycopg2.connect(conn_string)
 
	# conn.cursor will return a cursor object, you can use this cursor to perform queries
	cursor = conn.cursor()
	print "Connected!\n"
	
	# execute our Query
	cursor.execute('SELECT * FROM "TurningSection" ');
	
	# retrieve the records from the database
	records = cursor.fetchall()
	
	# print out the records using pretty print
	# note that the NAMES of the columns are not shown, instead just indexes.
	# for most people this isn't very useful so we'll show you how to return
	# columns as a dictionary (hash) in the next example.
	#pprint.pprint(records)
	
	print records[0][0];
	print records[0][1];
 
if __name__ == "__main__":
	main()