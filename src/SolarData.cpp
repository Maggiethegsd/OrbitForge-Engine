#include "SolarData.h"

#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <map>
#include <iomanip>
#include <iostream>

using namespace OrbitForge;

namespace OrbitForge 
{
    namespace SolarData
    {
        // Known bodies with their masses in kg
        static std::unordered_map<std::string, IntrinsicData> BodyCatalog = {
            {"Sun",     {1.99e30}},
            {"Mercury", {3.30e23}},
            {"Venus",   {4.867e24}},
            {"Earth",   {5.97e24}},
            {"Mars",    {6.42e23}},
            {"Jupiter", {1.90e27}},
            {"Saturn",  {5.68e26}},
            {"Uranus",  {8.68e25}},
            {"Neptune", {1.02e26}},
            {"Moon",    {7.35e22}}
        };

        // Known orbits of bodies around each other. Includes moons of various planets. Semi major axis in m.
        static std::map<std::pair<std::string, std::string>, OrbitalRelation> OrbitalData = {
            { {"Mercury", "Sun"}, {57909050000.0, 0.2056} },
            { {"Venus",   "Sun"}, {108208000000.0, 0.0068} },
            { {"Earth",   "Sun"}, {149597870700.0, 0.0167} }, 
            { {"Mars",    "Sun"}, {227939200000.0, 0.0934} },
            { {"Jupiter", "Sun"}, {7.78e11, 0.0484} },
            { {"Saturn",  "Sun"}, {1.43e12, 0.0541} },
            { {"Uranus",  "Sun"}, {2.88e12, 0.0463} },
            { {"Neptune", "Sun"}, {4.50e12, 0.0097} },
            
            // Geocentric Orbits
            { {"Moon",    "Earth"},  {3.84e8, 0.0549} }
        };

        double to_solarmass (double mass_in_kg) { return mass_in_kg/1.989e30; }
        double to_au (double dist_in_m) { return dist_in_m/149597870700.0; }
        double to_kg(double mass_in_sm) { return mass_in_sm * 1.989e30; }

        double get_mass_SI(std::string body_name)
        {
            auto it = BodyCatalog.find(body_name);
            if (it!=BodyCatalog.end())
                return it->second.mass_kg;

            std::cerr<<"WARNING: Body '" << body_name << "' not found in Orbital Catalog!";

            return 0.0;
        }
        
        double get_mass_SM(std::string body_name)
        {
            auto it = BodyCatalog.find(body_name);
            if (it!=BodyCatalog.end())
                return to_solarmass(it->second.mass_kg);
            
            std::cerr<<"WARNING: Body '" << body_name << "' not found in Body Catalog!";

            return 0.0;
        }

        double get_orbit_ecc(std::string body_name, std::string pgb_name)
        {
            auto it = OrbitalData.find({body_name, pgb_name});
            if (it!=OrbitalData.end())
                return it->second.eccentricity;

            std::cerr<<"WARNING: Body '" << body_name << "' not found in Orbital Catalog!";
            return 0.0;
        }

        double get_orbit_semi_major_SI(std::string body_name, std::string pgb_name)
        {
            auto it = OrbitalData.find({body_name, pgb_name});
            if (it!=OrbitalData.end())
                return it->second.semi_major_axis_m;

            std::cerr<<"WARNING: Body '" << body_name << "' not found in Orbital Catalog!";

            return 0.0;
        }

        double get_orbit_semi_major_AU(std::string body_name, std::string pgb_name)
        {
            auto it = OrbitalData.find({body_name, pgb_name});
            if (it!=OrbitalData.end())
                return to_au(it->second.semi_major_axis_m);

            std::cerr<<"WARNING: Body '" << body_name << "' not found in Orbital Catalog!";

            return 0.0;
        }
    }
}
