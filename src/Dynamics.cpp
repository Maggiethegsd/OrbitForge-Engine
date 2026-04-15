#pragma once

#include<cmath>
#include <vector>

#include "SolarData.h"
#include "Dynamics.h"
#include "CelestialBody.h"
#include "Vector3.h"

using namespace OrbitForge;

namespace OrbitForge 
{
    namespace Dynamics
    {
        double get_orbit_u(double pgb_mass, double orbit_radius, double G)
        {
            // for circular orbit
            // mv^2/R = GMm/R^2
            return sqrt(G*pgb_mass/orbit_radius);
        }

        double get_elliptical_orbit_u(double pgb_mass, double semi_major, double G, double eccentricity)
        {
            //eccentricity correction if out of bounds
            if (eccentricity <= 0) eccentricity = 0;
            if (eccentricity >= 1.0) eccentricity = 0.999;

            // vis viva equation for initial velocity from periapsis
            return sqrt( (G*pgb_mass/semi_major) * (1 + eccentricity)/(1-eccentricity));
        }

        double clamp_angle(double theta)
        {
            // get remainder by 2PI
            theta = fmod(theta, 2*PI);
            
            // convert to range if negative
            if (theta < 0)
                theta += 2*PI;
                
            return theta;
        }

        double get_eccentric_anomaly(double x, double y, double semi_major, double eccentricity)
        {
            // e = root(1 - (b^2/a^2))
            double b = sqrt(1 - eccentricity*eccentricity);

            // y coordinate on auxiliary circle (w.r.t geometric center of ellipse)
            double y_ = y/b;

            double E = std::atan2(y_, x);
            E = clamp_angle(E);
            return E;
        }

        double get_true_anomaly(double x, double y, double semi_major, double eccentricity)
        {
            // distance of focus (right) = e*a
            double f_x = eccentricity*semi_major;
            // assume focus is dead on x axis
            double f_y = 0;

            // coordinates of body w.r.t focus
            double p_x = x-f_x;
            double p_y = y;

            // get angle b/w body position and focus
            double theta = std::atan2(p_y, p_x);
            theta = clamp_angle(theta);
            return theta;
        }

        double get_orbit_time(double pgb_mass, double orbit_radius, double G)
        {
            // from time period for perfectly periodic motion
            return 2*PI*sqrt(pow(orbit_radius, 3)/(G*pgb_mass));
        }

        CelestialBody* get_pgb(std::vector<CelestialBody>& bodies)
        {
            auto heaviest = std::max_element(
                bodies.begin(),
                bodies.end(),
                [] (const CelestialBody& a, const CelestialBody& b) {
                    return a.mass < b.mass;
                }
            );

            return &(*heaviest);
        }

        Vector3 eccentric_to_cartesian(double E, double semi_major, double eccentricity)
        {
            // cos(E) = x/a by geometry
            double x = semi_major * cos(E);

            // e = sqrt(1 - (b^2/a^2))
            double semi_minor = semi_major * sqrt(1 - eccentricity * eccentricity);
            double y = semi_minor * sin(E);
            return Vector3(x,y,0);
        }

        Vector3 calculate_gravitational_force(const CelestialBody& body, const CelestialBody& attractor)
        {
            for (auto& b: body.ignore_bodies) { if (b == &attractor) return Vector3::zero; }

            // F = GMm/(r^2)
            Vector3 r = (attractor.r-body.r);
            double r_sqrd = r.magnitude_squared();

            if (r_sqrd<1e-7) {
                return Vector3::zero;
            }

            return (r.normalized()) * (G*body.mass*attractor.mass)/r_sqrd;
        }
        // for custom G
        Vector3 calculate_gravitational_force(const CelestialBody& body, const CelestialBody& attractor, double ugc)
        {
            Vector3 r = (attractor.r-body.r);
            double r_sqrd = r.magnitude_squared();

            if (r_sqrd<1e-7) {
                return Vector3::zero;
            }

            return (r.normalized()) * (ugc*body.mass*attractor.mass)/r_sqrd;
        }
        
        Vector3 calculate_raw_acceleration(Vector3 pos, Vector3 attractor_pos, double attractor_mass)
        {
            Vector3 r = attractor_pos - pos;
            double r_sqrd = r.magnitude_squared();

            if (r_sqrd < 1e-9) {
                return Vector3::zero;
            }

            // F = ma
            // F = GMm/(r^2)
            // ma = GMm/(r^2)
            return (r.normalized ()) * (G*attractor_mass)/r_sqrd;
        }


