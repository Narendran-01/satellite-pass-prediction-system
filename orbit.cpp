#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <string>
#include <perturb/perturb.hpp>

using namespace perturb;
using namespace std;

// ============================================================
// CubeSat Pass Predictor
// TLE -> SGP4 -> TEME -> ECEF -> ENU -> Pass/Doppler
// ============================================================

// Earth / physical constants (SI units unless stated otherwise)
const double EARTH_RADIUS = 6371000.0;       // m
const double EARTH_ROTATION_RATE = 7.2921151467e-5; // rad/s
const double SPEED_OF_LIGHT = 299792458.0;   // m/s
const double TX_FREQUENCY = 437000000.0;     // Hz

// Simulation settings
const double DT = 1.0;                       // s
const double SIMULATION_DURATION = 86400.0;  // s (24 hours)

// ------------------------------------------------------------
// Ground station
// ------------------------------------------------------------
struct GroundStation {
    double lat;   // radians
    double lon;   // radians
    double alt;   // metres
};

// Geodetic latitude/longitude/altitude -> ECEF.
// Current project uses a spherical-Earth approximation.
void ground_to_ecef(
    const GroundStation& gs,
    double& x,
    double& y,
    double& z)
{
    const double sin_lat = sin(gs.lat);
    const double cos_lat = cos(gs.lat);
    const double sin_lon = sin(gs.lon);
    const double cos_lon = cos(gs.lon);

    const double radius = EARTH_RADIUS + gs.alt;

    x = radius * cos_lat * cos_lon;
    y = radius * cos_lat * sin_lon;
    z = radius * sin_lat;
}

// ------------------------------------------------------------
// GMST
// ------------------------------------------------------------
double gmst(double jd)
{
    const double twopi = 2.0 * M_PI;

    const double tut1 = (jd - 2451545.0) / 36525.0;

    double temp =
        -6.2e-6 * tut1 * tut1 * tut1
        + 0.093104 * tut1 * tut1
        + (876600.0 * 3600.0 + 8640184.812866) * tut1
        + 67310.54841;

    temp = fmod(temp * M_PI / 180.0 / 240.0, twopi);

    if (temp < 0.0)
        temp += twopi;

    return temp;
}

// ------------------------------------------------------------
// TEME -> ECEF
//
// perturb/SGP4 provides position in km and velocity in km/s.
// This function converts them to metres and m/s so the rest
// of the project can consistently use SI units.
// ------------------------------------------------------------
void teme_to_ecef(
    const StateVector& sv,
    double jd,
    double& x_ecef,
    double& y_ecef,
    double& z_ecef,
    double& vx_ecef,
    double& vy_ecef,
    double& vz_ecef)
{
    const double theta = gmst(jd);

    const double c = cos(theta);
    const double s = sin(theta);

    // TEME position: km -> m
    const double x_teme = sv.position[0] * 1000.0;
    const double y_teme = sv.position[1] * 1000.0;
    const double z_teme = sv.position[2] * 1000.0;

    // Rotate TEME position into Earth-fixed coordinates
    x_ecef =  c * x_teme + s * y_teme;
    y_ecef = -s * x_teme + c * y_teme;
    z_ecef =  z_teme;

    // TEME velocity: km/s -> m/s
    const double vx_teme = sv.velocity[0] * 1000.0;
    const double vy_teme = sv.velocity[1] * 1000.0;
    const double vz_teme = sv.velocity[2] * 1000.0;

    const double vx_rot =  c * vx_teme + s * vy_teme;
    const double vy_rot = -s * vx_teme + c * vy_teme;
    const double vz_rot =  vz_teme;

    // Earth-rotation correction
    vx_ecef = vx_rot + EARTH_ROTATION_RATE * y_ecef;
    vy_ecef = vy_rot - EARTH_ROTATION_RATE * x_ecef;
    vz_ecef = vz_rot;
}

// ------------------------------------------------------------
// ECEF -> latitude / longitude / altitude
//
// Current project uses a spherical-Earth approximation.
// ------------------------------------------------------------
void ecef_to_latlon(
    double x,
    double y,
    double z,
    double& lat,
    double& lon,
    double& alt)
{
    const double rho = sqrt(x * x + y * y);
    const double r = sqrt(x * x + y * y + z * z);

    lat = atan2(z, rho);
    lon = atan2(y, x);
    alt = r - EARTH_RADIUS;
}

// ------------------------------------------------------------
// ECEF relative vector -> local ENU
// ------------------------------------------------------------
void ecef_to_enu(
    double dx,
    double dy,
    double dz,
    double lat,
    double lon,
    double& east,
    double& north,
    double& up)
{
    east =
        -sin(lon) * dx
        + cos(lon) * dy;

    north =
        -sin(lat) * cos(lon) * dx
        - sin(lat) * sin(lon) * dy
        + cos(lat) * dz;

    up =
        cos(lat) * cos(lon) * dx
        + cos(lat) * sin(lon) * dy
        + sin(lat) * dz;
}

