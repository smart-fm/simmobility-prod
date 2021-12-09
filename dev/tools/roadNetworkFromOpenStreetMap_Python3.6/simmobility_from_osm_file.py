################################################################################
# Module: simplify.py
# Description: Query OpenStreetMap and prepare SimMobility road network files.
# Written by: Iveel Tsogsuren, Arun Akkinepally
################################################################################
from collections import OrderedDict
import process_osm as posm
import osmnx as ox
import query_osm as qr
import networkx as nx
import os
from network import*
# from processing import*

# Get default SimMobility attribute values
inputFolder = "metadata/"
typeToWidthFname = os.path.join(inputFolder,"LinkCat_Roadtype_LaneWidth.csv")
ffsFname = os.path.join(inputFolder,"HCM2000.csv")


# Set this directory where you optionally pass boundary.shp file and get all outputs.
# boundary = os.getcwd() + '/Outputs/Tel_Aviv/Tel_Aviv_Metro_Area/buffer_wgs84.shp' # Can be either bounding box coordinates or
directory = '/Outputs/Test'
#boundary = [42.3645,42.3635,-71.1046,-71.108]
#boundary = [1.29805,1.29533,103.81190,103.81612] # Testing single link
#boundary = [1.29796,1.28763,103.81029,103.82711] # Testing more links
boundary = [1.3483,1.2622,103.7864,103.9211] # singapore cbd
#boundary = [1.4555,1.4338,103.7513,103.7850] # singapore - johor bahru border

# Prepare subfolders for outputs
outDir = os.getcwd() + directory
simmobility_crs_projected = outDir + "/simmobility_crs_projected"  # For later use.
simmobility_dir = outDir + "/simmobility_wgs84"
shapefile_dir = outDir + "/shapefiles"
sumo_dir = outDir + "/sumo"
graph_pickle_file = outDir + '/osm_graph.pkl'
input_osm_file = outDir + '/punggol.osm'
for d in [outDir, simmobility_dir, simmobility_crs_projected, shapefile_dir, sumo_dir]:
    if not os.path.exists(d):
        print(d)
        os.makedirs(d)

def query_OSM(directory, boundary, graph_pickle_file, query='drive_main'):
    """
    Query OpenStreetMap and save it as a pickle file.

    Parameters
    ----------
    directory : directory path where inputs and outputs should be
    boundary : Boundary for road network which can be either polygon boundary file
               or bounding box coordinates [north, south, west, east]
    network_type : string
        {'walk', 'bike', 'drive_all', 'drive_main', 'drive_main_links_included', 'drive_service', 'all', 'all_private', 'none'}
        what type of street or other network to get
    """
    mainG = ox.graph_from_file(input_osm_file, network_type=query, simplify=False, retain_all=True)
    # mainG = ox.graph_from_xml(input_osm_file, simplify=False, retain_all=True)
    nx.write_gpickle(mainG, graph_pickle_file)

def main(graph_pickle_file=graph_pickle_file, simmobility_dir=simmobility_dir):
    """
    Create all SimMobility road network tables and their shapes given a OSM graph file.

    Parameters
    ----------
    graph_pickle_file : file path of OSM graph in pickle format.
    simmobility_dir : output directory
    """
    lefthand_driving = True

    query_OSM(directory, boundary, graph_pickle_file)
    # all SimMobility road network module files.
    mainG = nx.read_gpickle(graph_pickle_file)
    roadnetwork = Network(mainG)
    roadnetwork.process_segments_links_nodes(clean_intersections=False)
    roadnetwork.lanes = posm.constructLanes(roadnetwork.segments, typeToWidthFname, lefthand_driving)
    roadnetwork.constructSegmentConnections()
    roadnetwork.construct_default_turning_path()

    # Files for creating smart turnning path connections using SUMO. TODO: use SUMO.
    #roadnetwork.writeSumoShapefile(sumo_dir)
    #roadnetwork.construct_turning_paths_from_SUMO(sumo_dir, lefthand_driving)

    roadnetwork.write_wgs84(foldername=simmobility_dir+inputFolder.split('/')[1])
    roadnetwork.writeShapeFiles(foldername=shapefile_dir+inputFolder.split('/')[1])



main()
