#pragma once

#include <string>
#include "Vector3.h"
#include "Dynamics.h"

#include<vector>
#include <cmath>

namespace OrbitForge {
    enum class BodyType {
        STAR,
        PLANET,
        PLANETARY_MOON,
        ASTEROID,
        MANMADE
    };

    struct CelestialBody {
        public:
        // position and velocity
        std::string name;
        BodyType body_type;

        char shape;
        double mass;
        double radius;
        Dynamics::Vector3 r;
        Dynamics::Vector3 v;
        Dynamics::Vector3 force;
        bool draw;
        bool is_kinematic;

        bool first_tick_complete;

        std::vector<CelestialBody*> ignore_bodies;
            
        CelestialBody(std::string body_name, BodyType type, double body_mass, double body_radius, char body_shape, Dynamics::Vector3 init_position, Dynamics::Vector3 init_velocity, bool draw_body, bool is_kinem);
        CelestialBody(std::string body_name, BodyType type, double body_mass, double body_radius, char body_shape, Dynamics::Vector3 init_position, Dynamics::Vector3 init_velocity, bool draw_body, bool is_kinem, std::vector<CelestialBody*> bodies_to_ignore);
        
        
        CelestialBody();
    };

    // completely relative data that stores the orbital elements of a celestial body in orbit around another body
    struct Orbit {
        public:
        CelestialBody *pgb, *orbiting_body;

        double mu = pgb->mass * Dynamics::G;

        double semi_major_axis;
        double semi_latus_rectum;
        double eccentricity;
        double inclination;
        double longitude_of_ascending_node;

        double longitude_of_periapsis;
        double arg_of_periapsis;

        double time_of_periapsis;

        double true_anomaly_at_epoch;
        double arg_of_latitude_at_epoch;
        double true_latitude_at_epoch;

        Dynamics::Vector3 h = Dynamics::Vector3::zero;
        Dynamics::Vector3 e = Dynamics::Vector3::zero;
        Dynamics::Vector3 n = Dynamics::Vector3::zero;
        Dynamics::Vector3 r = Dynamics::Vector3::zero;
        Dynamics::Vector3 v = Dynamics::Vector3::zero;


        Orbit(double _a, double _e, double _i, double _omega, double _w);
        Orbit();

        void update_osculating_elements() {
            h = Dynamics::Vector3::cross(r, v);

            e = ( r * (v.magnitude_squared() - mu/r.magnitude()) - v * Dynamics::Vector3::dot(r, v) ) / mu;

            // n = K x h, where K is the unit vector in the z direction
            n = Dynamics::Vector3::cross(Dynamics::Vector3(0, 0, 1), h);

            semi_latus_rectum = h.magnitude_squared()/mu;

            inclination = std::acos(h.z/h.magnitude());
            bool planes_coincide = (n==Dynamics::Vector3::zero);

            arg_of_periapsis = std::acos(Dynamics::Vector3::dot(n, e)/n.magnitude()/e.magnitude());
            true_anomaly_at_epoch = std::acos(Dynamics::Vector3::dot(e, r)/e.magnitude()/r.magnitude());

            arg_of_latitude_at_epoch = std::acos(Dynamics::Vector3::dot(n, r)/n.magnitude()/r.magnitude());

            if (planes_coincide == false)
            {
                longitude_of_periapsis = longitude_of_ascending_node + arg_of_periapsis;
            }

            true_latitude_at_epoch = longitude_of_periapsis + true_anomaly_at_epoch;
            

        }
    };
}