#pragma once

#include<vector>

#include "CelestialBody.h"
/* @brief Universal gravitational constant
   @warning Adjust according to units used for the system, don't blindly plug and chug 6.67x10^-11!
*/
extern const double G;

/* @brief Simulate and apply next physical state to a group of celestial bodies and rocket by applying interactive forces, by a timestep.
    * * @param celestial_bodies Bodies to simulate
    * * @param rocket Rocket used for the mission (to be handled separately in the future)
    * * @param G rocket_launched Rocket launched status (for handling forces on it and avoiding Earth's hill sphere)
    * * @param dt Timestep 
    * * @param lock_pgb Locks primary gravitational body (like the sun) in place to avoid excessive Barycentric drift
    * * @warning Make sure dt and G have same units of time! 
    */ 
void simulation_step(std::vector<CelestialBody>& celestial_bodies, CelestialBody& rocket, bool rocket_launched, double dt, bool lock_pgb);

/* @brief Calculates initial velocity required for perfect circular orbit with given radius, around given primary gravitational body
    * * @param pgb_mass Mass of body to be orbited around
    * * @param orbit_radius Radius of orbit
    * * @param G Universal Gravitational constant
    * * @return Required initial velocity in relative units of G and orbit radius
*/
double get_orbit_u(double pgb_mass, double orbit_radius, double G);

/* @brief Calculates orbital time period around given primary gravitational body, with given radius
    * * @param pgb_mass Mass of body we are orbiting around
    * * @param orbit_radius Radius of orbit
    * * @param G Universal Gravitational constant
    * * @warning Make sure units of length and mass followed by G are the same!
*/
double get_orbit_time(double pgb_mass, double orbit_radius, double G);

/* @brief Calculates initial velocity required for elliptical orbit with given semi-major axis, around given primary gravitational body
    * * @param pgb_mass Mass of body to be orbited around
    * * @param semi_major Length of semi-major axis 
    * * @param G Universal Gravitational constant
    * * @return Required initial velocity in relative units of G and semi-major axis
    * * @warning Make sure units of length and mass followed by G are the same!
*/
double get_elliptical_orbit_u(double pgb_mass, double semi_major, double G, double eccentricity);

/* @brief Calculates true anomaly of body w.r.t an elliptical orbit
    * * @param x x coordinate of body, relative to geometric center of ellipse
    * * @param y y coordinate of body, relative to geometric center of ellipse
    * * @param semi_major Length of semi-major axis of orbit
    * * @param eccentricity Eccentricity of given elliptical orbit (0-1)
    * * @return True anomaly of body in the elliptical orbit, in Radians
    * * @warning - x and y are relative to geometric center of the ellipse, NOT focus or absolute!
    * * @warning - Make sure units of x, y and semi-major axis are the same!
*/
double get_true_anomaly(double x, double y, double semi_major, double eccentricity);

/* @brief Calculates eccentric anomaly of body w.r.t an elliptical orbit
    * * @param x x coordinate of body, relative to geometric center of ellipse
    * * @param y y coordinate of body, relative to geometric center of ellipse
    * * @param semi_major Length of semi-major axis of orbit
    * * @param eccentricity Eccentricity of given elliptical orbit (0-1)
    * * @return Eccentric anomaly of body in the elliptical orbit, in Radians
    * * @warning - x and y are relative to geometric center of the ellipse, NOT focus or absolute!
    * * @warning - Make sure units of x, y and semi-major axis are the same!
*/
double get_eccentric_anomaly(double x, double y, double semi_major, double eccentricity);

/* @brief Clamps given angle to between 0-2π
    * * @param theta Angle in radians
    * * @return Equivalent angle in unit circle in radians (0-2π)
*/
double clamp_angle(double theta);

/* @brief Convert given eccentric anomaly in given elliptical orbit to cartesian coordinates
    * * @param E Eccentric anomaly of body
    * * @param eccentricity Eccentricity of given elliptical orbit (0-1)
    * * @param semi_major Semi-major axis of given elliptical orbit
    * * @return Cartesian coordinates of body, with geometric center of the elliptical orbit as the origin
    * * @warning - Again, returned coordinates are relative to geometric center, NOT absolute!
*/
Vector3 eccentric_to_cartesian(double E, double semi_major, double eccentricity);

/* @brief Calculate singular, raw acceleration experienced on a body due to another. Eliminates requirement of affected body's mass.
    * * @param pos Position vector of body being affected
    * * @param attractor_pos Position vector of affector
    * * @param attractor_mass Mass of attractor
    * * @return Acceleration on affected body due to affector (in units of that used by position vectors and attractor mass!)
*/
Vector3 calculate_raw_acceleration(Vector3 pos, Vector3 attractor_pos, double attractor_mass);

