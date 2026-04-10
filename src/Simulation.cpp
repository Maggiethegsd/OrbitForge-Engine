    #pragma once

    #include<cmath>
    #include <vector>

    #include "SolarData.h"
    #include "Simulation.h"
    #include "CelestialBody.h"
    #include "Vector3.h"

    //pin-point accuracy
    const double G = 0.0002959122082855911;;
    const double PI  =3.1415926;

    using namespace SolarData;


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
    

    // calculate raw acceleration on body caused by attractor
    Vector3 calculate_raw_acceleration(Vector3 pos, Vector3 attractor_pos, double attractor_mass)
    {
        Vector3 r = attractor_pos - pos;
        double r_sqrd = r.magnitude_squared();

        if (r_sqrd < 1e-9) {
            return Vector3::zero;
        }

        return (r.normalized ()) * (G*attractor_mass)/r_sqrd;
    }


    // calculate future trajectory of body with given mass, initial position, velocity and list of all affectors in the universe
    std::vector<Vector3> calculate_trajectory(double body_mass, Vector3 init_pos, Vector3 init_vel, std::vector<CelestialBody> affectors, double prediction_steps, double dt, bool only_affected_by_pgb)
    {
        CelestialBody body_ghost("ghost", BodyType::PLANET, body_mass, 1, 'o', init_pos, init_vel, false);
        std::vector<Vector3> trajectory;

        for (double t=0; t<prediction_steps; t+=dt)
        {   
            body_ghost.r += body_ghost.v * 0.5 * dt;

            body_ghost.force = Vector3::zero;
            for (auto& body : affectors) {
                if (only_affected_by_pgb && body.name!="Sun") 
                    continue;
                body_ghost.force += calculate_gravitational_force(body_ghost, body);
            }
            
            Vector3 acc = body_ghost.force / body_mass;
            body_ghost.v += acc * dt;
            
            body_ghost.r += body_ghost.v * 0.5 * dt;
            trajectory.push_back(body_ghost.r);
        }
        return trajectory;
    }
        // Generates a pure theoretical Keplerian flight path (The Nominal Trajectory)
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

    Vector3 calculate_encke_deviation(std::string target_name, std::vector<Vector3>& nominal_path, double dt, const std::vector<CelestialBody>& affectors, Vector3 pgb_r, double pgb_mass)
    {
        // encke deviations
        Vector3 dr = Vector3::zero;
        Vector3 dv = Vector3::zero;

        for (size_t i = 0; i < nominal_path.size(); i++) {
            double t = i*dt;

            Vector3 r_kep = nominal_path[i];
            Vector3 r_true = r_kep+dr;
            
            Vector3 acc_kep_sun = calculate_raw_acceleration(r_kep, pgb_r, pgb_mass);
            Vector3 acc_true_sun = calculate_raw_acceleration(r_true, pgb_r, pgb_mass);
            Vector3 d_acc_sun = acc_true_sun - acc_kep_sun;
            
            // perturbing accelerations on the true position
            Vector3 acc_perturb = Vector3::zero;
            for (const auto& body: affectors) {
                if (body.name=="Sun" || body.name == target_name) 
                    continue;

                double b_a = SolarData::get_orbit_semi_major_AU(body.name);
                double b_e = SolarData::get_orbit_ecc(body.name);

                Vector3 b_GC = pgb_r - Vector3(b_a * b_e, 0, 0);
                Vector3 b_r_GC = body.r - b_GC;
                double b_EA = get_eccentric_anomaly(b_r_GC.x, b_r_GC.y, b_a, b_e);

                Vector3 body_r_at_t = get_future_position_theoretical(pgb_r, pgb_mass, G, t, b_a, b_e, b_EA);
                
                acc_perturb += calculate_raw_acceleration(r_true, body_r_at_t, body.mass);
            }

            Vector3 d_acc = d_acc_sun + acc_perturb;
            dv += d_acc * dt;
            dr += dv * dt;
        }

        return dr;
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
            // f(E) = E - e*sin(E) - M
            // f'(E) = 1 - e*cos(E)
            // E_i+1 = E_i - f(E)/f'(E)

            double dE = (E_f - eccentricity*sin(E_f) - M_f)/(1-eccentricity*cos(E_f));
            E_f -= dE;

            if (abs(dE) < accuracy_tolerance)
                break;
            
            iterations++;
        }

        Vector3 r_from_GC = eccentric_to_cartesian(E_f, semi_major, eccentricity);
        Vector3 GC = pgb_r - Vector3(semi_major*eccentricity, 0, 0);

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
        const double accuracy_tolerance = 1e-6;

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

            if (abs(T_guess-T_target) < accuracy_tolerance)
                break;
            
            dT_dx = (4.0 - 4*q*q*x/z - 3.0*x*T_guess) / E;  
            x = x - (T_guess - T_target) / dT_dx;

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
        Vector3 theta_unit = Vector3(-r1_unit.y, r1_unit.x, 0);

        // Combine radial and tangential velocities safely! No trigonometry required.
        Vector3 u1 = (r1_unit * r1_dot) + (theta_unit * u1_theta);
        return u1;
    }

    // one step of simulation: apply interactive forces between heavenly bodies with time interval dt (in days) passing.
    // uses velocity-verlet integration
    void simulation_step(std::vector<CelestialBody>& celestial_bodies, CelestialBody& rocket, bool rocket_launched, double dt, bool lock_pgb)
    {
        for (auto&body : celestial_bodies) body.r += body.v * 0.5 * dt;
        if (rocket_launched) rocket.r += rocket.v * 0.5 * dt;
        
        for (auto&body: celestial_bodies) body.force = Vector3::zero;
        if (rocket_launched) rocket.force = Vector3::zero;

        for (int i = 0; i < celestial_bodies.size(); i++) {
            for (int j = 0; j < celestial_bodies.size(); j++) {
                if (i==j) continue;
                celestial_bodies[i].force += calculate_gravitational_force(celestial_bodies[i], celestial_bodies[j]);
            }
            
            if (rocket_launched && celestial_bodies[i].name=="Sun")
                    rocket.force+=calculate_gravitational_force(rocket, celestial_bodies[i]);
        }

        for (auto& body:celestial_bodies)
        {
            Vector3 acc = body.force/body.mass;
            body.v += acc*dt;
        }

        if (rocket_launched) {
            Vector3 acc = rocket.force/rocket.mass;
            rocket.v += acc * dt;
        }


        for (auto& body:celestial_bodies)
        {
            body.r += body.v * 0.5 * dt;
            if (body.name=="Sun" && lock_pgb) {
                    body.r = Vector3::zero;
                    body.v = Vector3::zero;
            }
        }

        if (rocket_launched) rocket.r += rocket.v * 0.5 * dt;
    }

