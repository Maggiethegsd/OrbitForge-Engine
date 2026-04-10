#pragma once 

#include <string>
#include <vector>

namespace SolarData
{
    struct TrueSolarBody {
        std::string name;
        double body_mass;
        double orbit_radius;
        double orbit_ecc;

        TrueSolarBody(std::string _name, double mass, double r, double e);
    };


    double get_orbit_semi_major_SI(std::string body_name);
    double get_mass_SI(std::string body_name);

    double get_orbit_ecc(std::string body_name);
    double get_orbit_semi_major_AU(std::string body_name);
    double get_mass_SM(std::string body_name);

    double to_solarmass(double mass_in_kg);
    double to_au(double dist_in_m);
}