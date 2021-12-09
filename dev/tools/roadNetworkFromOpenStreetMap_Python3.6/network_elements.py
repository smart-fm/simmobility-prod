from shapely.geometry import Point, LineString

class Node:
    def __init__(self,id,type,tLightId,x,y,z=0, osmid=-1):
        self.id = id
        self.type = type
        self.trafficLightId = tLightId
        self.x = x
        self.y = y
        self.z = z
        self.tag = None
        self.osmid = osmid

    def render(self):
        aList = [self.id,self.x,self.y,self.z,self.trafficLightId,self.tag,self.type,self.osmid]
        return aList

    def render2(self):
        aList = [self.id, self.type, self.trafficLightId, self.z, self.tag]
        return aList

    def geo_render(self):
        geo = Point(self.x, self.y)
        return [geo] + self.render2()


class Link:

    def __init__(self,id,roadtype,category,upnode,dnnode,name,numlanes,speedlimit,osm_tag='',oneway='',osmid=-1, segments = None):
        self.id = id
        self.type = roadtype
        self.category = category
        self.upnode = upnode # fromnode
        self.dnnode = dnnode # tonode
        self.fromnode = upnode
        self.tonode = dnnode
        self.name = name
        self.segments = segments
        self.numlanes = numlanes
        self.speedlimit = speedlimit
        self.osm_tag = osm_tag
        self.oneway = oneway
        self.osmid = osmid
        self.tags = None

    def render(self):
        aList = [self.id,self.type,self.category,self.upnode,self.dnnode,self.name,self.tags,self.osmid]
        return aList

class LinkTT:

    def __init__(self,id,mode,starttime,endtime,traveltime,length):
        self.id = id
        self.mode = mode
        self.starttime = starttime
        self.endtime = endtime
        self.traveltime = traveltime
        self.length = length

    def render(self):
        aList = [self.id,self.mode,self.starttime,self.endtime,self.traveltime]
        return aList

    def render2(self):
        aList = [self.id,self.mode,self.starttime,self.endtime,self.traveltime,self.length]
        return aList

class Segment:

    def __init__(self,id,linkid,sequence,numlanes,capacity,speedlimit,tag,category,position,length):
        self.id = id
        self.linkid = linkid
        self.seq = sequence
        self.numlanes = numlanes
        self.capacity = capacity
        self.speedlimit = speedlimit
        self.category = category
        self.tag = tag
        self.position = position
        self.length = length

    def render(self):
        aList = [self.id,self.linkid,self.seq,self.numlanes,int(self.capacity),int(self.speedlimit),self.tag,self.category]
        #aList = ",".join(aList)
        return aList

    def render2(self):
        aList = [self.id,self.linkid,self.seq,self.numlanes,int(self.capacity),int(self.speedlimit),self.tag,self.category,self.length]
        #aList = ",".join(aList)
        return aList

    def geo_render(self):
        geo = LineString([(point['x'], point['y']) for point in self.position])
        return [geo] + self.render2()


class Lane:

    def __init__(self,id,segid,width,vehiclemode=63,buslane=0,canstop=0,canpark=0,hov=0,hasshoulder=0,position=None):
        self.id = id
        self.segid = segid
        self.width = width
        self.canstop = canstop
        self.canpark = canpark
        self.hov = hov
        self.hasshoulder = hasshoulder
        self.buslane = buslane
        self.vehiclemode = vehiclemode
        self.tags = None
        self.position = position


    def render(self):
        aList = [self.id,self.width,self.vehiclemode,self.buslane,self.canstop,self.canpark,self.hov,self.hasshoulder,self.segid,self.tags]
        return aList

    def render2(self):
        aList = [self.id, self.segid]
        # self.width,self.vehiclemode,self.buslane,self.canstop,self.canpark,self.hov,self.hasshoulder,self.segid,self.tags]
        return aList

    def geo_render(self):
        geo = LineString([(point[0], point[1]) for point in self.position])
        return [geo] + self.render()

class Connector: # Connection between two segments.
    def __init__(self,id,fromlane,tolane,fromsegment,tosegment,istrueconn=None, tags=None):
        self.id = id
        self.fromlane = fromlane
        self.tolane = tolane
        self.fromsegment = fromsegment
        self.tosegment = tosegment
        self.istrueconn = istrueconn # indicates whether the from and to lanes are physically connected. 0 - not physically connected, 1 - physically connected
        self.tags = tags
    def render(self):
        return [self.id,self.fromsegment,self.tosegment,self.fromlane,self.tolane,self.istrueconn,self.tags]

class TurningPath:
    def __init__(self,id,from_lane,to_lane,group_id,max_speed, position, tags=None):
        self.id = id
        self.fromlane = from_lane
        self.tolane = to_lane
        self.groupid = group_id
        self.maxspeed = max_speed
        self.position = position
        self.tags = tags

    def render(self):
        return [self.id,self.fromlane,self.tolane,self.groupid,int(self.maxspeed),self.tags]

class SumoTurnPath:
    def __init__(self, fromLink, toLink, node):
        self.node = node
        self.fromLink = fromLink
        self.toLink = toLink
        self.dir = None
        self.toSeg = None
        self.fromSeg = None
        self.fromLane = None
        self.toLane = None

class TurningGroup:
    # Turning group is the data structure in the intersection and it is the collection of turning paths

    def __init__(self,id,nodeid,fromlink,tolink):
        self.id = id
        self.nodeid = nodeid
        self.fromlink = fromlink
        self.tolink = tolink
        self.phases = None
        self.rules = None
        self.visibility = None
        self.tags = None


    def render(self):
        aList = [self.id,self.nodeid,self.fromlink,self.tolink,self.phases,self.rules,self.visibility,self.tags]
        return aList