        std::vector<Vector3> calculate_trajectory(double body_mass, Vector3 init_pos, Vector3 init_vel, std::vector<CelestialBody>& affectors, std::vector<CelestialBody*> affectors_to_ignore, double prediction_steps, double dt, bool only_affected_by_pgb)
        {
            // create ghost simulation
            CelestialBody body_ghost("ghost", BodyType::PLANET, body_mass, 1, 'o', init_pos, init_vel, false, true);
            std::vector<Vector3> trajectory;
            
            // Reserve memory: mission_duration / dt
            trajectory.reserve(static_cast<size_t>(prediction_steps / dt) + 1);

            // Find the PGB for Kepler math anchoring
            CelestialBody* pgb = get_pgb(affectors);
            Vector3 pgb_r = pgb->r; 
            double pgb_mass = pgb->mass;
            

            for (double t=0; t<prediction_steps; t+=dt)
            {   
                // move ghost using velocity-verlet integration
                body_ghost.r += body_ghost.v * 0.5 * dt;
                
                // reset total acc for frame
                Vector3 total_acc = Vector3::zero;

                for (auto& body : affectors) {
                    if (only_affected_by_pgb && &body != pgb) continue;

                    // Don't apply earth's influence (body to ignore) as currently we dont account for escape velocity or hill sphere
                    // Otherwise causes errors in calculation (plunges to sun)
                    bool ignore_this = false;
                    for (CelestialBody* affector: affectors_to_ignore)
                    {   
                        if (affector == &body) {
                             ignore_this = true;
                             break;
                        }
                    }
                    if (ignore_this) continue;

                    Vector3 current_pos = body.r;
                    if (&body != pgb && body.body_type != BodyType::MANMADE) {
                        // Calculate exactly where planets are on day t by figuring out their orbits
                        Vector3 body_pos = body.r;
                        Vector3 body_vel = body.v;
                        double b_a = Dynamics::calculate_orbit_semi_major(body_pos, body_vel, G, pgb_mass);
                        double b_e = Dynamics::calculate_orbit_eccentricity(body_pos, body_vel, G, pgb_mass);

                        Vector3 b_GC = pgb_r - Vector3(b_a * b_e, 0, 0);
                        Vector3 b_r_GC = body.r - b_GC;
                        double b_EA = get_eccentric_anomaly(b_r_GC.x, b_r_GC.y, b_a, b_e);
                        current_pos = get_future_position_theoretical(pgb_r, pgb_mass, G, t, b_a, b_e, b_EA);
                    }
                    
                    total_acc += calculate_raw_acceleration(body_ghost.r, current_pos, body.mass);
                }
                
                body_ghost.v += total_acc * dt;
                body_ghost.r += body_ghost.v * 0.5 * dt;
                
                trajectory.push_back(body_ghost.r);
            }
            return trajectory;
        }
        // Gives pure theoretical Keplerian flight path (Nominal Trajectory)
        std::vector<Vector3> get_nominal_path(Vector3 pgb_r, double pgb_mass, double a, double e, double E_initial, double time_of_flight, double dt)
        {
            std::vector<Vector3> nominal_path;
            
            // Reserve memory to prevent expensive vector reallocations
            int steps = std::ceil(time_of_flight / dt);
            nominal_path.reserve(steps);

            for (double t = 0; t < time_of_flight; t += dt) {
                Vector3 r_kep = get_future_position_theoretical(pgb_r, pgb_mass, G, t, a, e, E_initial);
                nominal_path.push_back(r_kep);
            }
            
            return nominal_path;
        }

