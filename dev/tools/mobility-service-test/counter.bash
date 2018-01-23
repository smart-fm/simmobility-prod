#!/bin/bash
# Launch this script as 
#     ./counter.bash <my dev/Basic> folder
OUT_LOCATION=$1




echo "Requests with origin=destination"
grep "with the same origin and destination" $OUT_LOCATION/warn.log | wc -l

echo "Requests sent"
grep "Request sent" $OUT_LOCATION/controller.log | wc -l

echo "Pick-ups"
grep "Pickup succeeded" $OUT_LOCATION/controller.log | wc -l

echo "Drop-offs"
grep "Drop-off" $OUT_LOCATION/controller.log | wc -l

echo "WAIT_SMS lines"
grep "WAIT_SMS" $OUT_LOCATION/traveltime.csv | wc -l




echo "drivers subscribed"
grep   "Subscription received"  $OUT_LOCATION/controller.log | wc -l
#
echo "drivers receiving at least one schedule"
grep "to driver" $OUT_LOCATION/controller.log | awk -F "to driver" '{print $2}'  | sort | uniq | wc -l

#
