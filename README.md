\# Satellite Pass Prediction System



A C++ SGP4-based system for satellite orbit propagation, ground-station tracking, pass prediction, and Doppler-shift calculation.



\## Overview



This project takes a satellite Two-Line Element (TLE) set and propagates the satellite orbit using the SGP4 model. The propagated state is then processed through a sequence of coordinate transformations and ground-station calculations to determine whether the satellite is visible from a selected ground station.



The system calculates:



\- Satellite position and velocity

\- Ground-track latitude and longitude

\- Ground-station azimuth and elevation

\- Slant range

\- Range rate

\- Received frequency

\- Doppler shift

\- Acquisition of Signal (AOS)

\- Maximum elevation

\- Loss of Signal (LOS)



The simulation can generate a complete 24-hour tracking dataset and identify individual satellite passes over the ground station.



\---



**## System Architecture**



```text

&#x20;                   TLE

&#x20;                    |

&#x20;                    v

&#x20;             +-------------+

&#x20;             |     SGP4    |

&#x20;             |  propagation|

&#x20;             +-------------+

&#x20;                    |

&#x20;                    v

&#x20;                  TEME

&#x20;                    |

&#x20;                    v

&#x20;             TEME -> ECEF

&#x20;                    |

&#x20;                    v

&#x20;           Ground Station

&#x20;             Coordinates

&#x20;                    |

&#x20;                    v

&#x20;             ECEF -> ENU

&#x20;                    |

&#x20;         +----------+----------+

&#x20;         |          |          |

&#x20;         v          v          v

&#x20;     Azimuth   Elevation     Range

&#x20;         |          |          |

&#x20;         +----------+----------+

&#x20;                    |

&#x20;                    v

&#x20;             Range Rate

&#x20;                    |

&#x20;                    v

&#x20;              Doppler Shift

&#x20;                    |

&#x20;                    v

&#x20;            Pass Detection

&#x20;             /           \\

&#x20;           AOS       Maximum Elevation

&#x20;             \\           /

&#x20;                  LOS



**Coordinate Reference Frames**



The SGP4 propagator provides the satellite state in the TEME (True Equator, Mean Equinox) reference frame.



The project therefore performs the following transformation:

&#x09;

TEME

&#x20;|

&#x20;| Earth rotation transformation

&#x20;v

ECEF

&#x20;|

&#x20;| Ground-station relative vector

&#x20;v

ENU

&#x20;|

&#x20;+--> East

&#x20;+--> North

&#x20;+--> Up

&#x20;|

&#x20;+--> Azimuth

&#x20;+--> Elevation

&#x20;+--> Range

The ENU representation is then used to determine the satellite's position relative to the ground station.



**Pass Prediction**



Satellite visibility is determined using the elevation angle of the satellite relative to the ground station. 



The system identifies: 



Acquisition of Signal (AOS)- The satellite crosses from Elevation < 0°to Elevation >= 0° indicating that the satellite has risen above the local horizon.

Maximum Elevation- During a pass, the system tracks the highest elevation reached by the satellite.

Loss of Signal (LOS)- The satellite crosses from Elevation >= 0°to Elevation < 0° indicating that the satellite has fallen below the horizon.



**Doppler Shift**



The system also calculates the expected Doppler shift of a communication signal.



The calculation uses:

1.Transmitted frequency

2.Relative range rate between satellite and ground station

3.Speed of light



For a nominal carrier frequency of 437 MHz, the received frequency varies according to the relative motion between the satellite and the ground station.

This allows the system to estimate the frequency offset that a ground station would observe during a pass.



**Simulation**



The final system was tested over a complete 24-hour simulation.

The simulation generates one-second tracking samples, resulting in 86,401 time samples covering 0 s -> 86,400 s

The generated CSV contains the calculated orbital, tracking, communication, visibility, and conservation-related parameters for each sample.



Generated files are intentionally excluded from version control through .gitignore.





**## Example Pass Result**



One of the detected passes produced the following result:



AOS Time          : 79351 s

LOS Time          : 80005 s

Pass Duration     : 654 s

Maximum Elevation : 64.09 deg

Time of Max Elev. : 79677 s



At maximum elevation:



Time              : 79677 s

Elevation         : 64.09 deg

Azimuth            : 233.62 deg

Range              : 468.94 km

Range Rate         : 16.47 m/s

Doppler Shift      : -24.01 Hz



The complete simulation also identified additional passes during the 24-hour propagation period.



**## Technology Stack**



1.C++

2.SGP4

3.TLE orbit data

4.perturb library

5.PowerShell / Windows command line

6.CSV data logging



**## External Dependency**



This project uses perturb, a modern C++11 wrapper for SGP4 orbit propagation, developed by **Gunvir Ranu.**



The external library is used for TLE parsing and SGP4 orbit propagation, providing the satellite state in the TEME reference frame.

The coordinate transformations, ground-station geometry, tracking calculations, pass detection, and Doppler calculations surrounding the propagator are implemented as part of this project.



perturb is distributed under the MIT License.



Original repository: https://github.com/gunvirranu/perturb





**## Building**



This project currently uses the perturb library as an external dependency and is **not included in this repository**.

After cloning this repository, obtain the perturb library separately and build it according to its documentation.



The project can then be compiled using:

g++ orbit.cpp -I".\\perturb\\include" -L".\\perturb\\examples\\cmake-local\\build\\build" -lperturb -o orbit\_sgp4



Run the executable with:

.\\orbit\_sgp4.exe



The program generates the tracking data and simulation output locally.



**## Repository Structure**



satellite-pass-prediction-system/

|

+-- .gitignore

+-- README.md

+-- orbit.cpp



Generated files such as executables, CSV datasets, and telemetry output are excluded from version control.



**## Development Progression**



The final system evolved through several stages.



The earlier development stage implemented orbital propagation and coordinate transformations using a numerically integrated orbital model.

This stage was used to understand and validate:

Gravitational acceleration

J2 perturbation

Numerical integration

ECI/ECEF transformations

Ground-station geometry

ENU coordinates

Elevation and azimuth

Range and range rate

Doppler calculations

Transition to SGP4



The project was subsequently transitioned to TLE-driven SGP4 propagation.

The final architecture therefore uses a real TLE as the source of orbital state rather than relying on a manually defined initial position and velocity.



**## Limitations**



This project is a software-based satellite tracking and pass prediction system and should not be interpreted as a flight-qualified spacecraft navigation system.



Accuracy depends on factors including:



TLE age and quality

SGP4 model limitations

Coordinate transformation assumptions

Ground-station coordinates

Numerical precision

Atmospheric and environmental effects not represented by the model

Future Improvements



Possible future extensions include:



Automated TLE retrieval

Multiple satellite support

Multiple ground stations

Configurable elevation masks

Improved Earth orientation modeling

Real-time tracking

Interactive ground-track visualization

Antenna pointing commands

Integration with SDR hardware

Automated frequency correction based on Doppler prediction



Author

Narendran S



