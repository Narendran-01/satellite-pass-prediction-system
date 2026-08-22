import pandas as pd
import matplotlib.pyplot as plt
import numpy as np


# ============================================================
# 1. Load simulation data
# ============================================================

data = pd.read_csv("orbit.csv")

# Ground station: Chennai, India
station_lat = 13.0827
station_lon = 80.2707


# ============================================================
# 2. Identify visible passes
# ============================================================

visible = data[data["visible"] == 1].copy()

# Separate individual passes.
# A new pass begins whenever there is a gap greater than 1 second.
visible["pass_group"] = (
    visible["time_s"].diff().fillna(1).ne(1).cumsum()
)

passes = [
    group for _, group in visible.groupby("pass_group")
]

# Select the pass with the highest maximum elevation.
strongest_pass = max(
    passes,
    key=lambda group: group["elevation_deg"].max()
)

max_row = strongest_pass.loc[
    strongest_pass["elevation_deg"].idxmax()
]


# ============================================================
# 3. Load world map
# ============================================================

img = plt.imread("worldmap.jpg")


# ============================================================
# 4. Create plot
# ============================================================

plt.figure(figsize=(14, 7))

plt.imshow(
    img,
    extent=[-180, 180, -90, 90]
)


# ============================================================
# 5. Plot complete 24-hour ground track
# ============================================================

longitude = data["longitude_deg"].to_numpy()
latitude = data["latitude_deg"].to_numpy()

# Prevent Matplotlib from drawing an incorrect line
# across the map when longitude wraps from +180° to -180°.
wrap_points = np.where(
    np.abs(np.diff(longitude)) > 180
)[0] + 1

start = 0

for end in np.append(wrap_points, len(data)):

    plt.plot(
        longitude[start:end],
        latitude[start:end],
        linewidth=0.8,
        alpha=0.45
    )

    start = end


# ============================================================
# 6. Highlight strongest detected pass
# ============================================================

plt.plot(
    strongest_pass["longitude_deg"],
    strongest_pass["latitude_deg"],
    linewidth=3,
    label="Detected pass"
)


# ============================================================
# 7. Mark maximum elevation point
# ============================================================

plt.scatter(
    max_row["longitude_deg"],
    max_row["latitude_deg"],
    s=80,
    marker="o",
    zorder=5,
    label=f"Maximum elevation: {max_row['elevation_deg']:.2f}°"
)


# ============================================================
# 8. Mark ground station
# ============================================================

plt.scatter(
    station_lon,
    station_lat,
    s=100,
    marker="^",
    zorder=6,
    label="Ground station"
)

plt.annotate(
    "Chennai Ground Station",
    (station_lon, station_lat),
    xytext=(10, 10),
    textcoords="offset points"
)


# ============================================================
# 9. Labels and formatting
# ============================================================

plt.xlabel("Longitude (degrees)")
plt.ylabel("Latitude (degrees)")

plt.title(
    "Satellite Ground Track — 24-Hour SGP4 Simulation",
    fontsize=15
)

plt.xlim(-180, 180)
plt.ylim(-90, 90)

plt.grid(
    True,
    alpha=0.25
)

plt.legend(
    loc="lower left"
)

plt.tight_layout()


# ============================================================
# 10. Save result
# ============================================================

plt.savefig(
    "ground_track.png",
    dpi=200,
    bbox_inches="tight"
)

plt.show()


# ============================================================
# 11. Print detected pass information
# ============================================================

print("\nStrongest detected pass")
print("------------------------")

print(
    f"AOS              : {strongest_pass['time_s'].iloc[0]:.0f} s"
)

print(
    f"Maximum Elevation : {max_row['elevation_deg']:.2f} deg"
)

print(
    f"Time of Max       : {max_row['time_s']:.0f} s"
)

last_visible_time = strongest_pass["time_s"].iloc[-1]

los_candidates = data[
    (data["time_s"] > last_visible_time) &
    (data["visible"] != 1)
]

if not los_candidates.empty:
    los_time = los_candidates["time_s"].iloc[0]
else:
    los_time = last_visible_time

print(
    f"LOS              : {los_time:.0f} s"
)    

print(
    f"Pass Duration     : "
    f"{los_time - strongest_pass['time_s'].iloc[0]:.0f} s"
)

print("\nGround track saved as ground_track.png")