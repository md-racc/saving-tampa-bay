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
