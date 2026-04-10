#pragma once

#include <string>
#include "Vector3.h"

struct CelestialBody {
    public:
    // position and velocity
    std::string name;
    char shape;
    double mass;
    double radius;
    Vector3 r;
    Vector3 v;
    Vector3 force;
    bool draw;
        
    CelestialBody(std::string body_name, double body_mass, double body_radius, char body_shape, Vector3 init_position, Vector3 init_velocity, bool draw_body);
};