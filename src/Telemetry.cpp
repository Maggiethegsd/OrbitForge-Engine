#include "Telemetry.h"
#include "Dynamics.h"
#include "Vector3.h"
#include "MissionPlanner.h"
#include "SolarData.h"

#include<iomanip>
#include<fstream>
#include <unordered_map>
#include<string>
#include<vector>

using namespace OrbitForge;
using namespace OrbitForge::Dynamics;

namespace OrbitForge
{
    namespace Telemetry
    {
        std::ofstream system_dynamic_data;
        std::ofstream system_static_data;
        std::ofstream trajectories_data;

        void setup_telemetry_logging(std::vector<CelestialBody>& bodies)
        {
            #pragma region CSV Setup
            std::ios_base::sync_with_stdio(false);
            
            // start writing to CSV files
            system_dynamic_data.open("C:/Users/lenovo/Documents/OrbitForge-Engine/simulation_data/simulation_dynamic_data.csv");
            system_static_data.open("C:/Users/lenovo/Documents/OrbitForge-Engine/simulation_data/simulation_static_data.csv");
            trajectories_data.open("C:/Users/lenovo/Documents/OrbitForge-Engine/simulation_data/rocket_traj_data.csv");

            system_dynamic_data << std::fixed << std::setprecision(7);
            trajectories_data << std::fixed << std::setprecision(6);

            // create base headers
            system_dynamic_data<<"Time";
            trajectories_data<<"Time,Traj_X,Traj_Y,Traj_Z\n";
            
            system_static_data << "body_name,body_mass,body_radius,body_shape,body_draw\n";

            // create headers for all bodies
            for (auto& body:bodies) {
                system_static_data << body.name << ',' << body.mass << ',' << body.radius << ',' << body.shape << ',' << body.draw << '\n';

                if (body.body_type!=BodyType::STAR && body.body_type!=BodyType::MANMADE) {
                    system_dynamic_data<<","
                    <<body.name<<"_X,"
                    <<body.name<<"_Y,"
                    <<body.name<<"_Z,"
                    <<body.name<<"_XfromE,"
                    <<body.name<<"_YfromE,"
                    <<body.name<<"_ZfromE,"
                    <<body.name<<"_true_anomaly,"
                    <<body.name<<"_eccentric_anomaly,"
                    <<body.name<<"_time_of_periapsis";
                }
                    
                else {
                    system_dynamic_data<<","
                    <<body.name<<"_X,"
                    <<body.name<<"_Y,"
                    <<body.name<<"_Z";
                }
            }
            system_dynamic_data<<'\n';
            system_static_data.close();

            #pragma endregion
        }

        
        void log_telemetry_frame(double time, std::vector<CelestialBody>& bodies, std::unordered_map<std::string, double> periapsisTimes)
        {
            system_dynamic_data << time;
            // find the primary attractor
            CelestialBody* pgb = nullptr;
            for (auto& body: bodies)
            {
                if (body.body_type == BodyType::STAR) {
                    pgb = &body;
                    break;
                }
            }

            // loop over all bodies, track their ephemeris data
            for (auto& body: bodies)
            {
                // stars and manmade only need x y z coords tracked, planets and asteroids need a lot more...
                system_dynamic_data
                <<","<<body.r.x
                <<","<<body.r.y
                <<","<<body.r.z;

                if (body.body_type == BodyType::STAR || body.body_type == BodyType::MANMADE)
                    continue;

                double e = Dynamics::calculate_orbit_eccentricity(body.r, body.v, G, pgb->mass);
                double a = Dynamics::calculate_orbit_semi_major(body.r, body.v, G, pgb->mass);
                double E = Dynamics::get_eccentric_anomaly(body.r.x, body.r.y, a, e);

                Vector3 r_from_Ecc = Dynamics::eccentric_to_cartesian(E, a, e);
                double true_anom = Dynamics::get_true_anomaly(body.r.x, body.r.y, a, e);
                double ecc_anom = Dynamics::get_eccentric_anomaly(body.r.x, body.r.y, a, e);

                Vector3 r_rel = body.r - pgb->r;
                Vector3 v_rel = body.v - pgb->v;
                
                system_dynamic_data
                <<","<<r_from_Ecc.x
                <<','<<r_from_Ecc.y
                <<','<<r_from_Ecc.z
                <<','<<true_anom
                <<','<<ecc_anom
                <<','<<periapsisTimes[body.name];
            }
            system_dynamic_data<<'\n';
        }

        void log_trajectory(std::vector<Vector3> trajectory, double start_time, double end_time, double dt)
        {
            double traj_time = start_time;
            for (auto& traj: trajectory)
            {
                trajectories_data << traj_time << ','  << traj.x << ',' << traj.y << ',' << traj.z << '\n';
                traj_time += dt;
            }
        }

        void log_mission_manifest(MissionManifest manifest)
        {
            std::ofstream manifest_file("C:/Users/lenovo/Documents/OrbitForge-Engine/simulation_data/" + manifest.missionID + "_manifest.csv");
            
            manifest_file << "Mission_ID,Origin,Target,Launch_Day,Mission_Duration,Payload_Mass,Launch_Phase_Angle,Injection_Delta_V,Ship_ID\n";
            manifest_file << manifest.missionID<<","<<
                             manifest.originBody<<","<<
                             manifest.targetBody<<","<<
                             manifest.targetLaunchDay<<","<<
                             manifest.targetTravelTime<<","<<
                             SolarData::to_kg(manifest.payloadMass)<<","<<
                             manifest.phaseAngleAtLaunch<<","<<
                             manifest.injectionDeltaV<<","<<
                             manifest.shipID;
        }
        
        void close_all_files()
        {
            system_dynamic_data.close();
            system_static_data.close();
            trajectories_data.close();
        }
    }
}
    