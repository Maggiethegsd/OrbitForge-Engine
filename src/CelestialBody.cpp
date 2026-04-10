#include "CelestialBody.h"

CelestialBody::CelestialBody(std::string body_name, BodyType type, double body_mass, double body_radius, char body_shape, 
                             Vector3 init_position, Vector3 init_velocity, bool draw_body) {
    name = body_name;
    body_type = type;
    mass = body_mass;
    radius = body_radius;
    shape = body_shape;
    r = init_position;
    v = init_velocity;
    force = Vector3::zero;
    draw = draw_body;
    
    // tracks dot of positon of PGB w.r.t body and velocity of body
    // eccentricity w.r.t primary body
    double orbit_eccentricity = 0.0;
    double periapsis_time = 0.0;
    double previous_true_anomaly = 0.0;

    bool first_tick_complete = false;
}