import networkx as nx
import osmnx as ox
import time
import numpy as np
from itertools import chain
from numpy.linalg import norm
from network_elements import*
from collections import defaultdict
from shapely.geometry import Point
from shapely.geometry import LineString
from shapely.geometry import Polygon
from shapely.geometry import MultiPolygon
from shapely.ops import unary_union
from geopy.distance import great_circle
import csv,pdb,os,itertools,copy,math
import unicodedata

import query_osm as qr

SEGMENT_LOWER_BOUND = 20
MERGING_DIST_THRESHOLD = 20
LANE_WIDTH = 3.7 # meters

DEFAULT_LINK_OSM_ATTR = {
'width': None,
'lanes': None,
'name': None,
'maxspeed': None,
'highway': None,
'oneway': None,
'osmid': None
}

DEFAULT_LINK_ATTR = {
'width':6,
'lanes':1,
'name':'noname',
'road_type':1,
'category':1,
'maxspeed': None,
'highway': None,
'oneway': None,
'osmid':None
}

DEFAULT_SEGMENT_ATTR = {
'lanes' : 1,
'capacity' : 1000,
'maxspeed': 60,
'category': 2, #linkCat (metadata)--> A,B,C,D,E
'tag':"",
}

assert DEFAULT_LINK_ATTR['lanes'] == DEFAULT_SEGMENT_ATTR['lanes']

# G = nx.read_gpickle(GRAPH_FILE)

def getNodeTypes(G):
    sourceNodes = set()
    sinkNodes = set()
    mergingNodes = set()
    divergingNodes = set()
    intersectionNodes = set()
    trafficLiNodes = set()
    uniNodes = set()

    for node, data in G.nodes(data=True):
        try:
            if data["trafficLid"] != 0:
                trafficLiNodes.append(node)
        except:
            pass
        outEdges = G.out_degree(node)
        inEdges = G.in_degree(node)
        neighbors = set(list(G.predecessors(node)) + list(G.successors(node)))
        num_neighbors = len(neighbors)
        d = G.degree(node)

        if node in neighbors:
            # self-loop
            intersectionNodes.add(node)
        elif inEdges == 0:
            sourceNodes.add(node)
        elif outEdges == 0:
            sinkNodes.add(node)
        elif num_neighbors==2 and (d==2 or d==4):
            uniNodes.add(node)
        elif inEdges == 2 and outEdges == 1:
            mergingNodes.add(node)
        elif inEdges == 1 and outEdges == 2:
            divergingNodes.add(node)
        else:
            intersectionNodes.add(node)
    return {'source':sourceNodes,
            'sink':sinkNodes,
            'uniNodes':uniNodes, # to be removed
            'merging': mergingNodes,
            'diverging': divergingNodes,
            'intersection':intersectionNodes,
            'trafficL':trafficLiNodes}

def add_edges_for_dead_ends(G):
    # TODO:
    # How should we handle dead nodes and subgraphs that do not any outgoing edges?
    # Currently we have dead end nodes, and we can solve this by adding outgoing links.
    # However, there could be small loops which do not have any outgoing edges
    # to the rest of the network. If we have bigger such sub-graphs, then
    # it is a serious issue. It means that is is impossible to travel from one part
    # to another part of the network.
    pass
    return G


def fix_short_links(linkGraph):
    print('link ++++++++++++++++++++++')
    # for nodeId, data in linkGraph.nodes(data=True):
    #     print(nodeId, data)
    print("num links ", len(linkGraph.edges()))
    print("num nodes ", len(linkGraph.nodes))
    # print("nodes ", linkGraph.nodes)
    removed_nodes = []
    left_nodes = []

    too_close_pairs = set()
    for u,v,key,data in linkGraph.edges(keys=True, data=True):
        # We cannot remove (contract) nodes in for loop.
        if data['length'] < 1:
            if u > v:
                too_close_pairs.add((u,v))
            else:
                too_close_pairs.add((v,u))

    representive  = {}
    for u,v in too_close_pairs:
        while u in representive:
            u = representive[u]
        while v in representive:
            v = representive[v]
        linkGraph = nx.contracted_nodes(linkGraph, u, v, self_loops=False)
        representive[v] = u # v will be represented by v

    print('after merge ++++++++++++++++++++++')
    print("num links ", len(linkGraph.edges()))
    print("removed nodes ", len(removed_nodes))
    print("num nodes ", len(linkGraph.nodes))
    try:
        print("2-3163635428", linkGraph.nodes[3163635428])
    except:
        print("2-3163635428 not 2")
    # for u,v,key,data in linkGraph.edges(keys=True, data=True):
    # for nodeId, data in linkGraph.nodes(data=True):
    #     print(nodeId, data)
    #     print(data['length'])
    # KeyError: 3163635428
    # (2, {'osmid': -1, 'contraction': {3163635395: {'y': 39.4941045, 'x': -76.3163574, 'osmid': 3163635395, 'contraction': {3163635428: {'y': 39.4941064, 'x': -76.3163575, 'osmid': 3163635428}}}}})

    return linkGraph

