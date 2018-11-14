echo
grep "total number of service vehicles loaded" controller.log > subscription_stat.csv
cat subscription_stat.csv 
echo "======================="
echo -n "Request received: " 
grep "Request received" controller.log | wc -l
echo
echo -n "Rail_SMS Trip converted to Walk: " 
grep "with Walk trip" warn.log | wc -l
echo
echo -n "Pickup: "
grep "Pickup succ" controller.log | wc -l
echo
echo -n "Drop Off: "
grep "Drop-off" controller.log | wc -l
echo
echo -n "Single Ride Request: "
grep SingleRideRequest controller.log |wc -l
echo
grep -o 'maximum of [0-9]\?[0-9]' controller.log | awk '{print $3}' | sed -e '1d' | sort | uniq -c | awk '{print $2,$1}'
echo
echo
grep "Parked at node" controller.log > parking.csv
echo -n "No. of parked vehicle: "
wc -l parking.csv
echo
