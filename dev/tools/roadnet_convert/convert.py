#!/usr/local/bin/python3

'''This program converts a SUMO traffic network to Sim Mobility format (this includes 
   flipping the network for driving on the left).
   Note: Run SUMO like so:
     ~/sumo/bin/netgenerate --rand -o random.sumo.xml --rand.iterations=200 --random -L 2
'''

import sys
import math

try:
  from lxml import objectify
  from lxml import etree
except ImportError:
  raise Exception("lxml module does not exist\n(Hint: try \"sudo apt-get install python-lxml\" on Ubuntu.)")

#TODO: I realize now that naming every file the same "e.g., "something.sumo, somethingelse.sumo" was not
#      the best idea; need to clean up the package structure at some point. ~Seth
import geo.helper
import geo.convert.sumo2simmob
import geo.convert.osm2simmob
import geo.parsers.sumo
import geo.parsers.osm
import geo.formats.simmob
import geo.formats.sumo
import geo.formats.osm
import geo.serializers.simmob
import geo.serializers.anm
import geo.serializers.vissim
import geo.serializers.osm
import geo.serializers.out_txt
from geo.position import Point
from geo.position import Location
from geo.helper import DynVect
from geo.helper import ScaleHelper

#This line will fail to parse if you are using Python2 (that way at least we fail early).
def __chk_versn(x:"Error: Python 3 is required; this program will not work with Python 2"): pass


#TODO: We might want to add a *small* buffer to anything with a "bounds" (e.g., OSM output), as our 
#      lane points might *barely* overflow this.


def run_main(inFileName, outFileName):
  #Resultant datastructure  
  rn = None  #simmob.RoadNetwork

  #Try to parse and convert
  if inFileName.endswith('.sumo.xml'):
    snet = geo.parsers.sumo.parse(inFileName)
    rn = geo.convert.sumo2simmob.convert(snet)
  elif inFileName.endswith('.osm'):
    onet = geo.parsers.osm.parse(inFileName)
    rn = geo.convert.osm2simmob.convert(onet)

  if not rn:
    raise Exception('Unknown road network format: ' + inFileName)

  #Remove nodes which aren't referenced by anything else.
  #TODO: This remains *here*, and might be disabled with a switch.
  nodesPruned = len(rn.nodes)
  geo.helper.remove_unused_nodes(rn.nodes, rn.links)
  nodesPruned -= len(rn.nodes)
  print("Pruned unreferenced nodes: %d" % nodesPruned)

  #Translate our network so that all (x,y) points are positive.
  globMin = geo.helper.get_global_min(rn)
  geo.helper.translate_network(rn, globMin)

  #Before printing the XML network, we should print an "out.txt" file for 
  #  easier visual verification with our old GUI.
  geo.serializers.out_txt.serialize(rn, outFileName+".out.txt")

  #Also print in OSM format, for round-trip checking.
  geo.serializers.osm.serialize(rn, outFileName+".osm", [Location(1.264,103.646), Location(1.438,103.992)])   #We'll cheat a bit for now and center it on Singapore.

  #And VISSIM
  geo.serializers.vissim.serialize(rn, outFileName+".inp")
  geo.serializers.anm.serialize(rn, outFileName+".anm")

  #Now print the network in XML format, for use with the actual software.
  geo.serializers.simmob.serialize(rn, outFileName+".simmob.xml")


if __name__ == "__main__":
  if len(sys.argv) < 3:
    print ('Usage:\n' , sys.argv[0] , '<in_file.net.xml>,  <out_file.X>')
    sys.exit(0)

  run_main(sys.argv[1], sys.argv[2])
  print("Done")