# Modified OSMnx method. We use our node_type function instead of is_endpoint.
def get_paths_to_simplify(G, uniNodesSet):
    """
    Create a list of all the paths to be simplified between uniNodesSet nodes.

    The path is ordered from the first endpoint, through the interstitial nodes,
    to the second endpoint.

    Parameters
    ----------
    G : networkx multidigraph

    Returns
    -------
    paths_to_simplify : list
    """

    # first identify all the nodes that are endpoints
    start_time = time.time()
    endpoints = set([node for node in G.nodes() if not node in uniNodesSet])
    paths_to_simplify = []

    # for each endpoint node, look at each of its successor nodes
    for node in endpoints:
        for successor in G.successors(node):
            if successor not in endpoints:
                # if the successor is not an endpoint, build a path from the
                # endpoint node to the next endpoint node
                try:
                    path = ox.build_path(G, successor, endpoints, path=[node, successor])
                    paths_to_simplify.append(path)
                except RuntimeError:
                    print("RecursionError")
    return paths_to_simplify

def build_linkGraph(G, uniNodesSet):
    """

    Create a new link graph removing all nodes that are not intersections or
    dead-ends. Each edge (link) will have intersection or dead-end nodes.

    Parameters
    ----------
    G : graph ( will not be modified)

    Returns
    -------
    networkx multidigraph
    """

    linkGraph = G.copy()
    all_nodes_to_remove = []
    all_edges_to_add = []

    # construct a list of all the paths that need to be simplified
    paths = get_paths_to_simplify(G, uniNodesSet)
    # print('paths', paths)

    start_time = time.time()
    for path in paths:
        edge_attributes = {}
        all_edges = zip(path[:-1], path[1:])
        for u, v in all_edges:
            # each edge:
            # {'osmid': 8614961, 'oneway': True, 'lanes': '1', 'name': 'Pearl Street', 'highway': 'tertiary', 'width': '12.2', 'length': 72.679754393141266}

            # there shouldn't be multiple edges between interstitial nodes
            if not G.number_of_edges(u=u, v=v) == 1:
                # OSM data has multiple edges !!!
                print('Multiple edges between "{}" and "{}" found when merging path ends: '.format(u, v))
            edge = G.edges[u, v, 0]
            for key in edge:
                if key in edge_attributes:
                    edge_attributes[key].append(edge[key])
                else:
                    edge_attributes[key] = [edge[key]]

        # Combine intermediate edge attributes and build segments.
        for key in edge_attributes:
            # print(type(edge_attributes), edge_attributes)
            # don't touch the length attribute, we'll sum it at the end
            if key=='name':
                try:
                    edge_attributes[key] = edge_attributes[key][0].encode('utf-8')
                    #edge_attributes[key] = unicodedata.normalize('NFKD', edge_attributes[key][0]).encode('ascii', 'replace')
                except:
                    edge_attributes[key] = "noname"
            # try:
            #     name = data['name'].encode('utf-8')
            # except:
            #     name = DEFAULT_LINK_ATTR['name']
            # try:
            #     lanes = int(data['lanes'])
            # except:
            #     lanes = DEFAULT_LINK_ATTR['lanes']
            elif key=='maxspeed':
                speeds = [int(s) for s in edge_attributes[key][0].split() if s.isdigit()]
                if len(speeds) > 0:
                    edge_attributes[key] = int(speeds[0])
            elif not key == 'length':
                # keep one of each value in this attribute list,
                # consolidate it to the single value (the zero-th)
                edge_attributes[key] = edge_attributes[key][0]

        edge_attributes['length'] = sum(edge_attributes['length'])
        edge_attributes['coordinates'] = path
        # add the nodes and edges to their lists for processing at the end
        all_nodes_to_remove.extend(path[1:-1])
        all_edges_to_add.append({'origin':path[0],
                                 'destination':path[-1],
                                 'attr_dict':edge_attributes})

    # finally remove all the interstitial nodes between the new edges
    linkGraph.remove_nodes_from(set(all_nodes_to_remove))
    for edge in all_edges_to_add:
        linkGraph.add_edge(edge['origin'], edge['destination'], **edge['attr_dict'])

    # set all attributes, using default values
    for u,v,key,data in linkGraph.edges(keys=True, data=True):
        if not 'coordinates' in data:
            linkGraph.edges[u,v,key]['coordinates'] = [u,v]
        for attr in DEFAULT_LINK_OSM_ATTR:
            if attr not in data:
                linkGraph.edges[u,v,key][attr] = DEFAULT_LINK_ATTR[attr]


    msg = '--------------  Link graph (from {:,} to {:,} nodes and from {:,} to {:,} edges) in {:,.2f} seconds'
    # print(msg.format(len(list(G.nodes())), len(list(linkGraph.nodes())), len(list(G.edges())), len(list(linkGraph.edges())), time.time()-start_time))

    return linkGraph

