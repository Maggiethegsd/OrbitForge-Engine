#include "CelestialBody.h"

CelestialBody::CelestialBody(std::string body_name, double body_mass, double body_radius, char body_shape, 
                             Vector3 init_position, Vector3 init_velocity, bool draw_body) {
    name = body_name;
    mass = body_mass;
    radius = body_radius;
    shape = body_shape;
    r = init_position;
    v = init_velocity;
    force = Vector3::zero;
    draw = draw_body;
}