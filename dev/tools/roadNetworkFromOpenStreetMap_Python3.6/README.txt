This is an automated pipeline for generating SimMobility road network from
OpenStreetMap. The code base is modular and has modified libraries:
1. networkx=2.0 (other version might not compatible) https://networkx.org/
2. osmnx (modified as much as we need) https://github.com/gboeing/osmnx


### How to generate road network tables from OpenStreetMap

0. Define a boundary of the interest of area whose network you want to query. It can be either polygon shape file or bounding box. You can specify bounding box (4 coordinates) using the following tool
https://www.openstreetmap.org/export#map=10/35.2086/-80.8532
Search, export and manually select (optionally) a different area.

1. Query and generate SimMobility road network tables
simmobility_from_osm.py

2. Project road network modules to x, y coordinate system from latitude and longitude system.
project.py  (Projects all SimMobility road network files)

3. Upload all SimMobility files to the database:
uploadToDb.py


### Extra references:
/metadata

### OpenStreetMap
The key highway=* is the main key used for identifying any kind of road, street or path.
The value of the key helps indicate the importance of the highway within the road network as a whole.
See the table below for an ordered list from most important (motorway) to least important (service).

The tag highway=motorway is used only on ways, to identify the highest-performance roads within a territory.
Generally some restrictions are placed on the kind of vehicles or traffic which can be on roads which should be classed as highway=motorway, such as no pedestrians, bicycles, livestock, horses and so on.

### Speed limit:
https://wiki.openstreetmap.org/wiki/OSM_tags_for_routing/Maxspeed
