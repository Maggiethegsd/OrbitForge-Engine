#include "CelestialBody.h"
#include "vector"

using namespace OrbitForge;

namespace OrbitForge {
   /* @brief Physical body for the simulation with given mass, rendering radius, rendering shape, initial position and more
      @param body_name Name of the body (must be unique to avoid discrepancies)
      @param type Type of body
      @param body_mass Mass of body (in any unit, handling is your work!)
      @param body_radius Radius of body to be rendered, for Matplotlib
      @param body_shape Shape of body to be rendered, for Matplotlib
      @param init_position Body spawn position
      @param init_velocity Body spawning velocity
      @param draw_body Whether to actually draw in the render, or just keep for physical effects
      @param is_kinem Whether body is actively affected by physics (just gravitational force for now)
   */
   CelestialBody::CelestialBody(std::string body_name, BodyType type, double body_mass, double body_radius, char body_shape, 
                              Dynamics::Vector3 init_position, Dynamics::Vector3 init_velocity, bool draw_body, bool is_kinem) {
      name = body_name;
      body_type = type;
      mass = body_mass;
      radius = body_radius;
      shape = body_shape;
      r = init_position;
      v = init_velocity;
      force = Dynamics::Vector3::zero;
      draw = draw_body;
      is_kinematic = is_kinem;
      
      // Whether body is in simulation's first tick
      first_tick_complete = false;
   }

   CelestialBody::CelestialBody(std::string body_name, BodyType type, double body_mass, double body_radius, char body_shape, Dynamics::Vector3 init_position, Dynamics::Vector3 init_velocity, bool draw_body, bool is_kinem, std::vector<CelestialBody*> bodies_to_ignore)
   {
      name = body_name;
      body_type = type;
      mass = body_mass;
      radius = body_radius;
      shape = body_shape;
      r = init_position;
      v = init_velocity;
      force = Dynamics::Vector3::zero;
      draw = draw_body;
      is_kinematic = is_kinem;

      ignore_bodies = bodies_to_ignore;
      
      // Whether body is in simulation's first tick
      first_tick_complete = false;
   }
}