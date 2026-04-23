#include "Vector3.h"
#include "MissionPlanner.h"
#include "CelestialBody.h"
#include "Dynamics.h"
#include "Telemetry.h"
#include "Simulation.h"
#include "SolarData.h"

#include <string>
#include <stdexcept>
#include <fstream>

using namespace OrbitForge;
using namespace OrbitForge::Dynamics;

namespace OrbitForge { 
        MissionPlanner::MissionPlanner(const MissionManifest& mission_data) : manifest(mission_data), is_launched(false) {  }

        MissionManifest::MissionManifest(std::string id, std::string ship_id, std::string orig, std::string targ, double launch_day, double travel_time, double payload_mass) :
        missionID(id), shipID(ship_id), originBody(orig), targetBody(targ), targetLaunchDay(launch_day), payloadMass(payload_mass), targetTravelTime(travel_time), ship(nullptr) 
        { payloadMass=SolarData::to_solarmass(payload_mass); }

    void MissionPlanner::update(double current_time, std::vector<CelestialBody>& bodies, double dt)
    {
        CelestialBody* pgb_ptr = nullptr;
        CelestialBody* origin_ptr = nullptr;
        CelestialBody* target_ptr = nullptr;

        // assign pointers
        for (auto& body : bodies) {
            if (body.body_type == BodyType::STAR) pgb_ptr = &body;
            else if (body.name == manifest.originBody) origin_ptr = &body;
            else if (body.name == manifest.targetBody) target_ptr = &body;
        }

        // rocket on ground of origin body before launch day
        if (current_time < manifest.targetLaunchDay) {
            manifest.ship->r=origin_ptr->r;
            manifest.ship->v=origin_ptr->v;
        }

        // launch day!
        if (current_time >= manifest.targetLaunchDay && !is_launched && manifest.currentPhase != MissionPhase::FAIL)
        {
            std::cout << "\n[Mission Planner] Launching mission " << manifest.missionID << " on day " << (int)current_time << ".\n";
            manifest.currentPhase = MissionPhase::LIFTOFF;

            try { calculate_and_execute_launch(current_time, bodies, dt); }
            catch(std::exception e) { std::cout<<std::endl<<e.what(); }
        }

        // rocket in transit and we haven't reached.
        if (is_launched && manifest.currentPhase != MissionPhase::REACHED) {
            manifest.currentElapsedTime+=dt;
            manifest.distanceToTarget=Vector3::distance(manifest.ship->r, target_ptr->r);
        }

        if (manifest.currentPhase == MissionPhase::TRANSIT) {
            // if we have completed 90% of the way there
            if (manifest.distanceToTarget <= 0.1*Vector3::distance(origin_ptr->r, target_ptr->r))
                manifest.currentPhase = MissionPhase::APPROACHING;
        }
    }

