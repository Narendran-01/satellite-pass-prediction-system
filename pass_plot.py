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

# A new pass begins whenever there is a gap in the
# one-second visibility sequence.
visible["pass_group"] = (
    visible["time_s"].diff().fillna(1).ne(1).cumsum()
)

passes = [
    group for _, group in visible.groupby("pass_group")
]


# ============================================================
# 3. Select the strongest pass
# ============================================================

strongest_pass = max(
    passes,
    key=lambda group: group["elevation_deg"].max()
)

max_row = strongest_pass.loc[
    strongest_pass["elevation_deg"].idxmax()
]

# First visible sample
aos_time = strongest_pass["time_s"].iloc[0]

# Last visible sample
last_visible_time = strongest_pass["time_s"].iloc[-1]

# Find the first non-visible sample after the pass.
# This represents the LOS transition.
los_candidates = data[
    (data["time_s"] > last_visible_time) &
    (data["visible"] != 1)
]

if not los_candidates.empty:
    los_time = los_candidates["time_s"].iloc[0]
else:
    los_time = last_visible_time

max_time = max_row["time_s"]
max_elevation = max_row["elevation_deg"]

pass_duration = los_time - aos_time


# ============================================================
# 4. Plot elevation profile
# ============================================================

plt.figure(figsize=(12, 6))

plt.plot(
    strongest_pass["time_s"],
    strongest_pass["elevation_deg"],
    linewidth=2
)


# ============================================================
# 5. Mark AOS
# ============================================================

aos_row = strongest_pass.iloc[0]

plt.scatter(
    aos_row["time_s"],
    aos_row["elevation_deg"],
    s=70,
    zorder=5,
    label=f"AOS — {aos_time:.0f} s"
)


# ============================================================
# 6. Mark maximum elevation
# ============================================================

plt.scatter(
    max_time,
    max_elevation,
    s=90,
    zorder=5,
    label=f"Maximum — {max_elevation:.2f}°"
)

plt.annotate(
    f"Maximum elevation\n"
    f"{max_elevation:.2f}° @ {max_time:.0f} s",
    (max_time, max_elevation),
    xytext=(15, -45),
    textcoords="offset points",
    arrowprops=dict(arrowstyle="->"),
    fontsize=10
)


# ============================================================
# 7. Mark LOS
# ============================================================

los_row = strongest_pass.iloc[-1]

plt.scatter(
    los_time,
    los_row["elevation_deg"],
    s=70,
    zorder=5,
    label=f"LOS — {los_time:.0f} s"
)


# ============================================================
# 8. Add pass summary box
# ============================================================

summary = (
    f"AOS       : {aos_time:.0f} s\n"
    f"Duration  : {pass_duration:.0f} s\n"
    f"LOS       : {los_time:.0f} s"
)

plt.text(
    0.02,
    0.97,
    summary,
    transform=plt.gca().transAxes,
    verticalalignment="top",
    fontsize=10,
    bbox=dict(
        boxstyle="round",
        alpha=0.85
    )
)


# ============================================================
# 9. Formatting
# ============================================================

plt.xlabel("Time (seconds)")
plt.ylabel("Elevation (degrees)")

plt.title(
    "Satellite Pass Elevation Profile",
    fontsize=15
)

plt.axhline(
    0,
    linewidth=1
)

plt.grid(
    True,
    alpha=0.25
)

plt.legend()

plt.tight_layout()


# ============================================================
# 10. Save result
# ============================================================

plt.savefig(
    "pass_elevation.png",
    dpi=200,
    bbox_inches="tight"
)

plt.show()


# ============================================================
# 11. Print pass summary
# ============================================================

print("\nPass elevation profile")
print("----------------------")

print(f"AOS              : {aos_time:.0f} s")
print(f"Maximum Elevation : {max_elevation:.2f} deg")
print(f"Time of Max       : {max_time:.0f} s")
print(f"LOS              : {los_time:.0f} s")
print(f"Pass Duration     : {pass_duration:.0f} s")

print("\nSaved as pass_elevation.png")