# cut from the start coordinate.
def interCoord(fromPoint, toPoint, lenght, G, fromTailSide=True):
    proportion = lenght/G.edges[fromPoint, toPoint, 0]['length']
    dx = (G.nodes[toPoint]['x'] - G.nodes[fromPoint]['x']) * proportion
    dy = (G.nodes[toPoint]['y'] - G.nodes[fromPoint]['y']) * proportion
    if fromTailSide:
        x,y = G.nodes[fromPoint]['x'] + dx, G.nodes[fromPoint]['y'] + dy
    else:
        x,y = G.nodes[toPoint]['x'] - dx, G.nodes[toPoint]['y'] - dy
    # assert 0.1 > (distanceNodes(fromPoint, toPoint, G) - distance(fromPoint, (y,x), G) - distance(toPoint, (y,x), G))
    return x,y

def coordsLen(coordinates, G):
    totalLen = 0
    geoLen = 0
    for tail, head in zip(coordinates[:-1], coordinates[1:]):
        totalLen += G.edges[tail, head, 0]['length']
        geoLen += distanceNodes(tail, head, G)
    # assert 0.1 > totalLen - geoLen
    return totalLen


def shortenFromTail(ends, G, originalG, size):
    coordinates = G.edges[ends[0], ends[1], ends[2]]['coordinates']
    length = 0
    head = 1 # first node will be definitely get removed.
    while head < len(coordinates):
        next_edge = originalG.edges[coordinates[head-1], coordinates[head], 0]['length']
        if size < length + next_edge:
            break
        length  += next_edge
        head += 1

    if head == len(coordinates):
        msg = 'Cut {:,} is too long for the link between {:,} and {:,} nodes, size of {:,}'
        # print(msg.format(size, ends[0], ends[1], length))
    else:
        # shorten from the TAIL node side
        newx, newy = interCoord(coordinates[head-1], coordinates[head], size-length, originalG, fromTailSide=True)
        originalG.max_node_id += 1
        new_coordinates = [originalG.max_node_id] + coordinates[head:]
        G.edges[ends[0], ends[1], 0]['coordinates'] = new_coordinates

        # add node info to the original graph
        originalG.add_node(originalG.max_node_id, x=newx, y=newy)
        originalG.add_edge(originalG.max_node_id, coordinates[head], length=distanceNodes(originalG.max_node_id, coordinates[head], originalG))

        # Validate
        before_shorten = coordsLen(coordinates, originalG)
        after_shorten = coordsLen(new_coordinates, originalG)
        # assert 0.1 > before_shorten - after_shorten - size


def shortenFromHead(ends, G, originalG, size):
    coordinates = G.edges[ends[0], ends[1], ends[2]]['coordinates']
    length = 0
    tail = len(coordinates) - 2 # first node will be definitely get removed.
    while tail >= 0:
        next_edge = originalG.edges[coordinates[tail], coordinates[tail+1], 0]['length']
        if size < length + next_edge:
            break
        length  += next_edge
        tail -= 1

    if tail < 0:
        msg = 'Cut {:,} is too long for the link between {:,} and {:,} nodes, size of {:,}'
        # print(msg.format(size, ends[0], ends[1], length))
    else:
        # shorten from the HEAD node side
        newx, newy = interCoord(coordinates[tail], coordinates[tail+1], size-length, originalG, fromTailSide=False)
        originalG.max_node_id += 1
        new_coordinates = coordinates[:tail+1] +[originalG.max_node_id]
        G.edges[ends[0], ends[1], 0]['coordinates'] = new_coordinates

        # add node info to the original graph
        originalG.add_node(originalG.max_node_id, x=newx, y=newy)
        originalG.add_edge(coordinates[tail], originalG.max_node_id, length=distanceNodes(originalG.max_node_id, coordinates[tail], originalG))

        # Validate
        before_shorten = coordsLen(coordinates, originalG)
        after_shorten = coordsLen(new_coordinates, originalG)
        # assert 0.1 > before_shorten - after_shorten - size

def distanceNodes(node1, node2, G):
    return great_circle((G.nodes[node1]['y'], G.nodes[node1]['x']), (G.nodes[node2]['y'], G.nodes[node2]['x'])).meters

def distance(node1, coord, G):
    return great_circle((G.nodes[node1]['y'], G.nodes[node1]['x']), (coord[0], coord[1])).meters

