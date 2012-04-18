require 'simmob_classes'
require 'mitsim_classes'
require 'misc_compute'


#The vehicle.out file contains, for vehicles that have reached their destinations, data pertaining to:
#    origin,
#    destination,
#    departure time,
#    arrival time,
#    mileage
#    speed
#(Actually: 11 values +5)


module MS_VehicleParser


def self.read_veh_file(vehFileName, drivers)
  minDepartureTime = -1
  File.open(vehFileName).each { |line|
    if line =~ /([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9]+) +([0-9.]+) +([0-9]+)/
      #Retrieve and check the ID
      id = $1.to_i
      raise "Agent ID expected but doesn't exist: #{id}" unless drivers.has_key? id

      #Retrieve and check the driver.
      dr = drivers[id]
      raise "Agent completed twice: #{id}" if dr.completed
      dr.arrival = $8.to_i
      dr.completed = true
      raise "Origin mismatch for agent: #{id} => #{$4}" if dr.originNode.nodeID != $4
      raise "Dest mismatch for agent: #{id} => #{$5}" if dr.destNode.nodeID != $5
      raise "Departure time mismatch for agent: #{id}" if dr.departure.round != $7.to_i
      raise "Arrived before departure for agent: #{id}" if dr.arrival < dr.departure
      t2 = $2.to_i
      raise "Expected boolean, not: #{t2}" unless t2==0 or t2==1
      dr.tempVeh2 = t2==0 ? false : true
      dr.tempVeh3 = $3.to_i
      dr.tempVeh6 = $6.to_i
      dr.tempVeh9 = $9.to_i
      dr.tempVeh10 = $10.to_f
      t11 = $11.to_i
      raise "Expected boolean, not: #{t11}" unless t11==0 or t11==1
      dr.tempVeh11 = t11==0 ? false : true

      #Calculate min. departure time
      minDepartureTime = dr.departure if minDepartureTime==-1 or dr.departure<minDepartureTime
    else 
      puts "Skipped line: #{line}"
    end
  }
  return minDepartureTime
end

end #module MS_VehicleParser

