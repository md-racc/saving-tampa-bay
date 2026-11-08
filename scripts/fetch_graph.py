#!/usr/bin/env python3
# adi - get tampa roads and export csv

import argparse
import csv
import math
import sys
from pathlib import Path

import osmnx as ox

# settings
ROOT = Path(__file__).resolve().parent.parent
RAW_DIR = ROOT / "data" / "raw"
OUT_DIR = ROOT / "data" / "processed"
CENTER = (27.9506, -82.4572) # tampa

DEFAULT_DIST_M = 10000
WATER_RANGE_M = 1000.0 
SAFE_ELEVATION_M = 10.0 
WATER_WEIGHT = 0.5     
ELEVATION_WEIGHT = 0.5  

ELEVATION_URL = "https://api.opentopodata.org/v1/ned10m?locations={locations}"
WATER_TAGS = {
    "natural": ["water", "coastline", "bay"],
    "waterway": ["river", "canal"],
}
MIN_WATER_AREA_M2 = 100_000

PLACES = [
    # high foottraffic
    ("Downtown - Curtis Hixon Park", "start", 27.9497, -82.4626),
    ("Ybor City", "start", 27.9600, -82.4370),
    ("Channelside - Amalie Arena", "start", 27.9427, -82.4518),
    ("Harbour Island", "start", 27.9395, -82.4560),
    ("Davis Islands", "start", 27.9230, -82.4545),
    ("Hyde Park Village", "start", 27.9378, -82.4790),
    ("University of Tampa", "start", 27.9466, -82.4652),
    ("Bayshore Boulevard", "start", 27.9240, -82.4820),
    #exfil
    ("I-275 North at Hillsborough Ave", "evac", 27.9958, -82.4650),
    ("I-4 East at 40th St", "evac", 27.9690, -82.4100),
    ("I-275 at Dale Mabry Hwy", "evac", 27.9615, -82.5055),
]

def pull_graph(dist_m):

    print(f"[1/7] Downloading road network within {dist_m} m of downtown Tampa...")
    G = ox.graph_from_point(CENTER, dist=dist_m, network_type="drive")
    print(f"      downloaded {len(G):, } nodes, {G.number_of_edges():,} edges")

    print("[2/7] Keeping largest strong connected component...")
    G = ox.truncate.largest_component(G, strongly=True)
    print(f"      kept {len(G):,} nodes, {G.number_of_edges():,} edges")
    return G


def populate_elevation(G):
    n_calls = math.ceil(len(G)/100)
    print(f"[3/7] Looking up elevations ({n_calls} requests, ~1 per second; cached after first run)...")

    ox.settings.elevation_url_template = ELEVATION_URL 
    ox.elevation.add_node_elevations_google(G, batch_size=100, pause=1.0)

    for _, data in G.nodes(data=True):
        if data.get("elevation") is None:
            data["elevation"] = 0.0

def populate_water_distance(G, dist_m):
    print("[4/7] Downloading water features and measuring water distance...")
    water = ox.features_from_point(CENTER, tags=WATER_TAGS, dist=dist_m + 2000)
    water = water[~water.geometry.geom_type.isin(["Point", "MultiPoint"])]
    water = ox.projection.project_gdf(water[["geometry"]])

    is_polygon = water.geometry.geom_type.isin(["Polygon", "MultiPolygon"])
    water = water[~is_polygon | (water.geometry.area >= MIN_WATER_AREA_M2)]
    print(f"      {len(water):,} water features (after dropping small ponds)")

    nodes = ox.convert.graph_to_gdfs(G, edges=False)[["geometry"]]
    nodes = ox.projection.project_gdf(nodes, to_crs=water.crs)
    
    joined = nodes.sjoin_nearest(water, distance_col="water_m")
    water_m = joined.groupby(level=0)["water_m"].min()

    for node_id, dist in water_m.items():
        G.nodes[node_id]["water_m"] = float(dist)

# flood risk

def clamp01(x):
    return max(0.0, min(1.0, x))