ROAD_CATEGORY = {
"motorway":1, #EXPRESSWAY
"trunk":1, #EXPRESSWAY
"primary":2, #Urban
"secondary":3, #Urban
"tertiary": 4, #Access
"motorway_link":6,
"trunk_link": 6, #sliproad
"primary_link":6,
"secondary_link":6,
"tertiary_link":6,
}

# mph --> km/hr
# 65 mph is the maximum speed limit that can be established on any
# highway;
# 55 mph on highways that are not interstate highways or expressways;
# 50 mph on undivided highways except as noted below;
# 35 mph on divided highways in residential districts;
# 30 mph on highways in business districts or on undivided highways
# in residential districts;
# 15 mph in alleys in Baltimore County.

ROAD_CATEGORY_SPEED_LIMIT = {
"motorway": 100, #EXPRESSWAY
"trunk": 90, #EXPRESSWAY
"primary": 80, #Urban
"secondary": 70, #Urban
"tertiary": 50, #Access
"motorway_link": 100,
"trunk_link": 90, #sliproad
"primary_link": 80,
"secondary_link": 70,
"tertiary_link": 50,
}

# "highway"]["area"!~"yes"]["highway"~"motorway|trunk|primary|secondary|tertiary|motorway_link|trunk_link|primary_link|secondary_link|tertiary_link"
def constructNodesLinks(G, originalG, tempnodeDict):

    nodes = {}
    links = {}
    link_id = 1

    for fromnode, tonode, key, data in G.edges(keys=True,data=True):

        '''
        #chuyaw removed this. It will return Ture/False value in oneway tag
        for attr in ['name', 'oneway', 'highway']:
            print(attr)
            try:
                data[attr] = data[attr].encode('utf-8')
            except:
                data[attr] = ''
        '''

        try:
            lcategory = ROAD_CATEGORY[data['highway']]
        except:
            lcategory = 1

        try:
            data['lanes'] = int(data['lanes'])
        except:
            data['lanes'] = 2

        try:
            data['osmid'] = int(data['osmid'])
        except:
            data['osmid'] = -1


        try:
            lspeedlimit = data['maxspeed']
            if not isinstance(lspeedlimit,int):
                speeds = [int(s) for s in lspeedlimit.split() if s.isdigit()]
                if len(speeds) > 0:
                    lspeedlimit = int(speeds[0])
                else:
                    lspeedlimit = 60
        except:
            lspeedlimit = 60
        link = Link(link_id,
                    lcategory,
                    lcategory,
                    fromnode,
                    tonode,
                    data['name'],
                    data['lanes'],
                    lspeedlimit,
                    osm_tag=data['highway'],
                    oneway=data['oneway'],
                    osmid=data['osmid'])
        G.edges[fromnode, tonode, key]['id'] = link_id
        G.edges[fromnode, tonode, key]['name'] = data['name']
        G.edges[fromnode, tonode, key]['lanes'] = data['lanes']
        links[link_id] = link
        link_id += 1

    coordinates = nx.get_edge_attributes(G,'coordinates')
    # for u,v,data in G.edges():

    max_node_id = max(originalG.nodes())
    originalG.max_node_id = max_node_id

    new_nodes = []
    if 2 in G.nodes:
        print("3-2", G.nodes[2])
    print(len(G.nodes))
    # to_remove = set()
    # for nodeId in G.nodes():
    #     if len(list(G.neighbors(nodeId))) == 0:
    #         to_remove.add(nodeId)
    # print('to_remove ', to_remove)
    # G.remove_nodes_from(to_remove)

    for nodeId, data in G.nodes(data=True):
        # print('node id, max node id-------: ', nodeId, originalG.max_node_id)
        nodeType = 0
        trafficLid = 0
        if nodeId in tempnodeDict['intersection']:
            nodeType = 2
        elif nodeId in tempnodeDict['diverging'] or tempnodeDict['merging']:
            nodeType = 3
        elif nodeId in tempnodeDict['source'] or tempnodeDict['sink']:
            nodeType = 1

        try:
            if data["trafficLid"] != 0:
                trafficLiNodes.append(node)
        except:
            pass

        try:
            data["osmid"] = int(osmid['osmid'])
        except:
            data["osmid"] = -1

        try:
            node = Node(nodeId, nodeType, trafficLid, data['x'], data['y'], osmid=data["osmid"])
        except:
            print("what??????????")
            # 37382561
            # 3163635428
            print(nodeId, nodeType, data)
            node = Node(nodeId, nodeType, trafficLid, data['x'], data['y'], osmid=data["osmid"])
            # (2, {'osmid': -1, 'contraction': {3163635395: {'y': 39.4941045, 'x': -76.3163574, 'osmid': 3163635395, 'contraction': {3163635428: {'y': 39.4941064, 'x': -76.3163575, 'osmid': 3163635428}}}}})
            break
        nodes[nodeId] = node

        # turning path radius for intersections

        if nodeId: #in tempnodeDict['uniNodes']:
            in_edges = [((u,v, key), int(data['lanes'])) for u,v,key,data in G.in_edges(nodeId, keys=True, data=True)]
            out_edges = [((u,v, key), int(data['lanes'])) for u,v,key,data in G.out_edges(nodeId, keys=True, data=True)]
            if len(in_edges) + len(out_edges) > 0:
                # print('no neighbor', nodeId, G.nodes[nodeId])
                maxRadius = max([ edge[1] * LANE_WIDTH for edge in in_edges + out_edges]) # width * lanes
                for ends, _ in in_edges:
                     new_node = shortenFromHead(ends, G, originalG, maxRadius)
                     new_nodes.append(new_node)
                for ends, _ in out_edges:
                    new_node = shortenFromTail(ends, G, originalG, maxRadius)
                    new_nodes.append(new_node)
    print('nodes -----------------------------------------', len(nodes))
    # print(nodes)



    return nodes, links


