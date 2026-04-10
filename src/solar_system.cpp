#include<vector>
#include<fstream>
#include<thread>

#include "CelestialBody.h"
#include "Simulation.h"
#include "SolarData.h"
#include "Vector3.h"

using namespace SolarData;

CelestialBody create_planet(std::string name, double radius, double start_angle_deg, bool draw) {
    double mass = SolarData::get_mass_SM(name);
    double a = SolarData::get_orbit_semi_major_AU(name);
    double e = SolarData::get_orbit_ecc(name);

    //convert start angle to radians
    double theta = start_angle_deg * (3.141592/180);

    double mu = G*1.0;
    double p = a * (1.0 - e*e);
    double r = p/(1.0 + e*cos(theta));

    // convert from polar coordinates and spawn
    Vector3 init_pos = Vector3(r*cos(theta), r*sin(theta), 0);

    double h_p = sqrt(mu / p); // angular momentum term
    Vector3 u = Vector3(-h_p * sin(theta), h_p * (e + cos(theta)), 0);

    return CelestialBody(name, BodyType::PLANET, mass, radius, 'o', init_pos, u, draw);
}

int main()
{
    // Simulation parameters
    double simulation_runtime=500.00;
    double dt=0.005;
    double t=0;

    // Mission parameters
    bool rocket_launched = false;
    int launch_day = 50;
    double mission_duration = 259;
    double payload = 1500;
    payload = to_solarmass(payload);

    CelestialBody rocket("Artemis", BodyType::MANMADE, payload, .5, '^', Vector3::zero, Vector3::zero, true);

        std::vector<CelestialBody> solar_system = {
        CelestialBody("Sun", BodyType::STAR, 1, 10, 'o', Vector3::zero, Vector3::zero, true), 
        create_planet("Mercury", 1.5, 0.0,true),
        create_planet("Venus", 2.8, 0.0, true),
        create_planet("Earth", 3.0, 0.0, true),
        create_planet("Mars", 2.5, 69.0, true),
        create_planet("Jupiter", 6.0, 120.0, false)
    };

    std::ofstream solarsys_data_file ("C:/Users/lenovo/Documents/Lamberts_BVP/simulation_data/solar_system_data.csv");
    std::ofstream rocket_data_file ("C:/Users/lenovo/Documents/Lamberts_BVP/simulation_data/rocket_data.csv");
    std::ofstream rocket_traj_file ("C:/Users/lenovo/Documents/Lamberts_BVP/simulation_data/rocket_traj_data.csv");

    solarsys_data_file<<"Time";
    rocket_data_file<<"Time,Rocket_X,Rocket_Y,Rocket_Z\n";
    rocket_traj_file<<"Time,Traj_X,Traj_Y,Traj_Z\n";

    for (auto& body:solar_system) {
        if (body.body_type!=BodyType::STAR)
            solarsys_data_file<<","<<body.name<<"_X,"<<body.name<<"_Y,"<<body.name<<"_Z,"<<body.name<<"_XfromE,"<<body.name<<"_YfromE,"<<body.name<<"_ZfromE,"<<body.name<<"_true_anomaly,"<<body.name<<"_eccentric_anomaly,"<<body.name<<"_time_of_periapsis,"<<body.name<<"_mass,"<<body.name<<"_radius,"<<body.name<<"_shape,"<<body.name<<"_draw";if (body.body_type!=BodyType::STAR);
            
        else
            solarsys_data_file<<","<<body.name<<"_X,"<<body.name<<"_Y,"<<body.name<<"_Z,"<<body.name<<"_mass,"<<body.name<<"_radius,"<<body.name<<"_shape,"<<body.name<<"_draw";
    }
    solarsys_data_file<<"\n";
    
    std::cout<<"\nSimulating for " << simulation_runtime << " days";

    bool is_first_tick = true;
    while (t<simulation_runtime)
    {
        simulation_step(solar_system, rocket, rocket_launched, dt, false);

        Vector3 earth_pos = Vector3::zero;
        Vector3 earth_vel = Vector3::zero;

        Vector3 sun_pos = Vector3::zero;
        double sun_mass = 0;
        Vector3 sun_velocity=Vector3::zero;

        Vector3 mars_pos = Vector3::zero;


        solarsys_data_file<<t;
        // update parameters in solar system data
        for (auto& body:solar_system) {
            // set position and mass of sun
            if (body.name=="Sun") {
                sun_pos = body.r;
                sun_mass = body.mass;
                sun_velocity = body.v;
            }
            
            // set position and velocity of earth
            if (body.name=="Earth") {
                earth_pos = body.r;
                earth_vel = body.v;
            }

            if (body.name=="Mars")
                mars_pos = body.r;

            body.orbit_eccentricity=SolarData::get_orbit_ecc(body.name);

            // get geometric center (GC) of body-sun elliptical orbit (sun is at focus)
            double body_a = SolarData::get_orbit_semi_major_AU(body.name);
            double body_e = SolarData::get_orbit_ecc(body.name);
            Vector3 GC = sun_pos - Vector3(body_a*body_e, 0, 0);
            
            // get relative position of planet w.r.t GC of planet-sun orbit
            Vector3 r_rel = body.r - GC;
            
            // true anomaly is calculated from focus, with planet coordinates relative to GC
            double true_anomaly = get_true_anomaly(r_rel.x, r_rel.y, body_a, body_e);
            // eccentric anomaly is calculated from geometric center, with planet coordinates relative to GC
            double E = get_eccentric_anomaly(r_rel.x, r_rel.y, body_a, body_e);
            
            // convert eccentric anomaly of current orbit to cartesian coordinates (relative to GC)
            Vector3 r_from_Ecc = eccentric_to_cartesian(E, body_a, body_e);

            // r(E/GC) = r(E) - r(GC)
            // r(E) = r(E/GC) + r(GC)
            // convert to absolute coords
            r_from_Ecc += GC;

            // check if periapsis
            // distance of periapsis = a(1-e)

            //std::cout<<"Dot b/w Velocity and Outward Vector for " << body.name << ": " << cur_v_dot << "\n";
             //track periapsis time
            if (body.first_tick_complete)
            {
                if (body.previous_true_anomaly > 6 && true_anomaly < 1.0)
                {
                    std::cout<<"\n\nPeriapsis recorded for " << body.name << " on day " << t << " with e="<<body.orbit_eccentricity;
                    body.periapsis_time = t;
                }
            }
            
            body.previous_true_anomaly = true_anomaly;

            if (body.body_type != BodyType::STAR)
                solarsys_data_file<<","<<body.r.x<<","<<body.r.y<<","<<body.r.z<<","<<r_from_Ecc.x<<','<<r_from_Ecc.y<<','<<r_from_Ecc.z<<','<<true_anomaly<<','<<E<< ','<<body.periapsis_time << ',' << body.mass<<","<<body.radius<<","<<body.shape<<","<<body.draw;
            else
                solarsys_data_file<<","<<body.r.x<<","<<body.r.y<<","<<body.r.z<<","<<body.mass<<","<<body.radius<<","<<body.shape<<","<<body.draw;

            body.first_tick_complete = true;
        }
        solarsys_data_file<<'\n';
        
        if (t >= launch_day && !rocket_launched)
        {
            double mars_e = SolarData::get_orbit_ecc("Mars");
            double mars_a = SolarData::get_orbit_semi_major_AU("Mars");

            Vector3 GC_mars_orbit = sun_pos - Vector3(mars_e*mars_a, 0, 0);

            // position of mars relative to geometric center of the orbit, as required by eccentric anomaly calculator  
            Vector3 mars_r_GC = mars_pos - GC_mars_orbit;

            double mars_EA = get_eccentric_anomaly(mars_r_GC.x, mars_r_GC.y, mars_a, mars_e);

            std::vector <Vector3> mars_nominal_path = get_nominal_path(sun_pos, sun_mass, mars_a, mars_e, mars_EA, mission_duration, dt);
            //apply encke deviation
            Vector3 dr2 = calculate_encke_deviation("Mars", mars_nominal_path, dt, solar_system, sun_pos, sun_mass);
            Vector3 r2 = mars_nominal_path.back();

            r2 += dr2;
            
            Vector3 u = solve_lambert(earth_pos, r2, sun_pos, sun_mass, t, t+mission_duration, G);
            
            rocket.r = earth_pos;
            rocket.v = u+sun_velocity;

            std::vector<Vector3> rocket_trajectory = calculate_trajectory(payload, rocket.r, rocket.v, solar_system, mission_duration, dt, true);

            double traj_t = t;
            for (auto& path:rocket_trajectory) {
                rocket_traj_file << traj_t << ',' << path.x << ',' << path.y << ',' << path.z << '\n';
                traj_t+=dt;
            }   

            rocket_launched=true;
        }

        if (!rocket_launched)
            rocket_data_file<<t<<","<<earth_pos.x<<","<<earth_pos.y<<","<<earth_pos.z<<"\n";
        if (rocket_launched)
            rocket_data_file<<t<<","<<rocket.r.x<<","<<rocket.r.y<<","<<rocket.r.z<<"\n";

        is_first_tick = false;
        t+=dt;
    }
    
    std::cout<<"\nSimulation complete for " << simulation_runtime << " days.";

    solarsys_data_file.close();
    rocket_data_file.close();
    rocket_traj_file.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // contact python

    //std::cout<<"\n\nContacting Python..."<<std::endl;
    //system("conda run solar_system_viz_MP.py");


    return 0;
}
