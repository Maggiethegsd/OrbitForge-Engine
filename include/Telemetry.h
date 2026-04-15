#pragma once

#include<vector>
#include<unordered_map>

#include "CelestialBody.h"
#include "Vector3.h"
#include "MissionPlanner.h"

namespace OrbitForge
{
    namespace Telemetry
    {   
        extern std::ofstream system_dynamic_data;
        extern std::ofstream system_static_data;
        extern std::ofstream trajectories_data;

        void setup_telemetry_logging(std::vector<CelestialBody>& bodies);

        void log_telemetry_frame(double time, std::vector<CelestialBody>& bodies, std::unordered_map<std::string, double> periapsisTimes);

        void log_trajectory(std::vector<Dynamics::Vector3> trajectory, double start_time, double end_time, double dt);

        void close_all_files();

        void log_mission_manifest(MissionManifest manifest);
    }
}
