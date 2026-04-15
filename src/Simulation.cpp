#include<vector>
#include<fstream>
#include<thread>
#include <iomanip>
#include <unordered_map>
#include <functional>
#include<cmath>

#include "CelestialBody.h"
#include "Dynamics.h"
#include "SolarData.h"
#include "Vector3.h"
#include "Telemetry.h"
#include "EventBus.h"
#include "MissionPlanner.h"

using namespace OrbitForge;
using namespace OrbitForge::Dynamics;

namespace OrbitForge 
{
    namespace Simulation
    {
        // Simulation parameters
        // runtime in days
        static double simulation_runtime=250.00;
        // timestep in days
        static double dt=0.005;
        // starting time
        static double t=0;

        // telemetry interval. Do we need to log every single timestep?
        static double telemetry_interval = .02;
        static double next_telemetry_time = 0.0;

        static std::unordered_map<std::string, double> previousDots;
        static std::unordered_map<std::string, double> periapsisTimes;

        MissionManifest mission1("105-Day Mercury Fly-By", "Jarvis-5", "Earth", "Mercury", 50, 105, 1500);
        MissionPlanner planner_m1(mission1);

        /* @brief Helper function to create and store planets/asteroids in the simulation
        @param name Name of the planet
        @param radius Rendering radius of the planet, for matplotlib
        @param start_angle_deg Spawn angle of planet (with pericentre as base)
        @param draw Whether to actually draw it in the render
        @return The planet template
        */
        CelestialBody create_planet(std::string name, double radius, double start_angle_deg, bool draw) {
            double mass = SolarData::get_mass_SM(name);
            double a = SolarData::get_orbit_semi_major_AU(name);
            double e = SolarData::get_orbit_ecc(name);

            //convert start angle to radians
            double theta = start_angle_deg * (PI/180);

            //simple elliptical geometry transformations

            // gravitational parameter
            double mu = G*1.0;
            // latus rectum
            double p = a * (1.0 - e*e);
            double r = p/(1.0 + e*cos(theta));

            // convert from polar coordinates and spawn
            Vector3 init_pos = Vector3(r*cos(theta), r*sin(theta), 0);

            // angular momentum
            double h_p = sqrt(mu / p); 
            // required initial velocity
            Vector3 u = Vector3(-h_p * sin(theta), h_p * (e + cos(theta)), 0);

            return CelestialBody(name, BodyType::PLANET, mass, radius, 'o', init_pos, u, draw, false);
        }

        std::vector<CelestialBody> simulation_bodies;

        void init_simulation()
        {
            simulation_bodies = {
                CelestialBody("Sun", BodyType::STAR, 1, 10, 'o', Vector3::zero, Vector3::zero, true, false), 
                create_planet("Mercury", 1.5, 312.97,true),
                create_planet("Venus", 2.8, 0.0, true),
                create_planet("Earth", 3.0, 0.0, true),
                create_planet("Mars", 2.5, 67.36, true),
                create_planet("Jupiter", 6.0, 142.23, true),
                CelestialBody(mission1.shipID, BodyType::MANMADE, mission1.payloadMass, 3.5, '^', Vector3::zero, Vector3::zero, true, true)
            };
        };

        static void track_periapsis(double t)
        {
            CelestialBody* pgb;

            pgb = Dynamics::get_pgb(simulation_bodies);
            for (auto& body : simulation_bodies) {
                if (body.body_type == BodyType::STAR || body.body_type == BodyType::MANMADE) continue;

                // vector pointing from sun to body
                Vector3 r_rel = body.r - pgb->r;
                // relative velocity of body w.r.t sun
                Vector3 v_rel = body.v - pgb->v;
                double current_dot = Vector3::dot(r_rel, v_rel);

                // if we've tracked this before, check for the crossing
                if (previousDots.find(body.name) != previousDots.end()) {
                    // check if we crossed from negative to positive (Periapsis)
                    if (previousDots[body.name] < 0 && current_dot >= 0) {
                        periapsisTimes[body.name] = t;
                        std::cout << body.name << " reached Periapsis at T+" << t << "\n";
                    }
                } 

                else {
                    periapsisTimes[body.name] = 0.0;
                }   

                previousDots[body.name] = current_dot;
            }
        }
    }
}


int main()
{
    using namespace OrbitForge::Simulation;
    init_simulation();

    Telemetry::setup_telemetry_logging(simulation_bodies);

    for (auto& body: simulation_bodies) {
        if (body.name==mission1.shipID)
            planner_m1.manifest.ship = &body;
    }

    if (planner_m1.manifest.ship == nullptr) {
        std::cerr<<"FATAL: Ship pointer for " << mission1.missionID << " missing.";
        return 1;
    }
    
    std::cout<<"\nSimulating for " << simulation_runtime << " days";

    bool is_first_tick = true;
    while (t<simulation_runtime)
    {  
        if (std::fmod(std::floor(t), 500) == 0)
            std::cout<<"\nSimulating for day " << t << std::endl;

        // simulation tick
        Dynamics::simulation_step(simulation_bodies, dt, false);

        planner_m1.update(t, simulation_bodies, dt);

        track_periapsis(t);
        // update parameters in solar system data
        for (auto& body:simulation_bodies) {
            body.first_tick_complete = true;
        }

        if (t >= next_telemetry_time) {
            Telemetry::log_telemetry_frame(t, simulation_bodies, periapsisTimes);
            next_telemetry_time += telemetry_interval;
        }

        is_first_tick = false;
        t+=dt;
    }
    
    std::cout<<"\nSimulation complete for " << simulation_runtime << " days.";

    // close all data file to avoid leak
    Telemetry::close_all_files();

    return 0;
}
