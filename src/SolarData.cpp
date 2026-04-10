#include "SolarData.h"

#include <string>
#include <algorithm>
#include <vector>

using namespace SolarData;

namespace SolarData {
    TrueSolarBody::TrueSolarBody(std::string _name, double m, double r, double e) 
    : name(_name), body_mass(m), orbit_radius(r), orbit_ecc(e) {}; 
    std::vector<TrueSolarBody> TrueSolarBodies = {
    {"Mercury", 3.30e23, 57909050000.0, 0.2056},
    {"Venus",   4.87e24, 108208000000.0, 0.0068},
    // Set Earth to exactly 1 AU in meters
    {"Earth",   5.97e24, 149597870700.0, 0.0167}, 
    {"Mars",    6.42e23, 227939200000.0, 0.0934},
    {"Jupiter", 1.90e27, 7.78e11, 0.0484},
    {"Saturn",  5.68e26, 1.43e12, 0.0541},
    {"Uranus",  8.68e25, 2.88e12, 0.0463},
    {"Neptune", 1.02e26, 4.50e12, 0.0097},
};

    double to_solarmass (double mass_in_kg)
    {
        return mass_in_kg/1.989e30;
    }

    double to_au (double dist_in_m)
    {
        return dist_in_m/149597870700.0;
    }

    double get_mass_SI(std::string body_name)
    {
        double m = 1;
        for (auto&body:TrueSolarBodies)
            if (body.name==body_name)
                m = body.body_mass;

        return m;
    }

    double get_orbit_ecc(std::string body_name)
    {
        double e = 0.0;
        for (auto&body:TrueSolarBodies)
            if (body.name==body_name)
                e = body.orbit_ecc;

        return e;
    }

    double get_orbit_semi_major_SI(std::string body_name)
    {
        double a = 0.0;
        for (const auto&body:TrueSolarBodies)   
            if (body.name==body_name)
                a = body.orbit_radius;

        return a;
    }

    double get_orbit_semi_major_AU(std::string body_name)
    {
        double a = 0.0;
        for (auto& body : TrueSolarBodies)   
            if (body.name==body_name)
                a = to_au(body.orbit_radius);

        return a;
    }

    double get_mass_SM(std::string body_name)
    {
        double m = 0.0;
        for (auto& body:TrueSolarBodies)
            if (body.name==body_name)
                m= to_solarmass(body.body_mass);

        return m;
    }
}