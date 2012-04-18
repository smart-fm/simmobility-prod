#Various "helper" functions to perform parsing/calculation tasks.


#Useful class: mirrors our "DynamicVector" class from Sim Mobility
class DynamicVector
  def initialize(fromPoint, toPoint)
    @pos = [fromPoint.x.to_f, fromPoint.y.to_f]
    @mag = [toPoint.x-@pos[0], toPoint.y-@pos[1]]

    #Helper variable: prevents loss of direction when scaling to zero.
    @isZero = (toPoint.x==fromPoint.x) and (toPoint.y==fromPoint.y)
  end  

  def getX()
    return @pos[0]
  end
  def getY()
    return @pos[1]
  end
  def getMagnitude()
    return 0 if @isZero
    return Math.sqrt(@mag[0]**2 + @mag[1]**2)
  end

  def scaleTo(value)
    #Will this vector have no size after the scaling operation?
    @isZero = (value==0)
    unless @isZero  #If not, scale it
      #Edge case; only happens if the vector is specifically initialized with no size.
      raise "Can't scale vector; no initial size" if (@mag[0]==0 and @mag[1]==0)

      #Factor in the unit vector in the same step. Equivalent to converting to a unit vector then scaling, but
      #  less likely to introduce errors for small vectors
      factor = value / getMagnitude()
      @mag[0] *= factor
      @mag[1] *= factor
    end
  end
  def translate()
    dX = @isZero ? 0 : @mag[0]
    dY = @isZero ? 0 : @mag[1]
    @pos[0] += dX
    @pos[1] += dY
  end
  def flipRight()
    sign = 1
    newX = @mag[1]*sign
    newY = -@mag[0]*sign
    @mag = [newX, newY]
  end
end

 

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




