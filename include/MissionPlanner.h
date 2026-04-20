#pragma once

#include "CelestialBody.h"
#include "Vector3.h"
#include<vector>
#include<string>


namespace OrbitForge {
    enum class MissionPhase {
        ON_GROUND,
        LIFTOFF,
        LEO,
        TRANSIT,
        APPROACHING,
        REACHED,
        FAIL
    };

    struct MissionManifest {
        std::string missionID;

        std::string originBody;
        std::string targetBody;

        double targetLaunchDay;
        double targetTravelTime;

        double payloadMass;
        double phaseAngleAtLaunch;
        double injectionDeltaV;
        Dynamics::Vector3 injectionVector;

        CelestialBody* ship;
        std::string shipID;

        MissionPhase currentPhase = MissionPhase::ON_GROUND;
        double currentElapsedTime = 0.0;
        double distanceToTarget;

        double actualArrivalvelocity;
        double closestApproach;
        bool captureSuccessful = false;

        MissionManifest(std::string id, std::string ship_id, std::string orig, std::string targ, double launch_day, double travel_time, double payload_mass);
    };

    class MissionPlanner {
        public: 
            MissionPlanner::MissionPlanner(const MissionManifest& mission_data);
            bool has_launched() const { return is_launched; }
            MissionManifest manifest;
            bool is_launched = false;

            
            void update(double current_time, std::vector<CelestialBody>& bodies, double dt);
            void calculate_and_execute_launch(double current_time, std::vector<CelestialBody>& bodies, double dt);
    };
}
