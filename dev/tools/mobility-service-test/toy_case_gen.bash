#!/bin/bash

# Fill the following parameters and run this script before running SimMobility. 
# The folder from which you run this script is not important.

DB_NAME=simmobility_virtualcity_sms
N_REQS=10000
N_VEHICLES=8000
CTRL_ID=1 #controller id
SEED=0.3
DB_HOST=18.58.5.170

LATEST_REQ=20.25 #After this time, we will not generate any other request
SHIFT=24 #Time (h) at which all the drivers go to bed

#We will subsample requests and drivers from these two tables (must exist already)
SOURCE_DAS=demand_eytan.das_mitei_sd_dec17_ma32_oc15_spl0
SOURCE_FLEET=supply.taxi_fleet_dec_17

#Be sure you are referring to these two procedures in the simrun_Mid.xml
DEMAND_PROCEDURE=get_persons_between_toy_case
FLEET_PROCEDURE=get_taxi_fleet_toy_case






## You can ignore all the rest (it will do the necessary stuff, don't need to worry about it)


PSQLCONN="psql -h $DB_HOST --user=postgres -d $DB_NAME -c"
DAS_TABLE=demand.das_toy_case
FLEET_TABLE=supply.taxi_fleet_toy_case

COMMAND="SELECT setseed($SEED);"
COMMAND=$COMMAND" DROP TABLE IF EXISTS $DAS_TABLE ;"
COMMAND=$COMMAND" CREATE TABLE $DAS_TABLE AS (SELECT * FROM $SOURCE_DAS WHERE stop_mode LIKE 'SMS' and tour_no = 1 and stop_no = 1 and prev_stop_departure_time <= $LATEST_REQ ORDER BY random() LIMIT $N_REQS) ;"
COMMAND=$COMMAND"DROP TABLE IF EXISTS $FLEET_TABLE ;"
COMMAND=$COMMAND" CREATE TABLE $FLEET_TABLE AS (SELECT * FROM $SOURCE_FLEET LIMIT $N_VEHICLES) ;"
COMMAND=$COMMAND" UPDATE $FLEET_TABLE SET subscribed_controllers=$CTRL_ID, shift_duration=$SHIFT;"
PGPASSWORD=ITSLab2016! $PSQLCONN "$COMMAND"






COMMAND="DROP FUNCTION IF EXISTS $DEMAND_PROCEDURE(numeric, numeric);"
COMMAND=$COMMAND" CREATE OR REPLACE FUNCTION $DEMAND_PROCEDURE( "
COMMAND=$COMMAND"    IN start_time numeric,"
COMMAND=$COMMAND"    IN end_time numeric)"
COMMAND=$COMMAND"  RETURNS TABLE(person_id character varying, tour_no integer, tour_type character varying, stop_no integer, stop_type character varying, stop_location integer, stop_mode character varying, primary_stop boolean, arrival_time numeric, departure_time numeric, prev_stop_location integer, prev_stop_departure_time numeric, prev_stop_taz integer, stop_taz integer) AS"
COMMAND=$COMMAND" \$BODY\$ select person_id, tour_no, tour_type, stop_no, stop_type, stop_location, stop_mode, primary_stop, arrival_time, departure_time, prev_stop_location, prev_stop_departure_time, prev_stop_zone, stop_zone from $DAS_TABLE where person_id in (select person_id from $DAS_TABLE where prev_stop_departure_time between \$1 and \$2) order by person_id, tour_no, stop_no; \$BODY\$"
COMMAND=$COMMAND"  LANGUAGE sql VOLATILE"
COMMAND=$COMMAND"  COST 100"
COMMAND=$COMMAND"  ROWS 1000;"
COMMAND=$COMMAND" ALTER FUNCTION $DEMAND_PROCEDURE(numeric, numeric) OWNER TO postgres;"
PGPASSWORD=ITSLab2016! $PSQLCONN "$COMMAND"





COMMAND="DROP FUNCTION IF EXISTS $FLEET_PROCEDURE(character varying, character varying);"
COMMAND=$COMMAND" CREATE OR REPLACE FUNCTION $FLEET_PROCEDURE("
COMMAND=$COMMAND"     IN begin_time character varying, IN end_time character varying, OUT vehicle_no character varying, OUT driver_id character varying, OUT longtitude double precision, OUT latitude double precision, OUT start_time character varying, OUT subscribed_controllers bigint, OUT shift_duration integer)"
COMMAND=$COMMAND"   RETURNS SETOF record AS"
COMMAND=$COMMAND" \$BODY\$SELECT vehicle_no,driver_id,longtitude_at_start_point,latitude_at_start_point,start_time, cast(1 as bigint), shift_duration FROM $FLEET_TABLE  where subscribed_controllers <> 4 and start_time between \$1 and \$2;"
COMMAND=$COMMAND" \$BODY\$ "
COMMAND=$COMMAND"   LANGUAGE sql VOLATILE"
COMMAND=$COMMAND"   COST 100"
COMMAND=$COMMAND"   ROWS 1000;"
COMMAND=$COMMAND" ALTER FUNCTION $FLEET_PROCEDURE(character varying, character varying) OWNER TO postgres;"
PGPASSWORD=ITSLab2016! $PSQLCONN "$COMMAND"



