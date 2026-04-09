#include<vector>
#include<fstream>
#include<thread>

#include "CelestialBody.h"
#include "Simulation.h"
#include "SolarData.h"
#include "Vector3.h"


int main()
{
    // Simulation parameters
    double simulation_runtime=750.00;
    double dt=0.05;
    double t=0;

    // Mission parameters
    bool rocket_launched = false;
    int launch_day = 20;
    double payload = 1500;

    CelestialBody rocket("Artemis", payload, .5, '^', Vector3::zero, Vector3::zero, true);
    std::vector<CelestialBody> solar_system = {
        CelestialBody( "Sun", 1, 25, 'o', Vector3::zero, Vector3::zero, true), 
        CelestialBody( "Mercury", to_solarmass(TrueSolarData::mass_mercury), 1.5, 'o', Vector3(to_au(TrueSolarData::orbit_radius_mercury),0,0), Vector3(0,-get_orbit_u(1, to_au(TrueSolarData::orbit_radius_mercury), G), 0), true),
        CelestialBody( "Venus", to_solarmass(TrueSolarData::mass_venus), 2.8, 'o', Vector3(to_au(TrueSolarData::orbit_radius_venus),0,0), Vector3(0,-get_orbit_u(1, to_au(TrueSolarData::orbit_radius_venus), G), 0), true),
        CelestialBody( "Earth", to_solarmass(TrueSolarData::mass_earth), 3, 'o', Vector3(1,0,0), Vector3(0,-get_orbit_u(1, 1, G), 0), true),
        CelestialBody( "Mars", to_solarmass(TrueSolarData::mass_mars), 2, 'o', Vector3(to_au(TrueSolarData::orbit_radius_mars),0,0), Vector3(0,-get_orbit_u(1, to_au(TrueSolarData::orbit_radius_mars), G), 0), true),
        CelestialBody( "Jupiter", to_solarmass(TrueSolarData::mass_jupiter), 6, 'o', Vector3(to_au(TrueSolarData::orbit_radius_jupiter),0,0), Vector3(0,-get_orbit_u(1, to_au(TrueSolarData::orbit_radius_jupiter), G), 0), false),
        CelestialBody( "Saturn", to_solarmass(TrueSolarData::mass_saturn), 5, 'o', Vector3(to_au(TrueSolarData::orbit_radius_saturn),0,0), Vector3(0,-get_orbit_u(1, to_au(TrueSolarData::orbit_radius_saturn), G), 0), false)
        //{ "Uranus", 'o', to_solarmass(TrueSolarData::mass_uranus), 4, {to_au(TrueSolarData::orbit_radius_uranus),0,0}, {0,-get_orbit_init_velocity(1, to_au(TrueSolarData::orbit_radius_neptune), G), 0}, {0,0,0} },
        //{ "Neptune", 'o', to_solarmass(TrueSolarData::mass_neptune), 4.25, {to_au(TrueSolarData::orbit_radius_neptune),0,0}, {0,-get_orbit_init_velocity(1, to_au(TrueSolarData::orbit_radius_neptune), G), 0}, {0,0,0} }
        //{ "Asteroid 1", 's', .45, {-200,50,0}, {5,0, 0}, {0,0,0} }, 
        //{ "Asteroid 2", 's', .4, {200,-50,0}, {-10,0, 0}, {0,0,0} }, 
        //{ "Rocket", '^', 0.0085, {-100,11,0}, {2,-5, 0}, {0,0,0} }
    };

    std::ofstream solarsys_data_file ("C:/Users/lenovo/Documents/Lamberts_BVP/simulation_data/solar_system_data.csv");
    std::ofstream rocket_data_file ("C:/Users/lenovo/Documents/Lamberts_BVP/simulation_data/rocket_data.csv");

    solarsys_data_file<<"Time";
    rocket_data_file<<"Time,Rocket_X,Rocket_Y,Rocket_Z\n";

    for (auto& body:solar_system) {
        solarsys_data_file<<","<<body.name<<"_X,"<<body.name<<"_Y,"<<body.name<<"_Z,"<<body.name<<"_mass,"<<body.name<<"_radius,"<<body.name<<"_shape,"<<body.name<<"_draw";
    }
    solarsys_data_file<<"\n";

    while (t<simulation_runtime)
    {
        simulation_step(solar_system, rocket, rocket_launched, dt);
        Vector3 earth_pos = Vector3::zero;
        Vector3 earth_vel = Vector3::zero;

        std::cout<<"\nSimulated for Time " << t << " days";
        solarsys_data_file<<t;
        // update parameters in solar system data
        for (auto& body:solar_system) {
            if (body.name=="Earth")
                earth_pos = body.r;
                earth_vel = body.v;

            solarsys_data_file<<","<<body.r.x<<","<<body.r.y<<","<<body.r.z<<","<<body.mass<<","<<body.radius<<","<<body.shape<<","<<body.draw;
        }
        solarsys_data_file<<'\n';
        
        if (t >= launch_day && !rocket_launched)
        {
            rocket.r = earth_pos;
            rocket.v = earth_vel;
            rocket.v+=Vector3(0.005, 0.005,0);

            rocket_launched=true;
        }

        if (rocket_launched)
            rocket_data_file<<t<<","<<rocket.r.x<<","<<rocket.r.y<<","<<rocket.r.z<<"\n";

        t+=dt;
    }

    solarsys_data_file.close();
    rocket_data_file.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // contact python

    //std::cout<<"\n\nContacting Python..."<<std::endl;
    //system("conda run solar_system_viz_MP.py");


    return 0;
}