        Vector3 calculate_encke_deviation(CelestialBody* target, CelestialBody* pgb, std::vector<Vector3>& nominal_path, double dt, const std::vector<CelestialBody>& affectors)
        {
            // encke deviations
            Vector3 dr = Vector3::zero;
            Vector3 dv = Vector3::zero;

            for (size_t i = 0; i < nominal_path.size(); i++) {
                double t = i*dt;

                // keplerian (theoretical) position
                Vector3 r_kep = nominal_path[i];
                Vector3 r_true = r_kep+dr;
                
                // raw acceleration due to sun
                Vector3 acc_kep_sun = calculate_raw_acceleration(r_kep, pgb->r, pgb->mass);
                Vector3 acc_true_sun = calculate_raw_acceleration(r_true, pgb->r, pgb->mass);
                Vector3 d_acc_sun = acc_true_sun - acc_kep_sun;
                
                // perturbing accelerations on the true position
                Vector3 acc_perturb = Vector3::zero;
                for (const auto& body: affectors) {
                    // encke deviations are literally to calculate deviations due to other bodies other than sun
                    // so skip over sun and of course the actual body itself
                    if (&body==pgb || &body == target) 
                        continue;
                    
                    // figure out target's orbit
                    Vector3 body_pos = body.r;
                    Vector3 body_vel = body.v;

                    double b_a = Dynamics::calculate_orbit_semi_major(body_pos, body_vel, G, pgb->mass);
                    double b_e = Dynamics::calculate_orbit_eccentricity(body_pos, body_vel, G, pgb->mass);

                    Vector3 b_GC = pgb->r - Vector3(b_a * b_e, 0, 0);
                    Vector3 b_r_GC = body.r - b_GC;
                    double b_EA = get_eccentric_anomaly(b_r_GC.x, b_r_GC.y, b_a, b_e);

                    // calculate and apply small perturbations
                    Vector3 body_r_at_t = get_future_position_theoretical(pgb->r, pgb->mass, G, t, b_a, b_e, b_EA);

                    Vector3 acc_on_target = calculate_raw_acceleration(r_true, body_r_at_t, body.mass);
                    Vector3 acc_on_pgb = calculate_raw_acceleration(pgb->r, body_r_at_t, body.mass);
                    
                    acc_perturb += (acc_on_target - acc_on_pgb);
                }

                // apply deviations
                Vector3 d_acc = d_acc_sun + acc_perturb;
                dv += d_acc * dt;
                dr += dv * dt;
            }

            return dr;
        }

        double calculate_specific_mechanical_energy(Vector3 orbiting_r, Vector3 orbiting_v, double G, double M)
        {
            double E = (orbiting_v.magnitude_squared()/2) - (G*M/orbiting_r.magnitude());
            return E; 
        }

        double calculate_orbit_eccentricity(Vector3 orbiting_r, Vector3 orbiting_v, double G, double M)
        {
            double mu = G * M;

            Vector3 h = Vector3::cross(orbiting_r, orbiting_v);
            Vector3 e_vector = ( (Vector3::cross(orbiting_v, h)/mu) - orbiting_r.normalized());
            double e = e_vector.magnitude();

            return e;
        }

        double calculate_orbit_semi_major(Vector3 orbiting_r, Vector3 orbiting_v, double G, double M)
        {
            double energy = calculate_specific_mechanical_energy(orbiting_r, orbiting_v, G, M);
            double mu = G*M;

            return std::abs(mu / (2*energy));
        }

        // calculate absolute future position of body with keplers equations relating time at pericentre
        Vector3 get_future_position_theoretical(Vector3 pgb_r, double pgb_mass, double G, double time, double semi_major, double eccentricity, double eccentric_anomaly)
        {
            double mu = pgb_mass * G;
            // n: mean motion
            double n = sqrt(mu/(semi_major*semi_major*semi_major));
            // M_0: mean anomaly
            double M_0 = eccentric_anomaly - eccentricity*sin(eccentric_anomaly);
            // M_f: future mean anomaly after time 
            double M_f = M_0 + n*time;
            M_f = clamp_angle(M_f);

            //newton raphson
            int iterations = 0;
            const int max_iterations = 100;
            const double accuracy_tolerance = 1e-7;

            double E_f = M_f + eccentricity * sin(M_f); 

            while (iterations < max_iterations) {
                // jist of newton raphson just to keep in mind
                // f(E) = E - e*sin(E) - M
                // f'(E) = 1 - e*cos(E)
                // E_i+1 = E_i - f(E)/f'(E)

                double dE = (E_f - eccentricity*sin(E_f) - M_f)/(1-eccentricity*cos(E_f));
                E_f -= dE;

                if (abs(dE) < accuracy_tolerance)
                    break;
                
                iterations++;
            }

            // figure out geometric center
            Vector3 r_from_GC = eccentric_to_cartesian(E_f, semi_major, eccentricity);
            Vector3 GC = pgb_r - Vector3(semi_major*eccentricity, 0, 0);

            // return absolute position
            return GC + r_from_GC;
        }

