#Various "helper" functions to perform parsing/calculation tasks.
 

#Get average and standard deviation 
def GetStdDevAndAverage(list)
  #Sum, average
  sum = 0
  list.each {|item|
    sum += item
  }
  avg = sum / list.length.to_f

  #Sample variance, stdev
  sVar = 0
  list.each {|item|
    sVar += (item-avg)**2
  }
  sVar /= list.length.to_f
  stDev = Math.sqrt(sVar)
  
  return [avg, stDev]
end


#Parse a floating point number in scientific notation.
# NOTE: There is probably a way to do this in pure ruby.
def ParseScientificToFloat(str)
  if str =~ /([0-9]+\.[0-9]+)e([-+][0-9]+)/
    base = $1.to_f
    subscr = $2.to_f
    return base * 10.0 ** subscr
  else
    raise "String is not in scientific form: " + str
  end
end


#Distance formula
def Distance(point1, point2)
  return 0 if (point1.x==point2.x) and (point1.y==point2.y)  #Shortcut
  dX = point2.x - point1.x
  dY = point2.y - point1.y
  return Math.sqrt(dX**2 + dY**2)
end




