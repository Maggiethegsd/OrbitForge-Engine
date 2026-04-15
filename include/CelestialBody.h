#pragma once

#include <string>
#include "Vector3.h"

#include<vector>

namespace OrbitForge {
    enum class BodyType {
        STAR,
        PLANET,
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
}