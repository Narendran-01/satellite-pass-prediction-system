Satellite Pass Prediction System

Predicts satellite passes over a ground station using SGP4 orbit propagation — computing AOS, LOS, maximum elevation, and real-time Doppler shift from live TLE data. Built to understand the full signal chain from orbital mechanics to ground station reception.

Overview

This project takes a satellite Two-Line Element (TLE) set and propagates the satellite orbit using the SGP4 model. The propagated state is processed through a sequence of coordinate transformations and ground-station calculations to determine whether the satellite is visible from a selected ground station.

The system calculates:

Satellite position and velocity
Ground-track latitude and longitude
Ground-station azimuth and elevation
Slant range and range rate
Received frequency and Doppler shift
Acquisition of Signal (AOS)
Maximum elevation
Loss of Signal (LOS)

The simulation generates a complete 24-hour tracking dataset and identifies all satellite passes over the ground station.

System Architecture
Coordinate Reference Frames

The SGP4 propagator outputs the satellite state in the TEME (True Equator, Mean Equinox) reference frame. The system transforms this into ENU (East-North-Up) coordinates relative to the ground station to compute azimuth, elevation, and range.

Pass Prediction

Satellite visibility is determined using the elevation angle relative to the ground station:

Event	Condition
AOS (Acquisition of Signal)	Elevation crosses from < 0° to ≥ 0°
Maximum Elevation	Highest elevation reached during the pass
LOS (Loss of Signal)	Elevation crosses from ≥ 0° to < 0°
Doppler Shift

The system calculates the expected Doppler shift of a communication signal using:

Transmitted frequency
Relative range rate between satellite and ground station
Speed of light

For a nominal carrier frequency of 437 MHz, the received frequency varies according to the relative motion between satellite and ground station, allowing the system to estimate the frequency offset a ground station would observe during a pass.

Simulation

The system was tested over a complete 24-hour simulation at one-second resolution:

86,401 time samples covering 0 s → 86,400 s
Each sample contains: orbital state, tracking geometry, communication parameters, and visibility flags
Output is logged to CSV and excluded from version control via .gitignore
Example Pass Result

One detected pass from the 24-hour simulation:

AOS Time          : 79351 s
LOS Time          : 80005 s
Pass Duration     : 654 s
Maximum Elevation : 64.09 deg
Time of Max Elev. : 79677 s

At maximum elevation:

Time              : 79677 s
Elevation         : 64.09 deg
Azimuth           : 233.62 deg
Range             : 468.94 km
Range Rate        : 16.47 m/s
Doppler Shift     : -24.01 Hz
Technology Stack
Component	Detail
Language	C++
Orbit Propagation	SGP4 via perturb library
Orbital Data	Two-Line Element (TLE) sets
Output	CSV data logging
External Dependency

This project uses perturb, a modern C++11 wrapper for SGP4 orbit propagation, developed by Gunvir Ranu (MIT License).

perturb handles TLE parsing and SGP4 propagation, providing the satellite state in the TEME reference frame. All coordinate transformations, ground-station geometry, pass detection, and Doppler calculations are implemented independently in this project.

Building

Note: perturb is an external dependency and must be obtained separately from its repository before building.

After cloning this repository and building perturb, compile with:

bash
g++ orbit.cpp -I".\perturb\include" -L".\perturb\examples\cmake-local\build\build" -lperturb -o orbit_sgp4

Run the executable:

bash
.\orbit_sgp4.exe

The program generates tracking data and simulation output locally.

Repository Structure
satellite-pass-prediction-system/
│
├── .gitignore
├── README.md
└── orbit.cpp

Generated files (executables, CSV datasets, telemetry output) are excluded from version control.

Development Progression

The system evolved through two distinct stages:

Stage 1 — Numerical Integration Model

An earlier version implemented orbital propagation using a custom numerical integrator. This stage was used to understand and validate:

Gravitational acceleration and J2 perturbation
Numerical integration methods
ECI/ECEF coordinate transformations
Ground-station geometry (ENU, azimuth, elevation, range, range rate)
Doppler calculations

Stage 2 — SGP4 Transition

The system was subsequently transitioned to TLE-driven SGP4 propagation. The final architecture uses a real TLE as the orbital state source rather than a manually defined initial position and velocity, significantly improving real-world accuracy.

Limitations

This is a software-based satellite tracking and pass prediction system and should not be interpreted as a flight-qualified spacecraft navigation system. Accuracy is affected by:

TLE age and quality
SGP4 model assumptions
Coordinate transformation precision
Ground-station coordinate accuracy
Atmospheric and environmental effects not modelled
Future Improvements
Automated TLE retrieval
Multiple satellite and ground station support
Configurable elevation masks
Improved Earth orientation modelling
Real-time tracking mode
Interactive ground-track visualization
Antenna pointing command output
SDR hardware integration
Automated frequency correction based on Doppler prediction
Author

Narendran S
ECE Final Year | Embedded Systems & Space Technology
