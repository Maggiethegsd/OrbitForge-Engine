    #pragma once

    #include<cmath>
    #include <vector>

    #include "SolarData.h"
    #include "Simulation.h"
    #include "CelestialBody.h"
    #include "Vector3.h"

   const double G = 0.0002959122082855911;;
    const double PI  =3.1415926;


    // initial velocity for perfect circular orbit around given primary gravitational body and orbit radius
    double get_orbit_u(double pgb_mass, double orbit_radius, double G)
    {
        return sqrt(G*pgb_mass/orbit_radius);
    }
    // initial velocity for elliptical orbit around given primary gravitational body and orbit radius
    // orbit radius is for distance from sun at periapsis (semi major axis)
    double get_elliptical_orbit_u(double pgb_mass, double semi_major, double G, double eccentricity)
    {
        //eccentricity correction if out of bounds
        if (eccentricity <= 0) eccentricity = 0;
        if (eccentricity >= 1.0) eccentricity = 0.999;

        // vis viva equation
        return sqrt( (G*pgb_mass/semi_major) * (1 + eccentricity)/(1-eccentricity));
    }

    // converts any angle in radians to between 0-2pi
    double clamp_angle(double theta)
    {
        theta = fmod(theta, 2*PI);
        
        if (theta < 0)
            theta += 2*PI;
            
        return theta;
    }

    // x and y are relative to geometric center of the ellipse
    double get_eccentric_anomaly(double x, double y, double semi_major, double eccentricity)
    {
        double b = sqrt(1 - eccentricity*eccentricity);
        double y_ = y/b;

        double E = std::atan2(y_, x);
        E = clamp_angle(E);
        return E;
    }

    // x and y are relative to geometric center of the ellipse, NOT absolute
    double get_true_anomaly(double x, double y, double semi_major, double eccentricity)
    {
        double f_x = eccentricity*semi_major;
        double f_y = 0;

        double p_x = x-f_x;
        double p_y = y;

        double theta = std::atan2(p_y, p_x);
        theta = clamp_angle(theta);
        return theta;
    }


    // time period (days) for perfect circular orbit around given primary gravitational body and orbit radius
    double get_orbit_time(double pgb_mass, double orbit_radius, double G)
    {
        return 2*PI*sqrt(pow(orbit_radius, 3)/(G*pgb_mass));
    }

    // converts eccentric anomaly to cartesian coordinates (relative to geometric center of the ellipse, make sure to subtract the geometric center to get absolute coords)
    Vector3 eccentric_to_cartesian(double E, double semi_major, double eccentricity)
    {
        double x = semi_major * cos(E);

        double semi_minor = semi_major * sqrt(1 - eccentricity * eccentricity);
        double y = semi_minor * sin(E);
        return Vector3(x,y,0);
    }
    // newtons law of gravitation
    Vector3 calculate_gravitational_force(const CelestialBody& body, const CelestialBody& attractor)
    {
        Vector3 r = (attractor.r-body.r);
        double r_sqrd = r.magnitude_squared();

        if (r_sqrd<1e-9) {
            return Vector3::zero;
        }

        return (r.normalized()) * (G*body.mass*attractor.mass)/r_sqrd;
    }

    // calculate future trajectory of body with given mass, initial position, velocity and list of all affectors in the universe
    std::vector <Vector3> calculate_trajectory(double body_mass, Vector3 init_pos, Vector3 init_vel, std::vector<CelestialBody> affectors, double prediction_steps, double dt)
    {
        // create ghost object for body
        CelestialBody body_ghost("ghost", BodyType::PLANET, body_mass, 1, 'o', init_pos, init_vel, false);
        std::vector <Vector3> trajectory;
        for (double t=0; t<prediction_steps; t+=dt)
        {   
            // set ghost force to zero for current frame
            body_ghost.force = Vector3::zero;

            // set all affectors force to zero
            for (auto&body:affectors)
                body.force=Vector3::zero;

            // apply forces between all bodies
            for (int i = 0; i < affectors.size(); i++) {
                for (int j = 0; j < affectors.size(); j++) {
                    if (i==j)
                        continue;
                    affectors[i].force += calculate_gravitational_force(affectors[i], affectors[j]);
                }

                body_ghost.force += calculate_gravitational_force(body_ghost, affectors[i]);
            }


            // apply velocity and acceleration to all bodies
            for (auto& body:affectors)
            {
                Vector3 acc = body.force/body.mass;
                body.v += acc*dt;
                body.r += body.v*dt;
            }

            // apply velocity and acceleration to ghost
            Vector3 a = body_ghost.force/body_mass;
            body_ghost.v += a*dt;
            body_ghost.r += body_ghost.v*dt;

            // add ghost path dr to trajectory data
            trajectory.push_back(body_ghost.r);
        }

        return trajectory;
    }

    // calculate approximate future position of body with keplers equations relating time at pericentre
    Vector3 get_future_position(CelestialBody body, Vector3 pgb_r, double time, double time_pericentre, double semi_major, double eccentricity)
    {
        double n = sqrt(body.mass/(semi_major*semi_major*semi_major));

        return Vector3::zero;
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
        //initial guess
        double x = 0.5;
        double T_guess = 0.0;
        double dT_dx = 0.0;

        int iterations = 0;
        const int max_iterations = 100;
        const double tolerance = 1e-6;

        while (iterations < max_iterations)
        {
            double E = x*x - 1.0; 
            
            // for handling parabolic orbits where E=0.0
            if (abs(E) < 1e-7) E = (E<0) ? -1e-7 : 1e-7;

            double y = sqrt(abs(E));
            double z = sqrt(1.0 + q*q*E);

            double f = y*(z-q*x);
            double g = x*z-q*E;

            double d = 0.0;
            if (E<0.0) {
                d = std::atan2(f, g);
            }
            else {
                d = std::log(f+g);
            }

            T_guess = 2.0 * (x-q*z-d/y)/E;

            if (abs(T_guess-T_target) < tolerance)
                break;
            
            dT_dx = (4.0 - 4*q*q*x/z - 3.0*x*T_guess) / E;  
            x = x - (T_guess - T_target) / dT_dx;

            iterations++;
        }

        //recalculate E
        double E = x*x - 1;
        if (abs(E) < 1e-7) E = (E<0) ? -1e-7 : 1e-7;
        double z = sqrt(1.0 + q*q*E);
        double y = sqrt(abs(E));

        // initial radial velocity
        double r1_dot = sqrt(2*mu*s) * (q*z*(s-r1) - x*(s-r2)) / c*r1;
        double a = -s/2.0*E;

        double p = 2.0*r1-(r1*r1)/a-(r1*r1_dot)*(r1*r1_dot)/mu;
        double u1_theta = sqrt(mu*p)/r1;

        Vector3 r1_unit = ri.normalized();
        Vector3 r2_unit = rf.normalized();

        double cot_theta = 1.0/tan(theta);
        double csc_theta = 1.0/sin(theta);

        Vector3 u1 = r1_unit * (r1_dot - u1_theta*cot_theta) + r2_unit*(u1_theta*csc_theta);
        return u1;
    }

    // one step of simulation: apply interactive forces between heavenly bodies with time interval dt (in days) passing.
    void simulation_step(std::vector<CelestialBody>& celestial_bodies, CelestialBody& rocket, bool rocket_launched, double dt)
    {
        for (auto&body : celestial_bodies) {
            body.force=Vector3::zero;
        }

        if (rocket_launched) rocket.force=Vector3::zero;

        for (int i = 0; i < celestial_bodies.size(); i++) {
            for (int j = 0; j < celestial_bodies.size(); j++) {
                if (i==j) continue;
                celestial_bodies[i].force += calculate_gravitational_force(celestial_bodies[i], celestial_bodies[j]);
            }
            
            if (rocket_launched)
                rocket.force+=calculate_gravitational_force(rocket, celestial_bodies[i]);
        }

        for (auto& body:celestial_bodies)
        {
            Vector3 acc = body.force/body.mass;
            body.v += acc*dt;
            body.r += body.v*dt;
        }

        if (rocket_launched)
        {
            Vector3 acc = rocket.force/rocket.mass;
            rocket.v+=acc*dt;
            rocket.r+=rocket.v*dt;
        }
    }
