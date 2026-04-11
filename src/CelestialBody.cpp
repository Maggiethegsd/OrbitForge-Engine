#include "CelestialBody.h"

/* @brief Physical body for the simulation with given mass, rendering radius, rendering shape, initial position and more
   @param body_name Name of the body (must be unique to avoid discrepancies)
   @param type Type of body
   @param body_mass Mass of body (in any unit, handling is your work!)
   @param body_radius Radius of body to be rendered, for Matplotlib
   @param body_shape Shape of body to be rendered, for Matplotlib
   @param init_position Body spawn position
   @param init_velocity Body spawning velocity
   @param draw_body Whether to actually draw in the render, or just keep for physical effects
*/
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

    // Eccentricity of orbit w.r.t primary gravitational body
    double orbit_eccentricity = 0.0;
    // Latest time of periapsis of body around host
    double periapsis_time = 0.0;
    // True anomaly from previous tick (to track periapsis)
    double previous_true_anomaly = 0.0;
    // Whether body is in simulation's first tick
    bool first_tick_complete = false;
}