def offsettingLinks(G, originalG):

    geo_scale = 0.000008
    DefaultWidth = 3.5 * geo_scale

    for u,v,key,linkData in G.edges(keys=True,data=True):

        if linkData['oneway'] == True:
            # one-way link. Skip this step. no need offsetting
            pass
        else:
            # both-way link. offsetting this link
            coords = linkData['coordinates']
            link_id = linkData['id']
            nlanes = linkData['lanes']
            coords_data = [originalG.nodes[node] for node in coords]

            OriLinkCoords = [(float(data['x']), float(data['y'])) for i, data in enumerate(coords_data)]

            offsetwidth = (nlanes * DefaultWidth)
            OffsetLinkCoords = offsetPolyLine(OriLinkCoords,  (-0.5)*offsetwidth)

            polypointseq = 0
            for node in coords:
                linkVertice = originalG.nodes[node]
                print('!!! OLD LINK: ',linkVertice)
                xlinkVertice = linkVertice['x']
                ylinkVertice = linkVertice['y']

                newxlinkVertice = OffsetLinkCoords[polypointseq][0]
                newylinkVertice = OffsetLinkCoords[polypointseq][1]
                polypointseq +=  1
                originalG.nodes[node]['x'] = newxlinkVertice
                originalG.nodes[node]['y'] = newylinkVertice

                #originalG.nodes[node]['x'] = 103.82265493648787
                #originalG.nodes[node]['y'] = 1.2886989281208765

            coords_data_new = [originalG.nodes[node] for node in coords]
            print('!!!! NEW Link id ', link_id, ' Coord :', coords_data_new)

    return G, originalG

def geoFromPathGraph(G, originalG):
    pathGeo = {}
    coords = nx.get_edge_attributes(G,'coordinates')
    for link in coords:
        pathGeo[link] = LineString([Point((originalG.nodes[node]['x'], originalG.nodes[node]['y'])) for node in coords[link]])
    nx.set_edge_attributes(G, 'coordinates', pathGeo)


def attr(data, attr):
    if attr in data:
        if attr == 'width' or attr == 'lanes':
            try:
                return float(data[attr])
            except:
                return DEVAULT_OSM_VALUE[attr]
        return data[attr]
    else:
        return DEVAULT_OSM_VALUE[attr]

def attr2(data, attr):
    if attr in data:
        if attr == 'width' or attr == 'lanes':
            try:
                return float(data[attr])
            except:
                return DEVAULT_OSM_VALUE[attr]
        return data[attr]
    else:
        return DEVAULT_OSM_VALUE[attr]

def attribute(att_string, sub_attribute, parent_attribute, default_attribute):
    if att_string in sub_attribute:
        return sub_attribute[att_string]
    elif att_string in parent_attribute:
        return parent_attribute[att_string]
    return default_attribute[att_string]

