#!/usr/bin/env python3
import argparse
import csv
import os
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection


def read_csv(path, columns):
    with open(path, newline="", encoding="utf-8-sig") as file:
        reader = csv.DictReader(file)
        if not reader.fieldnames or not set(columns).issubset(reader.fieldnames):
            raise ValueError(f"{path} must have columns: {', '.join(columns)}")
        return list(reader)


def main():
    parser = argparse.ArgumentParser(description="Render routes and benchmarks after running the router.")
    parser.add_argument("nodes", help="nodes CSV (id, lat, lon)")
    parser.add_argument("edges", help="edges CSV (from, to)")
    parser.add_argument("--routes", default="output/routes.csv")
    parser.add_argument("--benchmarks", default="output/benchmarks.csv")
    parser.add_argument("--output", default="output", help="directory for PNG files")
    parser.add_argument("--severity", type=float, default=1.0,
                        help="flood severity used for the run (0-1); scales road hazard")
    parser.add_argument("--affected", type=float, default=0.5,
                        help="hazard (risk x severity) at or above which a road counts as affected")
    args = parser.parse_args()

    try:
        nodes = {row["id"]: (float(row["lon"]), float(row["lat"]))
                 for row in read_csv(args.nodes, ("id", "lat", "lon"))}
        roads, affected, hazards = [], [], []
        for row in read_csv(args.edges, ("from", "to", "risk")):
            segment = (nodes[row["from"]], nodes[row["to"]])
            hazard = float(row["risk"]) * args.severity
            if hazard >= args.affected:
                affected.append(segment)
                hazards.append(hazard)
            else:
                roads.append(segment)

        benchmarks = read_csv(args.benchmarks, ("algorithm", "found", "time_ms", "nodes_explored", "cost"))
        routes = {}
        for row in read_csv(args.routes, ("algorithm", "sequence", "lat", "lon")):
            routes.setdefault(row["algorithm"], []).append(
                (int(row["sequence"]), float(row["lon"]), float(row["lat"])))
        os.makedirs(args.output, exist_ok=True)

        fig, ax = plt.subplots(figsize=(10, 8))
        ax.add_collection(LineCollection(roads, colors="#b6bec8", linewidths=0.35, label="Road network"))
        if affected:
            flooded = LineCollection(affected, cmap="Purples", linewidths=0.9, label="Affected roads")
            flooded.set_array(hazards)
            flooded.set_clim(args.affected - 0.3, 1.0)  # keep the lightest affected road visible
            ax.add_collection(flooded)
            fig.colorbar(flooded, ax=ax, shrink=0.6, label="Flood hazard (risk x severity)")
        colors = {"Dijkstra": "#cf513d", "A*": "#2864b7"}
        for name, points in routes.items():
            points.sort()
            longitudes = [point[1] for point in points]
            latitudes = [point[2] for point in points]
            ax.plot(longitudes, latitudes,
                    color=colors.get(name, "#287a57"),
                    linewidth=3 if name == "Dijkstra" else 1.8,
                    linestyle="--" if name == "A*" else "-",
                    marker="o" if len(points) == 1 else None, label=name)
        if routes:
            first_route = next(iter(routes.values()))
            ax.scatter(first_route[0][1], first_route[0][2], marker="o", s=45,
                       color="#13804a", edgecolors="white", zorder=5, label="Start")
            ax.scatter(first_route[-1][1], first_route[-1][2], marker="*", s=120,
                       color="#202d45", edgecolors="white", zorder=5, label="Destination")
        else:
            ax.text(0.5, 0.05, "No route found", transform=ax.transAxes,
                    ha="center", bbox={"facecolor": "white", "alpha": 0.9})
        ax.autoscale()
        ax.set_aspect("equal", adjustable="datalim")
        ax.set(xlabel="Longitude", ylabel="Latitude",
               title=f"Evacuation routes (severity {args.severity:g}, {len(affected)} affected roads)")
        ax.legend()
        fig.tight_layout()
        fig.savefig(os.path.join(args.output, "routes.png"), dpi=180)
        plt.close(fig)

        fig, axes = plt.subplots(1, 3, figsize=(12, 4))
        names = [row["algorithm"] for row in benchmarks]
        metrics = [("time_ms", "Time (ms)"),
                   ("nodes_explored", "Nodes explored"),
                   ("cost", "Route cost")]
        for ax, (key, title) in zip(axes, metrics):
            values = [float(row[key] or 0) for row in benchmarks]
            bars = ax.bar(names, values, color=[colors.get(name, "#287a57") for name in names])
            ax.set_title(title)
            if key == "cost":
                for bar, row in zip(bars, benchmarks):
                    if row["found"] != "1":
                        ax.annotate("No route", (bar.get_x() + bar.get_width() / 2, 0),
                                    ha="center", va="bottom")
        fig.tight_layout()
        fig.savefig(os.path.join(args.output, "benchmarks.png"), dpi=180)
        plt.close(fig)
    except (OSError, ValueError, KeyError) as error:
        print(f"Could not create visualizations: {error}", file=sys.stderr)
        return 1
    print(f"Saved routes.png and benchmarks.png in {args.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