/* @brief on a body due to another. Eliminates requirement of body mass.
    * * @param pos Position vector of body being affected
    * * @param attractor_pos Position vector of affector
    * * @param attractor_mass Mass of attractor
    * * @return Cartesian coordinates of body, with geometric center of the elliptical orbit as the origin
*/

/* @brief Calculate future position of body in perfectly elliptical orbit after time t with Kepler's equations.
    * * @param pgb_r Position vector of primary gravitational body at focus
    * * @param pgb_mass Mass of primary gravitational body
    * * @param G Universal Gravitational constant
    * * @param time Amount of time in future to predict for 
    * * @param semi_major Semi-major axis of given elliptical orbit 
    * * @param eccentricity Eccentricity of given elliptical orbit
    * * @param eccentric_anomaly Current eccentric anomaly of body in orbit
    * * @warning - Make sure units of time, length and mass for G and semi-major axis are same!
    * * @warning - Calculates absolute positions, NOT relative to foci or geometric center!
*/
Vector3 get_future_position_theoretical(Vector3 pgb_r, double pgb_mass, double G, double time, double semi_major, double eccentricity, double eccentric_anomaly);

/* @brief Simple as it gets - Calculate gravitational force on body due to affector.
    * * @param body Affected body
    * * @param body Affector body
    * * @return Force vector from affected body towards affector
    * * @warning - Uses system's value of universal gravitational constant (scaled in AU, days and SM)
    * * @warning - So an overload is provided for custom G input.
*/
Vector3 calculate_gravitational_force(const CelestialBody& body, const CelestialBody& attractor);

/* @brief Simple as it gets - Calculate gravitational force on body due to affector (with user-provided G).
    * * @param body Affected body
    * * @param body Affector body
    * * @return Force vector from affected body towards affector
    * * @param G Value of Universal Gravitational constant
*/
Vector3 calculate_gravitational_force(const CelestialBody& body, const CelestialBody& attractor, double ugc);

/* @brief Calculate nominal path taken by body, with provided orbit around primary gravitational body and in given time.
   * * @param pgb_r Current position vector of primary gravitational body
   * * @param pgb_mass Mass of primary gravitational body
   * * @param a Semi-major axis of given orbit
   * * @param e Eccentricity of given orbit
   * * @param E_initial Initial eccentric anomaly of given orbit
   * * @param time_of_flight Time of flight to trace path for
   * * @param dt Time-step
   * * @return List of position vectors tracing the path
   * * @warning - Make sure all values have same units of mass and time!
*/
std::vector<Vector3> get_nominal_path(Vector3 pgb_r, double pgb_mass, double a, double e, double E_initial, double time_of_flight, double dt);

/* @brief Calculate Encke perturbations on given nominal path due to affectors.
   * * @param target_name Name of body the nominal path is of 
   * * @param nominal_path Nominal path of body
   * * @param dt Time-step
   * * @param affectors All affectors in universe
   * * @param pgb_r Current position vector of primary gravitational body
   * * @param pgb_mass Mass of primary gravitational body
   * * @return Final position of target with encke perturbations applied
*/
Vector3 calculate_encke_deviation(std::string target_name, std::vector<Vector3>& nominal_path, double dt, const std::vector<CelestialBody>& affectors, Vector3 pgb_r, double pgb_mass);

/* @brief Solve Lamberts problem for required initial velocity to intercept target from initial position in set duration. Uses 1960 NASA algorithm developed by Lancaster & Blanchard. 
   * * @param ri Initial position
   * * @param rf Target position
   * * @param pgb_r P
   * * @param affectors All affectors in universe
   * * @param pgb_r Current position vector of primary gravitational body
   * * @param M Mass of primary gravitational body
   * * @param t1 Starting time
   * * @param t2 Reaching time
   * * @param G Universal gravitational constant
   * * @return Required initial velocity vector
*/
Vector3 solve_lambert(Vector3 ri, Vector3 rf, Vector3 pgb_r, double M, double t1, double t2, double G);

/* @brief Calculate trajectory of body in gravitational force field.
   * * @param body_mass Mass of body
   * * @param init_pos Current/initial position of body
   * * @param init_vel Current/initial velocity of body
   * * @param affectors All affectors in universe
   * * @param prediction_steps How far ahead to predict (in steps)
   * * @param dt time-step
   * * @param only_affected_by_pgb Turn on if body is only to be affected by primary gravitational body (only affected by sun)
   * * @return List of vectors tracing the physical trajectory
*/
std::vector<Vector3> calculate_trajectory(double body_mass, Vector3 init_pos, Vector3 init_vel, const std::vector<CelestialBody>& affectors, double prediction_steps, double dt, bool only_affected_by_pgb);