// ------------------------------------------------------------
// Human-readable telemetry
// ------------------------------------------------------------
void printTelemetry(
    ofstream& fout,
    double t,
    double latitude_deg,
    double longitude_deg,
    double elevation_deg,
    double azimuth_deg,
    double range_km,
    double range_rate,
    double received_frequency_MHz,
    double doppler_shift,
    bool visible)
{
    fout << fixed << setprecision(2);

    fout << "============================================================\n";
    fout << "                 SATELLITE TELEMETRY\n";
    fout << "============================================================\n\n";

    fout << "Simulation Time     : " << t << " s\n\n";

    fout << "Satellite Position\n";
    fout << "------------------\n";
    fout << "Latitude            : " << latitude_deg << " deg\n";
    fout << "Longitude           : " << longitude_deg << " deg\n\n";

    fout << "Tracking Data\n";
    fout << "-------------\n";
    fout << "Elevation           : " << elevation_deg << " deg\n";
    fout << "Azimuth             : " << azimuth_deg << " deg\n";
    fout << "Range               : " << range_km << " km\n";
    fout << "Range Rate          : " << range_rate << " m/s\n\n";

    fout << setprecision(6);
    fout << "Communication\n";
    fout << "-------------\n";
    fout << "Received Frequency  : " << received_frequency_MHz << " MHz\n";

    fout << setprecision(2);
    fout << "Doppler Shift       : " << doppler_shift << " Hz\n\n";

    fout << "Visibility Status   : "
         << (visible ? "VISIBLE" : "BELOW HORIZON") << "\n";

    fout << "Motion              : "
         << (range_rate < 0 ? "APPROACHING" : "RECEDING") << "\n\n";

    fout << "============================================================\n\n";
}

// ------------------------------------------------------------
// Pass summary
// ------------------------------------------------------------
void printPassSummary(
    ofstream& fout,
    double aos_time,
    double los_time,
    double pass_duration,
    double max_elevation,
    double max_elevation_time)
{
    fout << fixed << setprecision(2);

    fout << "************************************************************\n";
    fout << "                     PASS SUMMARY\n";
    fout << "************************************************************\n\n";

    fout << "AOS Time             : " << aos_time << " s\n";
    fout << "LOS Time             : " << los_time << " s\n";
    fout << "Pass Duration        : " << pass_duration << " s\n";
    fout << "Maximum Elevation    : " << max_elevation << " deg\n";
    fout << "Time of Max Elev.    : " << max_elevation_time << " s\n\n";

    fout << "************************************************************\n\n";
}

