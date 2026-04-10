#pragma once

#include <string>
#include "Vector3.h"

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
    Vector3 r;
    Vector3 v;
    Vector3 force;
    bool draw;

    double orbit_eccentricity;
    double previous_true_anomaly;
    double periapsis_time;
    bool first_tick_complete;
        
    CelestialBody(std::string body_name, BodyType type, double body_mass, double body_radius, char body_shape, Vector3 init_position, Vector3 init_velocity, bool draw_body);
};