        // iteratively solve lamberts problem to figure out velocity required to intercept body with time interval t2-t1
        Vector3 solve_lambert(Vector3 ri, Vector3 rf, Vector3 pgb_r, double M, double t1, double t2, double G)
        {
            // set primary gravitational body as origin (frame of ref)
            // dirn = target - origin
            ri -= pgb_r;
            rf -= pgb_r;

            double r1 = ri.magnitude();
            double r2 = rf.magnitude();

            Vector3 normal = Vector3::cross(ri, rf);
            if (normal.magnitude() < 1e-5) {
                // Vectors are 180 degrees apart. Force the standard 2D plane.
                normal = Vector3(0, 0, 1); 
            } 
            else {
                normal = normal.normalized();
            }

            double c = (rf-ri).magnitude();

            double s = (r1+r2+c)/2;
            double mu = G*M;

            double theta = Vector3::angle_acos(ri, rf);

            //Determine direction of transfer, prograde or retrograde
            if (Vector3::cross(ri, rf).z < 0) {
                theta = 2.0*3.141592-theta;
            }

            double q = sqrt(r1 * r2)*cos(theta/2)/s;
            double del_t = t2 - t1;
            // normalized time of flight
            double T_target = sqrt(8.0*mu/(s*s*s)) * del_t;

            // use newton raphson method to estimate better roots till close to desired time with some tolerance
            // initial guess
            double x = 0.5;
            double T_guess = 0.0;
            double dT_dx = 0.0;

            int iterations = 0;
            const int max_iterations = 100;
            const double accuracy_tolerance = 1e-6;

            while (iterations < max_iterations)
            {
                double E = x*x - 1.0; 
                
                // for handling parabolic orbits where E=0.0
                if (abs(E) < 1e-7) E = (E<0) ? -1e-7 : 1e-7;
                
                // I will document more steps when I understand more of why they are exactly being done.
                // For now I am purely taking stepwise reference from Lancaster & Blanchards paper.
                double y = sqrt(abs(E));

                //protect z from floating point underflow
                double z_inner = 1.0 + q*q*E;
                z_inner = std::max(0.0, z_inner);
                double z = sqrt(z_inner);

                double f = y*(z-q*x);
                double g = x*z-q*E;

                double d = 0.0;
                if (E<0.0) {
                    d = std::atan2(f, g);
                }
                else {
                    double arg = f+g;
                    if (arg <= 0.0) arg = 1e-7;
                    d = std::log(arg);
                }

                T_guess = 2.0 * (x-q*z-d/y)/E;

                if (abs(T_guess-T_target) < accuracy_tolerance)
                    break;
                
                dT_dx = (4.0 - 4*q*q*x/z - 3.0*x*T_guess) / E;  

                double step = (T_guess-T_target) / dT_dx;
                step = std::max(-0.5, std::min(0.5, step));
                x = x-step;

                iterations++;
            }

            //recalculate E
            double E = x*x - 1;
            if (abs(E) < 1e-7) E = (E<0) ? -1e-7 : 1e-7;
            double z = sqrt(std::max(0.0, 1.0 + q*q*E));
            double y = sqrt(abs(E));

            // initial radial velocity
            double r1_dot = sqrt(2*mu*s) * (q*z*(s-r1) - x*(s-r2)) / (c*r1);
            double a = -s/(2.0*E);

            double p = 2.0*r1-(r1*r1)/a-(r1*r1_dot)*(r1*r1_dot)/mu;
            p = std::max(0.0, p);

            double u1_theta = sqrt(mu*p)/r1;

            Vector3 r1_unit = ri.normalized();
        
            // Rotate the position vector 90 degrees counter-clockwise to get the tangential direction
            Vector3 theta_unit = Vector3::cross(normal, r1_unit).normalized();

            // Combine radial and tangential velocities safely.
            // Skipped over calculating cot and cosec from the paper
            Vector3 u1 = (r1_unit * r1_dot) + (theta_unit * u1_theta);
            return u1;
        }

        // one step of simulation: apply interactive forces between heavenly bodies with time interval dt (in days) passing.
        // uses velocity-verlet integration
        void simulation_step(std::vector<CelestialBody>& celestial_bodies, double dt, bool lock_pgb)
        {
            // Half step
            for (auto&body : celestial_bodies) {
                if (body.is_kinematic)
                    continue;

                body.r += body.v * 0.5 * dt;           
                body.force = Vector3::zero;
            }

            // Apply forces among all
            for (int i = 0; i < celestial_bodies.size(); i++) {
                if (celestial_bodies[i].is_kinematic)
                    continue;
                    
                for (int j = 0; j < celestial_bodies.size(); j++) {
                    if (i==j) continue;

                    celestial_bodies[i].force += calculate_gravitational_force(celestial_bodies[i], celestial_bodies[j]);
                }
            }

            // Apply velocities and remaining half step
            for (auto& body:celestial_bodies)
            {
                if (body.is_kinematic)
                    continue;;

                Vector3 acc = body.force/body.mass;
                body.v += acc*dt;
                body.r += body.v * 0.5 * dt;
                if (body.body_type==BodyType::STAR && lock_pgb) {
                    body.r = Vector3::zero;
                    body.v = Vector3::zero;
                }
            }
        }
    }
}