int main()
{
    // --------------------------------------------------------
    // 1. TLE
    // --------------------------------------------------------
    string tle1 =
        "1 25544U 98067A   22071.78032407  .00021395  00000-0  39008-3 0  9996";

    string tle2 =
        "2 25544  51.6424  94.0370 0004047 256.5103  89.8846 15.49386383330227";

    auto sat = Satellite::from_tle(tle1, tle2);

    if (sat.last_error() != Sgp4Error::NONE)
    {
        cout << "SGP4 initialization failed!" << endl;
        return 1;
    }

    // Reference epoch used for the simulation.
    const auto start_time =
        JulianDate(DateTime { 2022, 3, 14, 1, 59, 26.535 });

    // --------------------------------------------------------
    // 2. Verify initial SGP4 state
    // --------------------------------------------------------
    StateVector initial_state;

    const auto initial_error =
        sat.propagate(start_time, initial_state);

    if (initial_error != Sgp4Error::NONE)
    {
        cout << "SGP4 propagation failed at initial time!" << endl;
        return 1;
    }

    cout << "SGP4 Position [km]: "
         << initial_state.position[0] << ", "
         << initial_state.position[1] << ", "
         << initial_state.position[2] << endl;

    cout << "SGP4 Velocity [km/s]: "
         << initial_state.velocity[0] << ", "
         << initial_state.velocity[1] << ", "
         << initial_state.velocity[2] << endl;

    // --------------------------------------------------------
    // 3. Output files
    // --------------------------------------------------------
    ofstream csv("orbit.csv");

    if (!csv)
    {
        cout << "Failed to create orbit.csv\n";
        return 1;
    }

    csv << "time_s,"
        << "teme_x_km,teme_y_km,teme_z_km,"
        << "teme_vx_kms,teme_vy_kms,teme_vz_kms,"
        << "ecef_x_km,ecef_y_km,ecef_z_km,"
        << "latitude_deg,longitude_deg,altitude_km,"
        << "east_km,north_km,up_km,"
        << "elevation_deg,azimuth_deg,range_km,range_rate_mps,"
        << "received_frequency_MHz,doppler_shift_Hz,"
        << "visible\n";

    ofstream fout("output.txt");

    if (!fout)
    {
        cout << "Failed to create output.txt\n";
        return 1;
    }

    // --------------------------------------------------------
    // 4. Ground station
    // --------------------------------------------------------
    GroundStation gs;

    // Chennai, India
    gs.lat = 13.0827 * M_PI / 180.0;
    gs.lon = 80.2707 * M_PI / 180.0;
    gs.alt = 6.0;

    double gs_x, gs_y, gs_z;

    ground_to_ecef(gs, gs_x, gs_y, gs_z);

    // --------------------------------------------------------
    // 5. Pass-tracking state
    // --------------------------------------------------------
    bool was_visible = false;
    bool first_iteration = true;

    double aos_time = 0.0;
    double max_elevation = 0.0;
    double max_elevation_time = 0.0;

    // --------------------------------------------------------
    // 6. Main SGP4 simulation
    // --------------------------------------------------------
    for (double t = 0.0; t <= SIMULATION_DURATION; t += DT)
    {
        const auto current_time =
            start_time + t / 86400.0;

        StateVector current_state;

        const auto propagation_error =
            sat.propagate(current_time, current_state);

        if (propagation_error != Sgp4Error::NONE)
        {
            cout << "SGP4 propagation failed at t = "
                 << t << " s" << endl;
            break;
        }

        // Print a lightweight progress indicator every 60 seconds.
        if (fmod(t, 60.0) == 0.0)
        {
            cout << "t = " << t << " s"
                 << " | Position = ("
                 << current_state.position[0] << ", "
                 << current_state.position[1] << ", "
                 << current_state.position[2] << ") km"
                 << endl;
        }

        // ----------------------------------------------------
        // TEME -> ECEF
        // ----------------------------------------------------
        double x_ecef, y_ecef, z_ecef;
        double vx_ecef, vy_ecef, vz_ecef;

        const double current_jd =
            current_time - JulianDate(0.0);

        teme_to_ecef(
            current_state,
            current_jd,
            x_ecef,
            y_ecef,
            z_ecef,
            vx_ecef,
            vy_ecef,
            vz_ecef);

        // One-time coordinate-frame diagnostic.
        if (t == 0.0)
        {
            cout << "\nTEME -> ECEF CHECK\n";
            cout << "JD = " << current_jd << endl;

            cout << "ECEF Position [km]: "
                 << x_ecef / 1000.0 << ", "
                 << y_ecef / 1000.0 << ", "
                 << z_ecef / 1000.0 << endl;

            cout << "ECEF Velocity [km/s]: "
                 << vx_ecef / 1000.0 << ", "
                 << vy_ecef / 1000.0 << ", "
                 << vz_ecef / 1000.0 << endl;
        }

        // ----------------------------------------------------
        // Satellite geodetic position
        // ----------------------------------------------------
        double sat_lat, sat_lon, sat_alt;

        ecef_to_latlon(
            x_ecef,
            y_ecef,
            z_ecef,
            sat_lat,
            sat_lon,
            sat_alt);

        // ----------------------------------------------------
        // Relative vector: satellite - ground station
        // ----------------------------------------------------
        const double dx = x_ecef - gs_x;
        const double dy = y_ecef - gs_y;
        const double dz = z_ecef - gs_z;

        // ----------------------------------------------------
        // ECEF -> local ENU
        // ----------------------------------------------------
        double east, north, up;

        ecef_to_enu(
            dx,
            dy,
            dz,
            gs.lat,
            gs.lon,
            east,
            north,
            up);

        // One-time ENU diagnostic.
        if (t == 0.0)
        {
            const double test_elevation =
                atan2(up, sqrt(east * east + north * north));

            double test_azimuth =
                atan2(east, north);

            double test_elevation_deg =
                test_elevation * 180.0 / M_PI;

            double test_azimuth_deg =
                test_azimuth * 180.0 / M_PI;

            if (test_azimuth_deg < 0.0)
                test_azimuth_deg += 360.0;

            cout << "\nENU CHECK\n";
            cout << "East  = " << east / 1000.0 << " km\n";
            cout << "North = " << north / 1000.0 << " km\n";
            cout << "Up    = " << up / 1000.0 << " km\n";
            cout << "Elevation = " << test_elevation_deg << " deg\n";
            cout << "Azimuth   = " << test_azimuth_deg << " deg\n";
        }

        // ----------------------------------------------------
        // Azimuth and range
        // ----------------------------------------------------
        double azimuth =
            atan2(east, north);

        double azimuth_deg =
            azimuth * 180.0 / M_PI;

        if (azimuth_deg < 0.0)
            azimuth_deg += 360.0;

        const double range =
            sqrt(dx * dx + dy * dy + dz * dz);

        const double range_km =
            range / 1000.0;

        // ----------------------------------------------------
        // Elevation
        //
        // Keep the validated project formulation:
        // angle between ground-station radial direction
        // and the satellite line-of-sight vector.
        // ----------------------------------------------------
        const double dot =
            gs_x * dx +
            gs_y * dy +
            gs_z * dz;

        const double gs_mag =
            sqrt(gs_x * gs_x +
                 gs_y * gs_y +
                 gs_z * gs_z);

        const double cos_alpha =
            max(-1.0, min(1.0,
                dot / (gs_mag * range)));

        const double alpha =
            acos(cos_alpha);

        const double elevation =
            M_PI / 2.0 - alpha;

        const double elevation_deg =
            elevation * 180.0 / M_PI;

        const bool visible =
            (elevation_deg > 0.0);

        // ----------------------------------------------------
        // Range rate and Doppler
        // ----------------------------------------------------
        const double dot_rr =
            dx * vx_ecef +
            dy * vy_ecef +
            dz * vz_ecef;

        const double range_rate =
            dot_rr / range;

        const double received_frequency =
            TX_FREQUENCY *
            (1.0 - range_rate / SPEED_OF_LIGHT);

        const double doppler_shift =
            received_frequency - TX_FREQUENCY;

        // ----------------------------------------------------
        // AOS detection
        // ----------------------------------------------------
        if (first_iteration)
        {
            was_visible = visible;
            first_iteration = false;
        }

        if (!was_visible && visible)
        {
            aos_time = t;
            max_elevation = elevation_deg;
            max_elevation_time = t;

            fout << "\n";
            fout << "************************************************************\n";
            fout << "               ACQUISITION OF SIGNAL (AOS)\n";
            fout << "************************************************************\n";
            fout << "AOS Time : " << aos_time << " s\n\n";
        }

        // ----------------------------------------------------
        // Maximum elevation tracking
        // ----------------------------------------------------
        if (visible && elevation_deg > max_elevation)
        {
            max_elevation = elevation_deg;
            max_elevation_time = t;
        }

        // ----------------------------------------------------
        // LOS detection
        // ----------------------------------------------------
        if (was_visible && !visible)
        {
            const double los_time = t;
            const double pass_duration =
                los_time - aos_time;

            fout << "\n";
            fout << "************************************************************\n";
            fout << "                 LOSS OF SIGNAL (LOS)\n";
            fout << "************************************************************\n";
            fout << "LOS Time : " << los_time << " s\n\n";

            printPassSummary(
                fout,
                aos_time,
                los_time,
                pass_duration,
                max_elevation,
                max_elevation_time);
        }

        was_visible = visible;

        // ----------------------------------------------------
        // CSV: write EVERY simulation sample
        // ----------------------------------------------------
        const double lat_deg =
            sat_lat * 180.0 / M_PI;

        const double lon_deg =
            sat_lon * 180.0 / M_PI;

        csv << fixed << setprecision(6)
            << t << ","
            << current_state.position[0] << ","
            << current_state.position[1] << ","
            << current_state.position[2] << ","
            << current_state.velocity[0] << ","
            << current_state.velocity[1] << ","
            << current_state.velocity[2] << ","
            << x_ecef / 1000.0 << ","
            << y_ecef / 1000.0 << ","
            << z_ecef / 1000.0 << ","
            << lat_deg << ","
            << lon_deg << ","
            << sat_alt / 1000.0 << ","
            << east / 1000.0 << ","
            << north / 1000.0 << ","
            << up / 1000.0 << ","
            << elevation_deg << ","
            << azimuth_deg << ","
            << range_km << ","
            << range_rate << ","
            << received_frequency / 1e6 << ","
            << doppler_shift << ","
            << (visible ? 1 : 0)
            << "\n";

        // ----------------------------------------------------
        // Human-readable telemetry every 10 seconds
        // ----------------------------------------------------
        if (fmod(t, 10.0) < DT)
        {
            printTelemetry(
                fout,
                t,
                lat_deg,
                lon_deg,
                elevation_deg,
                azimuth_deg,
                range_km,
                range_rate,
                received_frequency / 1e6,
                doppler_shift,
                visible);
        }
    }

    csv.close();
    fout.close();

    cout << "\nSimulation complete.\n";
    cout << "Complete data saved to orbit.csv\n";
    cout << "Telemetry and pass summaries saved to output.txt\n";

    return 0;
}