def constructSegments(linkGraph, linkDict, originalG, SEGMENT_LOWER_BOUND=20):
    linkToSeg = {}
    linkToData = {}
    segToLink = {}
    toSegment = {}
    SEGMENT_ID = 1

    for u,v,key,linkData in linkGraph.edges(keys=True,data=True):
        coords = linkData['coordinates']
        link_id = linkData['id']
        link_oneway = linkData['oneway']
        linkToData[link_id] = linkData
        linkToSeg[link_id] = []
        total_len = sum([originalG.edges[coords[i], coords[i+1], 0]['length'] for i in range(len(coords)-1)]) # after shortened
        tail = 0
        segment_len = 0
        acc_len = 0
        segment_coords = [coords[tail]]
        segment_attributes = {}
        SEQ = 1
        while tail < len(coords)-1:
            data = originalG.edges[coords[tail], coords[tail+1], 0]
            for key in data:
                segment_attributes[key] = data[key] # get whatever available
            segment_len += data['length']
            acc_len += data['length']
            if total_len - acc_len < SEGMENT_LOWER_BOUND or tail == len(coords)-2:
                # combine all leftover coordinates if they are not enough for a segment.
                segment_coords += coords[tail+1:]
                toSegment[SEGMENT_ID] = (segment_coords, segment_attributes, segment_len, link_id, SEQ, link_oneway)
                linkToSeg[link_id].append(SEGMENT_ID)
                SEGMENT_ID += 1
                SEQ += 1
                # print("case1",segment_len )
                break
            elif segment_len > SEGMENT_LOWER_BOUND:
                # a new segment
                segment_coords.append(coords[tail+1])
                toSegment[SEGMENT_ID] = (segment_coords, segment_attributes, segment_len, link_id, SEQ, link_oneway)
                linkToSeg[link_id].append(SEGMENT_ID)
                segment_coords = [coords[tail+1]]
                segment_attributes = {}
                # print("case2",segment_len )
                segment_len = 0
                SEGMENT_ID += 1
                SEQ += 1
            else:
                # print("case3-coord",segment_len )
                segment_coords.append(coords[tail+1])
            tail += 1

    segments = {}
    running_linkid = 0
    isonewaytag = False
    for segment_id in toSegment:
        coords, segment_attributes, length, link_id, seq, link_oneway = toSegment[segment_id]
        # no oneway tag, set to FalseOneway, next segments in the link, set the same oneway tag
        if ((running_linkid == 0) and ('oneway' not in segment_attributes)) or ((running_linkid != link_id) and ('oneway' not in segment_attributes)):
            segment_attributes['oneway'] = False
        if (running_linkid == 0) or (running_linkid != link_id):
            isonewaytag = segment_attributes['oneway']
            running_linkid = link_id
        coords_data = [originalG.nodes[node] for node in coords]

        position_points = [ {'x': data['x'],
                            'y': data['y'],
                            'seq': i} for i, data in enumerate(coords_data)]

        for attr in DEFAULT_SEGMENT_ATTR:
            if attr not in segment_attributes:
                if attr in linkToData[link_id]:
                    segment_attributes[attr] = linkToData[link_id][attr]
                else:
                    segment_attributes[attr] = DEFAULT_SEGMENT_ATTR[attr]
            if attr == 'maxspeed':
                try:
                    if not isinstance(segment_attributes['maxspeed'],int):
                        speeds = [int(s) for s in segment_attributes['maxspeed'].split() if s.isdigit()]
                        if len(speeds) > 0:
                            segment_attributes[attr] = int(speeds[0])
                        else:
                            segment_attributes[attr] = DEFAULT_SEGMENT_ATTR[attr]
                except:
                    segment_attributes[attr] = DEFAULT_SEGMENT_ATTR[attr]


        '''
        if 'oneway' not in segment_attributes:
            segment_attributes['oneway'] = False
        '''

        seg = Segment(
                segment_id,
                link_id,
                seq,
                linkDict[link_id].numlanes, # force to the same number of lanes for segments in the same link
                int(segment_attributes['capacity']),
                linkDict[link_id].speedlimit,
                #str(segment_attributes['oneway']),
                str(isonewaytag),
                int(linkDict[link_id].category),
                position_points,
                length)
        segToLink[segment_id] = link_id
        segments[segment_id] = seg

    return segments, segToLink, linkToSeg

def offsettingSegments(segments, segToLink):

    geo_scale = 0.000008
    Defaultwidth = float(4.5) * geo_scale

    for segid, segment in segments.items():
        if segment.tag == 'True':
            # one-way segment.
            #print(segid, 'is one way')
            pass
        else:
            #print(segid, 'is both way')
            #both-way segments. need offsetting
            segPos = segment.position
            segPos = [(float(item['x']), float(item['y'])) for item in segPos]
            NewSegPos = offsetPolyLine(segPos, ((segment.numlanes/2)) * (Defaultwidth))

            NewSegPosList = []
            seq =0
            for pos in NewSegPos:
                NewSegPosList.append({'x':pos[0],'y':pos[1],'seq':seq})
                seq +=1
            segment.position = NewSegPosList
    segment.tag = ''
    return segments

def setLinkSegmentAttr(segments, links, linkToSeg):
    # Contructing Link Travel Time Table
    linktts = {}
    for link_id in links:
        links[link_id].segments = linkToSeg[link_id]
        linkTime,linkLength = 0,0
        for segid in links[link_id].segments:
            segLength = segments[segid].length
            segTime =  segLength/(segments[segid].speedlimit*(0.277778))
            linkLength += segLength
            linkTime += segTime
        #length = getLength(pointList)
        #traveltime = length/20.833    #75kmph in m/s
        linktts[link_id] = LinkTT(link_id,mode="Car",starttime="00:00:00",endtime="23:59:59",traveltime=linkTime,length=linkLength)
    return linktts


