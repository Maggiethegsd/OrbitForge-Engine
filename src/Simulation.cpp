#pragma once

#include<cmath>
#include <vector>
#include<numbers>

#include "SolarData.h"
#include "Simulation.h"
#include "Vector3.h"

const double G = 2.95e-4;
const double PI  =3.1415926;


// initial velocity for perfect circular orbit around given primary gravitational body and orbit radius
inline double get_orbit_u(double pgb_mass, double orbit_radius, double G)
{
    return sqrt(G*pgb_mass/orbit_radius);
}
// time period (days) for perfect circular orbit around given primary gravitational body and orbit radius
inline double get_orbit_time(double pgb_mass, double orbit_radius, double G)
{
    return 2*PI*sqrt(pow(orbit_radius, 3)/(G*pgb_mass));
}

inline Vector3 calculate_gravitational_force(const CelestialBody& body, const CelestialBody& attractor)
{
    Vector3 r = (attractor.r-body.r);
    double r_sqrd = r.magnitude_squared();

    if (r_sqrd<1e-9) {
        return Vector3::zero;
    }

    return (r.normalized()) * (G*body.mass*attractor.mass)/r_sqrd;
}

std::vector <Vector3> calculate_trajectory(double body_mass, Vector3 init_pos, Vector3 init_vel, std::vector<CelestialBody> affectors, double prediction_steps, double dt)
{
    // create ghost object for body
    CelestialBody body_ghost("ghost", body_mass, 1, 'o', init_pos, init_vel, false);
    std::vector <Vector3> trajectory;
    for (double t=0; t<prediction_steps; t+=dt)
    {   
        // set ghost force to zero for current frame
        body_ghost.force = Vector3::zero;

        // set all affectors force to zero
        for (auto&body:affectors)
            body.force=Vector3::zero;

        // apply forces between all bodies
        for (int i = 0; i < affectors.size(); i++) {
            for (int j = 0; j < affectors.size(); j++) {
                if (i==j)
                    continue;
                affectors[i].force += calculate_gravitational_force(affectors[i], affectors[j]);
            }

            body_ghost.force += calculate_gravitational_force(body_ghost, affectors[i]);
        }


        // apply velocity and acceleration to all bodies
        for (auto& body:affectors)
        {
            Vector3 acc = body.force/body.mass;
            body.v += acc*dt;
            body.r += body.v*dt;
        }

        // apply velocity and acceleration to ghost
        Vector3 a = body_ghost.force/body_mass;
        body_ghost.v += a*dt;
        body_ghost.r += body_ghost.v*dt;

        // add ghost path dr to trajectory data
        trajectory.push_back(body_ghost.r);
    }

    return trajectory;
}


//Vector3 solve_lambert(Vector3 r1, Vector3 r2, double t, double G)
//{
//   double theta = Vector3::angle(r1, r2);
//
//    double c_sqr = r1.magnitude_squared() + r2.magnitude_squared() - (2 * r1.magnitude() * r2.magnitude() * cos(theta));
//
//    return Vector3::zero;
//}

inline void simulation_step(std::vector<CelestialBody>& celestial_bodies, CelestialBody& rocket, bool rocket_launched, double dt)
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