def compute_risk(G, use_elevation):

    print("[5/7] Computing simulated flood risk ...")
    for _, data in G.nodes(data=True):
        water_score = clamp01(1.0 - data["water_m"] / WATER_RANGE_M)
        if use_elevation:
            elev_score = clamp01(1.0 - data["elevation"] / SAFE_ELEVATION_M)
            data["risk"] = WATER_WEIGHT * water_score + ELEVATION_WEIGHT * elev_score
        else:
            data["risk"] = water_score
    for u, v, _, data in G.edges(keys=True, data=True):
        data["risk"] = max(G.nodes[u]["risk"], G.nodes[v]["risk"])
    risks = [d["risk"] for _, _, d in G.edges(data=True)]
    print(f"      edge risk: min {min(risks):.2f}, mean {sum(risks) / len(risks):.2f}, "
          f"max {max(risks):.2f}")


#TODO haversine + place snapper

def haversin(lat1,lon1,lat2,lon2):
    r = 6371000.0
    p1,p2 = math.radians(lat1),math.radians(lat2)
    dp = p2 - p1
    dl = math.radians(lon2-lon1)
    a = math.sin(dp / 2)** 2 + math.cos(p1)* math.cos(p2)*math.sin(dl/2)**2
    return 2*r *math.asin(math.sqrt(a))

def snap_places(G):
    print("[6/7] Snapping PoI to intersections...")
    snapped = []
    for p in PLACES:
        distances = []
        node_ids = []
        for node_id in G.nodes:
            d = haversin(p[2], p[3], G.nodes[node_id]["y"], G.nodes[node_id]["x"])
            distances.append(d)
            node_ids.append(node_id)

        best_d = min(distances)
        best_id = node_ids[distances.index(best_d)]

        if best_d > 300:
            print("     " + p[1] + "  " + p[0] + "  node " + str(best_id) + " (" + str(int(best_d)) + " m away) WARNING: far from road")
        else:
            print("     " + p[1] + "  " + p[0] + "  node " + str(best_id) + " (" + str(int(best_d)) + " m away)")
        snapped.append((p[0], p[1], best_id))
    return snapped
# file writing

def write_outputs(G, places):
    print("[7/7] Writing CSVs ...")
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    f = open(str(OUT_DIR) + "/nodes.csv", "w")
    f.write("id,lat,lon\n")
    for node in G.nodes:
        lat = round(G.nodes[node]["y"], 7)
        lon = round(G.nodes[node]["x"], 7)
        f.write(str(node) + "," + str(lat) + "," + str(lon) + "\n")
    f.close()

    f = open(str(OUT_DIR) + "/edges.csv", "w")
    f.write("from,to,length,risk\n")
    edge_count = 0
    for u, v, data in G.edges(data=True):
        f.write(str(u) + "," + str(v) + "," + str(round(data["length"], 2)) + "," + str(round(data["risk"], 4)) + "\n")
        edge_count += 1
    f.close()

    f = open(str(OUT_DIR) + "/places.csv", "w")
    f.write("name,kind,id\n")
    for p in places:
        if "," in p[0]:
            raise Exception("place name has a comma: " + p[0])
        f.write(p[0] + "," + p[1] + "," + str(p[2]) + "\n")
    f.close()

    graphml = str(RAW_DIR) + "/tampa_drive.graphml"
    ox.save_graphml(G, graphml)

    print("      " + str(OUT_DIR) + "/nodes.csv  (" + str(len(G.nodes)) + " rows)")
    print("      " + str(OUT_DIR) + "/edges.csv  (" + str(edge_count) + " rows)")
    print("      " + str(OUT_DIR) + "/places.csv  (" + str(len(places)) + " rows)")
    print("      " + graphml)



def main():
    parser = argparse.ArgumentParser(description="Download Tampa roads and export CSVs.")
    parser.add_argument("--dist", type=int, default=DEFAULT_DIST_M,
                        help=f"half-width of the download box in meters (default {DEFAULT_DIST_M})")
    parser.add_argument("--no-elevation", action="store_true",
                        help="skip the elevation API; risk uses distance to water only")
    args = parser.parse_args()

    RAW_DIR.mkdir(parents=True, exist_ok=True)
    ox.settings.cache_folder = RAW_DIR / "cache"
    ox.settings.requests_timeout = 300 

    G = pull_graph(args.dist)
    if not args.no_elevation:
        populate_elevation(G)
    populate_water_distance(G, args.dist)
    compute_risk(G, use_elevation=not args.no_elevation)
    places = snap_places(G)
    write_outputs(G, places)

    if G.number_of_edges() > 100_000:
        print(f"note: {G.number_of_edges():,} edges is over the proposal's ~100k; "
              "try a smaller --dist", file=sys.stderr)
    return 0

if __name__ == "__main__":

    main()

