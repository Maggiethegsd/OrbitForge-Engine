#pragma once

#include<vector>

#include "CelestialBody.h"

extern const double G;
void simulation_step(std::vector<CelestialBody>& celestial_bodies, CelestialBody& rocket, bool rocket_launched, double dt);
double get_orbit_u(double pgb_mass, double orbit_radius, double G);
double get_orbit_time(double pgb_mass, double orbit_radius, double G);
Vector3 calculate_gravitational_force(const CelestialBody& body, const CelestialBody& attractor);
Vector3 solve_lambert(Vector3 pos_earth, Vector3 pos_target, double time_of_flight, double G);