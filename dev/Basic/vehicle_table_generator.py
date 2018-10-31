#!/usr/bin/python
import psycopg2
import random, numpy

drivetrains = ['ICE', 'HEV', 'PHEV', 'BEV', 'FCV']

# postgres db connection settings
HOST = '172.25.184.156'
DBNAME = 'simmobility_virtualcity_sms_casestudy2017'
PORT='5432'
USER = 'postgres'
SCHEMA = "virtual_city"
PASSWORD = "HPCdb@2018"
TAXI_FLEET_TABLE = "taxi_fleet_aditi_new"
# Function for vehicle distribution
def rand_dist(n):
  return random.randint(0,4)

def custom_proportions(n):
  proportions = [0.75,0.20,0.03,0.02,0.00]
  return numpy.random.choice(numpy.arange(0,5), p=proportions)

# can replace with some other vehicle distribution
# VEHICLE_DIST_FUNC = rand_dist
VEHICLE_DIST_FUNC = custom_proportions

def get_vehicle_distribution(f,n):
  for i in range(n):
    yield f(n)

def main():
  conn_string = "host='{}' port='{}' dbname='{}' user='{}'".format(HOST, PORT, DBNAME, USER)
  print "Connecting to database\n -> %s" % (conn_string)

  conn = psycopg2.connect(database=DBNAME, user=USER, host=HOST, port=PORT, password=PASSWORD)
  cursor = conn.cursor()
  
  cursor.execute("DROP TABLE {}.vehicle".format(SCHEMA))
  conn.commit()
  cursor.execute("CREATE TABLE IF NOT EXISTS {}.vehicle (vehicle_id serial PRIMARY KEY, ind_id integer NOT NULL, vehicle_drivetrain varchar NOT NULL,  vehicle_make varchar,  vehicle_model varchar)".format(SCHEMA))
  conn.commit()
  # clear vehicle table
  # print "Deleting vehicle table"
  # cursor.execute("DELETE FROM {}.vehicle".format(SCHEMA))

  # print "Resetting vehicle id sequence"
  # cursor.execute("ALTER SEQUENCE {}.vehicle_id_seq RESTART WITH 1".format(SCHEMA))
  
  # get all user ids
  cursor.execute("SELECT id FROM {}.individual".format(SCHEMA))
  ids = cursor.fetchall()
  vehicle_dist = get_vehicle_distribution(VEHICLE_DIST_FUNC, len(ids))

  INSERT_COMMAND = "INSERT INTO {}.vehicle (ind_id,  vehicle_drivetrain,  vehicle_make,  vehicle_model) VALUES (%s, %s, %s, %s)".format(SCHEMA)

  print "Inserting new vehicles into table"
  for ind_id in ids:
    i = next(vehicle_dist)

    cursor.execute(INSERT_COMMAND, (ind_id[0], drivetrains[i],"","",))

  conn.commit()

  ## get all fleet drivers
  cursor.execute("SELECT driver_id FROM supply.{}".format(TAXI_FLEET_TABLE))
  driver_ids = cursor.fetchall()

  INSERT_COMMAND = "INSERT INTO {}.vehicle (ind_id,  vehicle_drivetrain,  vehicle_make,  vehicle_model) VALUES (%s, %s, %s, %s)".format(SCHEMA)
  print "Inserting new vehicles into table"
  for driver_id in driver_ids:
    if driver_id[0].startswith('SHA'):
      cursor.execute(INSERT_COMMAND,  (driver_id[0], drivetrains[0],"","",)) # assume all electric vehicels
    elif driver_id[0].startswith('AMD'):
      cursor.execute(INSERT_COMMAND,  (driver_id[0], drivetrains[3],"","",)) # assume all electric vehicels
    
  conn.commit()


  cursor.close()
  conn.close()

if __name__ == "__main__":
  main()