def processAndAddConnections(nodes,tempnodeDict,links,segments,lanes,laneconnections,turninggroups,segToLink):
    '''
    '''
    laneswithDwConn = []  # lanes with downstream connections
    laneswithUpConn = []  # lanes with upstream connections

    for connid,conn in laneconnections.iteritems():
        fLane,tLane = conn.fromlane,conn.tolane
        laneswithDwConn.append(fLane)
        laneswithUpConn.append(tLane)
    laneswithDwConn,laneswithUpConn = set(laneswithDwConn),set(laneswithUpConn)

    totLaneList = lanes.keys()
    terlaneswithoutDwConn, terlaneswithoutUpConn = set(),set()

    #---Getting the terminal lanes ---lanes at the end and begining of links--- that are not connected to sink (source) node and no downstream (upstream) connection
    for linkid,link in links.iteritems():
        tnode,hnode = link.upnode,link.dnnode
        #---first segment
        segid = link.segments[0]
        segment = segments[segid]
        #for i in xrange(segment.numlanes):
        for i in range(segment.numlanes):
            laneid = str(int(segid)*100 + i)
            if laneid not in laneswithUpConn and tnode not in tempnodeDict['source']:
                terlaneswithoutUpConn.add(laneid)
        #---last segment
        segid = link.segments[-1]
        segment = segments[segid]
        for i in xrange(segment.numlanes):
            laneid = str(int(segid)*100 + i)
            if laneid not in laneswithDwConn and hnode not in tempnodeDict['sink']:
                terlaneswithoutDwConn.add(laneid)


    #----Handling lanes without downstream connections
    unresolvedLanes = []
    connKeylist = laneconnections.keys()
    count = len(connKeylist) + 1 # count and id of the connections
    #---Copying the connections from one of the adjacent lanes
    for laneid1 in terlaneswithoutDwConn:
        resolved = False
        segid1 = laneid1[:-2]
        segid1 = int(segid1)
        for i in xrange(segments[segid1].numlanes):
            laneid2 = str(int(segid1)*100 + i)
            if laneid1!=laneid2:
                connkeys = [key for key in connKeylist if key[0]==laneid2]
                if connkeys:
                    fromLane,toLane = connkeys[0]
                    tosegid = toLane[:-2]
                    linkid1,toLink = segToLink[segid1],segToLink[tosegid]
                    maxSpeed = min(segments[segid1].speedlimit,segments[tosegid].speedlimit)
                    groupid = turninggroups[(linkid1,toLink)].id
                    connection = LaneConnection(count,laneid1,toLane,segid1,tosegid,isturn=True,maxspeed=maxSpeed,groupid=groupid)
                    #---Constructing the connection polyline
                    firstpoint,secondpoint = lanes[laneid1].position[-1],lanes[toLane].position[0]
                    connection.position = [firstpoint,secondpoint]
                    laneconnections[(laneid1,toLane)] = connection
                    count += 1
                    resolved = True
                    break
        if not resolved:
            unresolvedLanes.append(laneid1)
            # pdb.set_trace()

    print ("Before: ",len(terlaneswithoutDwConn))
    print ("After: ",len(unresolvedLanes), unresolvedLanes)
    return laneconnections


def constructLanes(segments,typeToWidthFname, lefthand_driving):
    '''
    '''
    catToWidth = {}
    geo_scale = 0.000008
    #with open(typeToWidthFname,'rb') as ifile:
    with open(typeToWidthFname, 'r') as ifile:
        reader = csv.DictReader(ifile)
        for row in reader:
            catToWidth[int(row['CategoryID'])] = row

    #---Contructing lanes attributes
    lanes = {}
    #for segid,segment in segments.iteritems():
    for segid, segment in segments.items():
        #for i in xrange(segment.numlanes):
        for i in range(segment.numlanes):
            laneid = int(segid)*100 + i
            width = catToWidth[segment.category]['LaneWidth']
            lane = Lane(id=laneid,segid=segid,width=width)
            lanes[laneid] = lane

    #---Contructing lane polylines
    #for segid,segment in segments.iteritems():
    for segid, segment in segments.items():
        segPos = segment.position
        segPos = [(float(item['x']),float(item['y'])) for item in segPos]
        width = float(catToWidth[segment.category]['LaneWidth'])*geo_scale
        #---looping over the lanes from the slowest to fastest
        #for i in xrange(segment.numlanes):
        for i in range(segment.numlanes):
            if lefthand_driving == True:
                laneid = int(segid) * 100 + segment.numlanes - 1 - i
            else:
                laneid = int(segid)*100 + i

            if segment.numlanes==1:
                lanePos = segPos
            elif segment.numlanes==2:
                lanePos = offsetPolyLine(segPos,(-0.5+i)*width)
            elif segment.numlanes==3:
                lanePos = offsetPolyLine(segPos,(-1+i)*width)
            elif segment.numlanes==4:
                lanePos = offsetPolyLine(segPos,(-1.5+i)*width)
            elif segment.numlanes==5:
                lanePos = offsetPolyLine(segPos,(-2.+i)*width)
            elif segment.numlanes==6:
                lanePos = offsetPolyLine(segPos,(-2.5+i)*width)
            elif segment.numlanes==7:
                lanePos = offsetPolyLine(segPos,(-3.+i)*width)
            elif segment.numlanes==8:
                lanePos = offsetPolyLine(segPos,(-3.5+i)*width)
            elif segment.numlanes==9:
                lanePos = offsetPolyLine(segPos,(-4.0+i)*width)
            elif segment.numlanes==10:
                lanePos = offsetPolyLine(segPos,(-4.5+i)*width)
            elif segment.numlanes==11:
                lanePos = offsetPolyLine(segPos,(-5.0+i)*width)
            elif segment.numlanes==12:
                lanePos = offsetPolyLine(segPos,(-5.5+i)*width)
            else:
                print ("+++ @constructLanes(): The following case is not defined: numLanes: ", segment.numlanes)
                pdb.set_trace()

            lanes[laneid].position = lanePos
    return lanes


