#include "SolarData.h"

#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include<map>

using namespace OrbitForge;

namespace OrbitForge 
{
    namespace SolarData
    {
        struct IntrinsicData {
            double mass_kg;
        };

        struct OrbitalRelation {
            double semi_major_axis_m;
            double eccentricity;
        };

        static std::unordered_map<std::string, IntrinsicData> BodyCatalog = {
            {"Sun", {1.989e+30}},
            {"Mercury", {3.30e23}},
            {"Earth", {5.97e24}},
            {"Mars", {6.42e23}},
            {"Jupiter", {1.90e27}},
            {"Saturn",  {5.68e269}},
            {"Uranus",  {8.68e25}},
            {"Neptune", {1.02e26}}
        };

        static std::map<std::pair<std::string, std::string>, OrbitalRelation> OrbitalData = {
            { {"Mercury", "Sun"}, {57909050000.0, 0.2056} },
            { {"Venus", "Sun"}, {108208000000.0, 0.0068} },
            { {"Earth", "Sun"}, {149597870700.0, 0.0167} }, 
            { {"Mars", "Sun"}, {227939200000.0, 0.0934} },
            { {"Jupiter", "Sun"}, {7.78e11, 0.0484} },
            { {"Saturn", "Sun"}, {1.43e12, 0.0541} },
            { {"Uranus", "Sun"}, {2.88e12, 0.0463} },
            { {"Neptune", "Sun"}, {4.50e12, 0.0097} },
            
            // Geocentric Orbits
            { {"Moon", "Earth"},  {384400000.0, 0.0549, 2.661e-06}}
        };


        double to_solarmass (double mass_in_kg)
        {
            return mass_in_kg/1.989e30;
        }

        double to_au (double dist_in_m)
        {
            return dist_in_m/149597870700.0;
        }

        double to_kg(double mass_in_sm)
        {
            return mass_in_sm * 1.989e30;
        }

        double get_mass_SI(std::string body_name)
        {
            double m = 1;
            auto it = BodyCatalog.find(body_name);

            if (it!=BodyCatalog.end()) {
                return it->second.mass_kg;
            }

            return m;
        }
        
        double get_mass_SM(std::string body_name)
        {
            double m = 1;
            auto it = BodyCatalog.find(body_name);

            if (it!=BodyCatalog.end()) {
                return to_solarmass(it->second.mass_kg);
            }

            return m;
        }

        double get_orbit_ecc(std::string body_name, std::string pgb_name)
        {
            double e = 0.0;
            if (pgb_name=="Sun") {
                for (auto&body:TrueSolarBodies)
                    if (body.name==body_name)
                        e = body.orbit_ecc;
            }

            else if (pgb_name=="Earth") {
                if (body_name=="Moon")
                    e = 0.0549;
            }

            return e;
        }

        double get_orbit_semi_major_SI(std::string body_name, std::string pgb_name)
        {
            double a = 0.0;
            if (pgb_name=="Sun")
                for (const auto&body:TrueSolarBodies)   
                    if (body.name==body_name)
                        a = body.orbit_radius;
            else if (pgb_name=="Earth") {
                if (body_name=="Moon")
                    a = 384400000;
            }

            return a;
        }

        double get_orbit_semi_major_AU(std::string body_name, std::string pgb_name)
        {
            double a = 0.0;
            for (auto& body : TrueSolarBodies)   
                if (body.name==body_name)
                    a = to_au(body.orbit_radius);
            
            else if (pgb_name=="Earth") {
                if (body_name=="Moon")
                    a = 0.00257;
            }


            return a;
        }
    }
}