    void MissionPlanner::calculate_and_execute_launch(double current_time, std::vector<CelestialBody>& bodies, double dt)
    {
        std::cout << "---- Calculating launch parameters for Mission " << manifest.missionID << " ----";
        
        CelestialBody* pgb = nullptr;
        CelestialBody* origin = nullptr;
        CelestialBody* target = nullptr;
        CelestialBody* ship = nullptr;

        // assign pointers
        for (auto& body : bodies) {
            if (body.body_type==BodyType::STAR) pgb = &body;
            else if (body.name == manifest.originBody) origin = &body;
            else if (body.name == manifest.targetBody) target = &body;
            else if (body.name == manifest.shipID) ship = &body;
        }
        manifest.ship = ship;
        manifest.ship->ignore_bodies.push_back(origin);

        Vector3 target_pos = target->r;
        Vector3 target_vel = target->v;

        Vector3 origin_pos = origin->r;
        Vector3 origin_vel = origin->v;

        Vector3 pgb_pos = pgb->r;
        Vector3 pgb_vel = pgb->v;
        double pgb_mass = pgb->mass;

        // get current ephemeris of target
        Vector3 target_rel_pos = target_pos - pgb_pos;
        Vector3 target_rel_vel = target_vel - pgb_vel;


        double target_e = Dynamics::calculate_orbit_eccentricity(target_rel_pos, target_rel_vel, G, pgb_mass);
        double target_a = Dynamics::calculate_orbit_semi_major(target_rel_pos, target_rel_vel, G, pgb_mass);

        // geometric center of target's orbit around pgb 
        Vector3 GC_target_orbit = pgb_pos - Vector3(target_e*target_a, 0, 0);

        // position of target relative to geometric center of its orbit, as required by eccentric anomaly calculator  
        Vector3 target_r_GC = target_pos - GC_target_orbit;

        double target_EA = Dynamics::get_eccentric_anomaly(target_r_GC.x, target_r_GC.y, target_a, target_e);

        std::vector <Vector3> target_nominal_path = Dynamics::get_nominal_path(pgb_pos, pgb_mass, target_a, target_e, target_EA, manifest.targetTravelTime, dt);
        if (target_nominal_path.empty())
            std::cout<<"Nominal path calculation came out empty!";

        Vector3 launch_offset(SolarData::to_au(6571000.0), 0.0, 0.0); // Earth radius
        manifest.ship->r = origin_pos+launch_offset;

        //apply encke deviation to target path
        // this is the true final position of the target, calculated by computer algorithms
        Vector3 r2 = target_nominal_path.back();

        
        Vector3 dr2 = Dynamics::calculate_encke_deviation(target, pgb, target_nominal_path, dt, bodies);
        r2 += dr2;

        std::cout << "\nOrigin: " << manifest.originBody;
        std::cout << "\nTarget: " << manifest.targetBody;
        std::cout << "\nLaunch Day: " << manifest.targetLaunchDay;
        std::cout << "\nPayload Mass: " << SolarData::to_kg(manifest.payloadMass);
        std::cout << "\nOrigin Pos: " << origin_pos;
        std::cout << "\nPGB mass: " << pgb_mass;


        // solve lamberts problem for launch
        Vector3 u = Dynamics::solve_lambert(ship->r, r2, pgb_pos, pgb_mass, current_time, current_time+manifest.targetTravelTime, G);

        if (std::isnan(u.x) || std::isnan(u.y)) {
            std::cerr << "\nLambert solver failed.";
            manifest.currentPhase = MissionPhase::FAIL;
            return;
        }

        Vector3 v_guess = u+pgb_vel;

        // newton raphson to get pinpoint accuracy value
        double delta = 1e-4;
        for (int iter=0; iter<4; iter++) {
            std::cout<<"\nRefining Launch Velocity, iteration: " << iter+1 << "...\n";

            // trajectory we are going to follow with the initial velocity we just calculated with lambert
            std::vector<Vector3> base_traj = Dynamics::calculate_trajectory(manifest.payloadMass, ship->r, v_guess, bodies, {ship, origin}, manifest.targetTravelTime, dt, false);
            if (base_traj.empty()) {
                std::cerr << "Fatal Error: Ghost trajectory aborted.\n";
                manifest.currentPhase = MissionPhase::FAIL;
                break; // Break the loop safely instead of crashing Visual Studio
            }
            
            Vector3 r_base = base_traj.back();
            
            // error between actual position of mars on arrival and position we are going to reach
            Vector3 error = r2 - r_base;

            // nudge individual components by delta amount
            Vector3 r_dx = Dynamics::calculate_trajectory(manifest.payloadMass, ship->r, v_guess + Vector3(delta, 0, 0), bodies, {ship, origin}, manifest.targetTravelTime, dt, false).back();
            Vector3 r_dy = Dynamics::calculate_trajectory(manifest.payloadMass, ship->r, v_guess + Vector3(0, delta, 0), bodies, {ship, origin}, manifest.targetTravelTime, dt, false).back();

            // get jacobian of base position we are going to reach and nudge we made
            // so get change relative of coordinates
            double Jxx = (r_dx.x - r_base.x)/delta;
            double Jyx = (r_dx.y - r_base.y)/delta;
            double Jxy = (r_dy.x - r_base.x)/delta;
            double Jyy = (r_dy.y - r_base.y)/delta;
            
            // adjust dvx and dvy
            double det = (Jxx*Jyy) - (Jxy*Jyx);
            if (std::abs(det) < 1e-12) {
                std::cerr << "Error: Jacobian determinant is zero. Solver failed." << std::endl;
                manifest.currentPhase = MissionPhase::FAIL;
                break; 
            }
            double dvx = (Jyy*error.x - Jxy * error.y)/det;
            double dvy = (Jxx*error.y - Jyx * error.x)/det;

            // add those adjustments to our guess velocity
            // loop over, take the new guess velocity and make it more accurate
            // so on till we are satisfied (with given tolerance)
            v_guess.x+=dvx;
            v_guess.y+=dvy;
        }
        
        std::cout<<"\nCalculated launch velocity: " << v_guess;
        // launch from earth duh
        // inhibit earth's velocity ofc
        manifest.ship->is_kinematic=false;

        manifest.ship->v = v_guess;
        manifest.currentPhase = MissionPhase::TRANSIT;
        manifest.injectionDeltaV = (manifest.ship->v - origin_vel).magnitude();
        manifest.phaseAngleAtLaunch = Dynamics::rad2deg(Vector3::angle(origin_pos, target_pos));

        // calculate ghost trajectory with perfectly adjusted initial v
        std::vector<Vector3> rocket_trajectory = Dynamics::calculate_trajectory(manifest.payloadMass, manifest.ship->r, manifest.ship->v, bodies, {origin}, manifest.targetTravelTime, dt, true);
        Telemetry::log_trajectory(rocket_trajectory, current_time, Simulation::simulation_runtime, dt);
        Telemetry::log_mission_manifest(manifest);

        is_launched=true;
    }
}   
