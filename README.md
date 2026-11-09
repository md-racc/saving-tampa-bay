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

## Requirements

- g++ with C++17, and `make`
- Python 3.10+ with `osmnx` and `matplotlib`

```sh
python -m venv .venv && source .venv/bin/activate
pip install osmnx matplotlib
```

## 1. Fetch the data

```sh
python scripts/fetch_graph.py
```

The default pulls all drivable roads within 16 km of downtown Tampa, about 100k data points
(vertices + edges). The first run takes up to ~10 minutes (OSM download + elevation API).
Results are cached in `data/raw/cache/`, so later runs are fast.

For a quick test on a smaller graph, use `--dist 5000` (meters). Add `--no-elevation` to skip
the elevation API; risk is then based on distance to water only.

Outputs: `data/processed/nodes.csv`, `edges.csv`, `places.csv`, and `data/raw/tampa_drive.graphml`.

If the download fails, the OSM server (Overpass) is overloaded. Wait a few minutes and re-run.

## 2. Build

```sh
make            # builds bin/router
make clean      # removes bin/
```

## 3. Run

List the available start points and evacuation targets:

```sh
./bin/router data/processed/nodes.csv data/processed/edges.csv --list
```

Route from a start to a target at a given flood severity (0 = no flood, 1 = worst):

```sh
./bin/router data/processed/nodes.csv data/processed/edges.csv <start> <end> <severity>
```

`<start>` and `<end>` can be a number from `--list`, a place name in quotes, or a raw OSM node ID:

```sh
./bin/router data/processed/nodes.csv data/processed/edges.csv 2 9 0.5
./bin/router data/processed/nodes.csv data/processed/edges.csv "Ybor City" "I-4 East at 40th St" 0.8
```

The program runs both algorithms on the same flooded graph and prints the route cost,
execution time, and nodes explored for each. It also writes:

- `output/benchmarks.csv`: algorithm, found, time_ms, nodes_explored, cost
- `output/routes.csv`: each route as an ordered list of node IDs with lat/lon

## 4. Visualize

```sh
python scripts/visualize.py data/processed/nodes.csv data/processed/edges.csv --severity 0.5
```

Pass the same `--severity` you used for the router, so the flooded-road shading matches the run.
`--affected` (default 0.5) sets the hazard level (risk × severity) above which a road is drawn
as affected.

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
