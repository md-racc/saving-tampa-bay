# Saving Tampa Bay

Flood-aware evacuation routing over the Tampa Bay road network.

**COP3530 Data Structures & Algorithms — Project 2**
Team _Goobers_ (Group 59) — Aditya Nair, Suhas Gaddam, Vishnu Araveeti

## Overview

During hurricanes and severe flooding, road closures and hazardous conditions make traditional
shortest-path navigation unreliable for planning. In coastal cities, the shortest route to a highway
or exfil location can very easily become submerged based on elevation. This project aims to
develop a flood-aware evacuation routing system using OSM road network data to identify
optimal routes over a flooded tampa bay road graph network. By simulating flood conditions and
closures, we can evaluation how hazards affect path finding with Dijkstra and A\* pathfinding
algorithms.

The project will work on public road network data from OpenStreetMap (OSM) extracted via the
python OSMnx library covering Tampa Bay with a focus on high foot traffic down town areas,
sized to fit 100,000 street segments. We’ll represent it as a directed graph, with intersections as
nodes containing Id’s, latitude, longitude, and weighted by length. Additional simulated
flood-risk values will be assigned to each segment based on distance to water and elevation, with
each value effecting the algorithm’s weights.

## How to build/run

Download the data by running :  python scripts/fetch_graph.py --dist 5000
afterwards, 

Build:

make

Afterwards, go to data/processed/places.csv and select the IDs for your starting and evacuation points. Then, with the command below, replace the first id with your starting selection and the second with your evacuation. The final number is the severity of the flood.

./bin/router data/processed/nodes.csv data/processed/edges.csv 1868658525 1020640107 0.5

If you want more data points, you can change the 5000 to something like 16000, but this can take 10 minutes or longer to download.

## How to create visuals

Use the command in your terminal: python scripts/visualize.py data/processed/nodes.csv data/processed/edges.csv

In your terminal and they will generate images in the output folder.



## Algorithms

We plan to implement two shortest-path algorithms **from scratch in C++** and
compare them:

| Algorithm    | How it works                                                |
| ------------ | ----------------------------------------------------------- |
| **Dijkstra** | Greedy shortest path by edge cost (distance + flood risk).  |
| **A\***      | Dijkstra guided by a straight-line heuristic to the target. |

The CLI will report execution time, nodes explored, and total route cost for each,
so their relative efficiency can be evaluated directly.

## Data

Road network data will be pulled from OpenStreetMap via `osmnx` (Python), overlayed
with a simulated flood-risk value per segment based on distance to water and
elevation, and exported to CSV for the C++ program.
