from shapely.geometry import LineString, Point
from shapely.ops import transform
from functools import partial
import pyproj
import pandas as pd
import geopandas as gpd
import pyproj


LAT_LONG_PROJECTION = pyproj.Proj(init='EPSG:4326')
BALTIMORE_PROJECTION = pyproj.Proj(init='EPSG:6487')
UTM48N_PROJECTION = pyproj.Proj(init='EPSG:32648')

LAT_LONG_CRS = {'init': 'epsg:4326'}
BALTIMORE_CRS = {'init': 'epsg:6487'}
UTM48N_CRS = {'init': 'epsg:32648'}

project = partial(
    pyproj.transform,
    LAT_LONG_PROJECTION,
    UTM48N_PROJECTION)

'''
# Function using pyproj to convert state plane to lat/lon
def convert_to_latlon(x, y):
    state_plane = pyproj.Proj(init='EPSG:3585', preserve_units=True)
    wgs = pyproj.Proj(proj='latlong', datum='WGS84', ellps='WGS84')
    lng, lat = pyproj.transform(state_plane, wgs, x, y)
    return lat, lng
def convert_from_latlon(lat, lon):
    state_plane = pyproj.Proj(init='EPSG:3585', preserve_units=True)
    wgs = pyproj.Proj(proj='latlong', datum='WGS84', ellps='WGS84')
    x, y = pyproj.transform(wgs, state_plane, lon, lat)
    return x, y
'''



inFOLDER = 'Outputs/Test/'
files_to_project = [
'node.csv',
'link_polyline.csv',
'link.csv',
'segment_polyline.csv',
'segment.csv',
'lane_polyline.csv',
'lane.csv',
'connector.csv',
'turning_path_polyline.csv',
'turning_path.csv',
'turning_group.csv',
'link_default_travel_time.csv']


def linksTravelTime():
    link_default_travel_time_cols = ['link_id', 'travel_mode', 'start_time', 'end_time', 'travel_time']
    link_travel_time_cols = ['link_id', 'downstream_link_id', 'start_time', 'end_time', 'travel_time']
    link_cols = ['id', 'road_type', 'category', 'from_node', 'to_node']
    link = pd.read_csv(outFOLDER + 'link-attributes-indexed.csv')
    link_default_travel_time = pd.read_csv(outFOLDER + 'linkttsdefault.csv')
    print('link_default_travel_time', link_default_travel_time.columns, len(link_default_travel_time))
    print('link', link.columns)
    link_default_travel_time.index = link_default_travel_time.id
    link.index = link.id
    travelTime_df =[]
    for linkID, row in link_default_travel_time.iterrows():
        link_to_node = link.loc[linkID, 'to_node']
        downstreams = link[link.from_node == link_to_node].id.tolist()
        travelTime_df += [(linkID, ds, row.starttime, row.endtime, row.traveltime) for ds in downstreams]
    travelTime_df = pd.DataFrame.from_records(travelTime_df, columns = link_travel_time_cols)
    travelTime_df.to_csv(outFOLDER + 'link_travel_time.csv', index=False)
#linksTravelTime()

def project_x_y(inFOLDER, fileName, toCRS=UTM48N_CRS):
    inSubFolder = inFOLDER + 'simmobility_wgs84/'
    outSubFolder = inFOLDER + 'simmobility_crs_projected/'
    df = pd.read_csv(inSubFolder + fileName)
    column_order = df.columns
    if not 'x' in column_order:
        df.to_csv(outSubFolder + fileName, index=False)
        return
    df['geometry'] = df.apply(lambda row: Point(row.x, row.y),axis=1)
    gdf = gpd.GeoDataFrame(df, crs=LAT_LONG_CRS, geometry=df['geometry'])
    gdf = gdf.to_crs(toCRS)
    gdf['x'] = gdf.apply(lambda row: row.geometry.x,axis=1)
    gdf['y'] = gdf.apply(lambda row: row.geometry.y,axis=1)
    df = gdf[column_order]
    df.to_csv(outSubFolder + fileName, index=False)

for f in files_to_project:
    project_x_y(inFOLDER, f)
