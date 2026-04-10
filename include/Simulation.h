#pragma once

#include<vector>

#include "CelestialBody.h"

extern const double G;
void simulation_step(std::vector<CelestialBody>& celestial_bodies, CelestialBody& rocket, bool rocket_launched, double dt, bool lock_pgb);
double get_orbit_u(double pgb_mass, double orbit_radius, double G);
double get_orbit_time(double pgb_mass, double orbit_radius, double G);
double get_elliptical_orbit_u(double pgb_mass, double orbit_radius, double G, double eccentricity);
double get_true_anomaly(double x, double y, double semi_major, double eccentricity);
double get_eccentric_anomaly(double x, double y, double semi_major, double eccentricity);
double clamp_angle(double theta);
Vector3 eccentric_to_cartesian(double E, double semi_major, double eccentricity);
Vector3 calculate_raw_acceleration(Vector3 pos, Vector3 attractor_pos, double attractor_mass);
Vector3 get_future_position_theoretical(Vector3 pgb_r, double pgb_mass, double G, double time, double semi_major, double eccentricity, double eccentric_anomaly);
Vector3 calculate_gravitational_force(const CelestialBody& body, const CelestialBody& attractor);
std::vector<Vector3> get_nominal_path(Vector3 pgb_r, double pgb_mass, double a, double e, double E_initial, double time_of_flight, double dt);
Vector3 calculate_encke_deviation(std::string target_name, std::vector<Vector3>& nominal_path, double dt, const std::vector<CelestialBody>& affectors, Vector3 pgb_r, double pgb_mass);
Vector3 solve_lambert(Vector3 ri, Vector3 rf, Vector3 pgb_r, double M, double t1, double t2, double G);
std::vector <Vector3> calculate_trajectory(double body_mass, Vector3 init_pos, Vector3 init_vel, std::vector<CelestialBody> affectors, double prediction_steps, double dt, bool only_affected_by_pgb);