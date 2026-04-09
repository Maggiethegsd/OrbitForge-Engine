#pragma once

#include<cmath>

#include "SolarData.h"
#include "Simulation.h"

const double G = 2.95e-4;

// initial velocity for perfect circular orbit around given primary gravitational body and orbit radius
double get_orbit_u(double pgb_mass, double orbit_radius, double G)
{
    return sqrt(G*pgb_mass/orbit_radius);
}
// time period (days) for perfect circular orbit around given primary gravitational body and orbit radius
double get_orbit_time(double pgb_mass, double orbit_radius, double G)
{
    return 2*3.141592*sqrt(pow(orbit_radius, 3)/(G*pgb_mass));
}

Vector3 calculate_gravitational_force(const CelestialBody& body, const CelestialBody& attractor)
{
    Vector3 r = (attractor.r-body.r);
    double r_sqrd = r.magnitude_squared();

    if (r_sqrd<1e-9) {
        return Vector3::zero;
    }

    return (r.normalized()) * (G*body.mass*attractor.mass)/r_sqrd;
}

Vector3 solve_lambert(Vector3 r1, Vector3 r2, double t, double G)
{
    double theta = Vector3::angle(r1, r2);

    double c_sqr = r1.magnitude_squared() + r2.magnitude_squared() - (2 * r1.magnitude() * r2.magnitude() * cos(theta));
}

void simulation_step(std::vector<CelestialBody>& celestial_bodies, CelestialBody& rocket, bool rocket_launched, double dt)
{
    for (auto&body : celestial_bodies) {
        body.force=Vector3::zero;
    }

    if (rocket_launched) rocket.force=Vector3::zero;

    for (int i = 0; i < celestial_bodies.size(); i++) {
        for (int j = 0; j < celestial_bodies.size(); j++) {
            if (i==j) continue;
            celestial_bodies[i].force += calculate_gravitational_force(celestial_bodies[i], celestial_bodies[j]);
        }
        
        if (rocket_launched)
            rocket.force+=calculate_gravitational_force(rocket, celestial_bodies[i]);
    }

    for (auto& body:celestial_bodies)
    {
        Vector3 acc = body.force/body.mass;
        body.v += acc*dt;
        body.r += body.v*dt;
    }

    if (rocket_launched)
    {
        Vector3 acc = rocket.force/rocket.mass;
        rocket.v+=acc*dt;
        rocket.r+=rocket.v*dt;
    }
}
