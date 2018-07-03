echo
echo "======================="
echo -n "Request received: " 
grep "Request received" controller.log | wc -l
echo
echo -n "Pickup: "
grep "Pickup succ" controller.log | wc -l
echo
echo -n "Drop Off: "
grep "Drop-off" controller.log | wc -l
echo
echo -n "2--> people sharing vehicle: "
grep "total 2" controller.log |wc -l
echo -n "3--> people sharing vehicle: "
grep "total 3" controller.log |wc -l
echo -n "4--> people sharing vehicle: "
grep "total 4" controller.log |wc -l
echo -n "5--> people sharing vehicle: "
grep "total 5" controller.log |wc -l
echo -n "6--> people sharing vehicle: "
grep "total 6" controller.log |wc -l
echo
echo -n "QUEUED REQUESTS and AVAILABLE driver at the end: "
grep "Requests to be scheduled" controller.log |tail -1
echo "========================"
echo -n "Pending requests: "
grep "requests are in the queue" controller.log|tail -1
echo
grep "Parked at node" controller.log > parking.csv
echo -n "No. of parked vehicle: "
wc -l parking.csv
echo
