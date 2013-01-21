import math
import random
from datetime import timedelta, datetime

id = 200
start_time = "05:38:00"

def createLines(fromnode,tonode,bt,iterval):
    inter = int(interval)
    random_second = inter #random.randrange(inter)
    number_of_psg = 10 #random.randrange(100)
    #print random_second
    newtime = bt
    global id
    lid = id
    for i in range(number_of_psg):  
        newtime = newtime+ timedelta(seconds=random_second)
        print newtime
        newtime_str = newtime.strftime("%H:%M:%S")
        id_s = str(lid)
        id_s2 = str(lid-199)
        line_subtrip = id_s+",1,"+fromnode+",node,"+tonode+",node,9,1,"+newtime_str+"\n"
        fo.write(line_subtrip)
        line_trip = id_s+","+fromnode+",node,"+tonode+",node"+"\n"
        fo2.write(line_trip)
        line_tripchain = id_s2+",1,trip,"+id_s+",0"+"\n"
        fo3.write(line_tripchain)
        lid = lid + 1
    id = lid



base_time = datetime.strptime(start_time, "%H:%M:%S")
print base_time

fo = open("subtrip.csv","w")
fo2 = open("trip.csv","w")
fo3 = open("tripchain.csv","w")
fi = open("input","r")
lines = fi.readlines()
for i in range(len(lines)):
    #print lines[i]
    s = lines[i]
    s = s.strip("\n")
    items = s.split(",")
    from_node = items[0]
    to_node = items[1]
    interval = items[2]
    print "get items: " ,items
    createLines(from_node,to_node,base_time,interval)
    
    

fo.close
fo2.close
fo3.close
fi.close


    
