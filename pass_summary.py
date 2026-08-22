import pandas as pd
import matplotlib.pyplot as plt


# ============================================================
# 1. Load simulation data
# ============================================================

data = pd.read_csv("orbit.csv")


# ============================================================
# 2. Find visible passes
# ============================================================

visible = data[data["visible"] == 1].copy()

visible["pass_group"] = (
    visible["time_s"].diff().fillna(1).ne(1).cumsum()
)

passes = [
    group for _, group in visible.groupby("pass_group")
]


# ============================================================
# 3. Select strongest pass
# ============================================================

strongest_pass = max(
    passes,
    key=lambda group: group["elevation_deg"].max()
)

max_row = strongest_pass.loc[
    strongest_pass["elevation_deg"].idxmax()
]


# ============================================================
# 4. Determine AOS and LOS
# ============================================================

aos_time = strongest_pass["time_s"].iloc[0]

last_visible_time = strongest_pass["time_s"].iloc[-1]

los_candidates = data[
    (data["time_s"] > last_visible_time) &
    (data["visible"] != 1)
]

if not los_candidates.empty:
    los_time = los_candidates["time_s"].iloc[0]
else:
    los_time = last_visible_time


# ============================================================
# 5. Extract maximum-elevation measurements
# ============================================================

max_time = max_row["time_s"]
max_elevation = max_row["elevation_deg"]
max_azimuth = max_row["azimuth_deg"]
max_range = max_row["range_km"]
max_range_rate = max_row["range_rate_mps"]
max_doppler = max_row["doppler_shift_Hz"]

pass_duration = los_time - aos_time


# ============================================================
# 6. Create result card
# ============================================================

fig, ax = plt.subplots(figsize=(10, 6))

ax.axis("off")


# ============================================================
# 7. Title
# ============================================================

ax.text(
    0.5,
    0.88,
    "SATELLITE PASS PREDICTION",
    ha="center",
    va="center",
    fontsize=22,
    fontweight="bold"
)

ax.text(
    0.5,
    0.81,
    "SGP4 simulation — Chennai Ground Station",
    ha="center",
    va="center",
    fontsize=12
)


# ============================================================
# 8. Pass timing
# ============================================================

ax.text(
    0.15,
    0.67,
    "PASS TIMING",
    fontsize=12,
    fontweight="bold"
)

ax.text(
    0.15,
    0.59,
    f"AOS\n{aos_time:.0f} s",
    fontsize=14,
    va="top"
)

ax.text(
    0.42,
    0.59,
    f"MAX ELEVATION\n{max_elevation:.2f}°",
    fontsize=14,
    va="top"
)

ax.text(
    0.69,
    0.59,
    f"LOS\n{los_time:.0f} s",
    fontsize=14,
    va="top"
)


# ============================================================
# 9. Pass duration
# ============================================================

ax.text(
    0.5,
    0.40,
    f"PASS DURATION   {pass_duration:.0f} s",
    ha="center",
    fontsize=18,
    fontweight="bold"
)


# ============================================================
# 10. Maximum-elevation measurements
# ============================================================

ax.text(
    0.15,
    0.28,
    "AT MAXIMUM ELEVATION",
    fontsize=12,
    fontweight="bold"
)

ax.text(
    0.15,
    0.21,
    f"Time              {max_time:.0f} s",
    fontsize=12
)

ax.text(
    0.15,
    0.15,
    f"Azimuth           {max_azimuth:.2f}°",
    fontsize=12
)

ax.text(
    0.55,
    0.21,
    f"Range             {max_range:.2f} km",
    fontsize=12
)

ax.text(
    0.55,
    0.15,
    f"Range Rate        {max_range_rate:.2f} m/s",
    fontsize=12
)

ax.text(
    0.55,
    0.09,
    f"Doppler Shift     {max_doppler:.2f} Hz",
    fontsize=12
)


# ============================================================
# 11. Save result
# ============================================================

plt.savefig(
    "pass_summary.png",
    dpi=200,
    bbox_inches="tight"
)

plt.show()


# ============================================================
# 12. Print result
# ============================================================

print("\nPass Summary")
print("------------")
print(f"AOS              : {aos_time:.0f} s")
print(f"LOS              : {los_time:.0f} s")
print(f"Pass Duration     : {pass_duration:.0f} s")
print(f"Maximum Elevation : {max_elevation:.2f} deg")
print(f"Time of Max       : {max_time:.0f} s")
print(f"Azimuth           : {max_azimuth:.2f} deg")
print(f"Range             : {max_range:.2f} km")
print(f"Range Rate        : {max_range_rate:.2f} m/s")
print(f"Doppler Shift     : {max_doppler:.2f} Hz")

print("\nSaved as pass_summary.png")