def offsetPolyLine(polyLine,offset):
    '''
    '''

    if len(polyLine)==1:
        print ("+++ @offsetPolyLine(): polyline has only 1 point")
        pdb.set_trace()

    # if abs(offset)<10**-6:
    #     return polyLine
    lineList = []
    # off-set all the lines (pairs of points)
    for point1,point2 in zip(polyLine,polyLine[1:]):
        dx = point2[0]-point1[0]
        dy = point2[1]-point1[1]
        try:
            n = np.array([-dy,dx])/norm([-dy,dx])
        except:
            pdb.set_trace()
        if not type(norm([-dy,dx])) is np.float64:
            pdb.set_trace()

        offpoint1 = np.array(point1) + offset*n
        offpoint2 = np.array(point2) + offset*n
        lineList.append([offpoint1,offpoint2])

    #print lineList
    offsetPolyLine = []
    #---determing the middle points of two offset line
    if len(polyLine)>2:
        for line1,line2 in zip(lineList,lineList[1:]):
            if line1[1][0]-line1[0][0] != 0 and line2[1][0]-line2[0][0] != 0:
                m1 = (line1[1][1]-line1[0][1])/(line1[1][0]-line1[0][0])
                c1 = line1[1][1] - m1*line1[1][0]
                m2 = (line2[1][1]-line2[0][1])/(line2[1][0]-line2[0][0])
                c2 = line2[1][1] - m2*line2[1][0]
                m0 = (m1 - m2) if (m1 - m2) != 0 else 0.001
                interx = (c2-c1)/m0
                intery = m1*interx + c1
                offsetPolyLine.append(np.array([interx,intery]))

    #---inserting the first and last points
    offsetPolyLine.insert(0,lineList[0][0])
    offsetPolyLine.append(lineList[-1][1])

    #---Should be updated to handle exceptions where inf is at the end points
    #for i in xrange(len(offsetPolyLine)):
    for i in range(len(offsetPolyLine)):
        point = offsetPolyLine[i]
        if point[0]==np.inf or point[1]==np.inf:
            p1,p2 = offsetPolyLine[i-1],offsetPolyLine[i+1]
            newp = (p1+p2)/2.
            offsetPolyLine[i] = newp

    return offsetPolyLine


def clean_intersections(G, tolerance=15, dead_ends=False):
    """
    Clean-up intersections comprising clusters of nodes by merging them and
    returning their centroids.

    Divided roads are represented by separate centerline edges. The intersection
    of two divided roads thus creates 4 nodes, representing where each edge
    intersects a perpendicular edge. These 4 nodes represent a single
    intersection in the real world. This function cleans them up by buffering
    their points to an arbitrary distance, merging overlapping buffers, and
    taking their centroid. For best results, the tolerance argument should be
    adjusted to approximately match street design standards in the specific
    street network.

    Parameters
    ----------
    G : networkx multidigraph
    tolerance : float
        nodes within this distance (in graph's geometry's units) will be
        dissolved into a single intersection
    dead_ends : bool
        if False, discard dead-end nodes to return only street-intersection
        points

    Returns
    ----------
    G : modified and cleaned networkx multidigraph
    """

    if not dead_ends:
        if 'streets_per_node' in G.graph:
            streets_per_node = G.graph['streets_per_node']
        else:
            streets_per_node = ox.count_streets_per_node(G)

        dead_end_nodes = [node for node, count in streets_per_node.items() if count <= 1]
        # G = G.copy()
        G.remove_nodes_from(dead_end_nodes)
    return G
