#pragma once 

#include <string>
#include <vector>

namespace SolarData
{
    /* @brief Data holder for actual celestial objects in the real universe.
       @param name Name of celestial body
       @param mass Mass of celestial body
       @param orbit_radius Semi-major axis of its orbit around the sun
       @param orbit_ecc Eccentricity of its orbit around the sun
    
    */
    struct TrueSolarBody {
        std::string name;
        double body_mass;
        double orbit_radius;
        double orbit_ecc;

        TrueSolarBody(std::string _name, double mass, double r, double e);
    };

    /* @brief Fetch value of semi-major axis of body in SI units .
       @param name Name of celestial body
       @return Semi-major axis of body in SI units (m).
    */
    double get_orbit_semi_major_SI(std::string body_name);
     /* @brief Fetch value of semi-major axis of body's orbit in SI units .
       @param name Name of celestial body
       @return Semi-major axis of body in SI units (m).
    */
    double get_mass_SI(std::string body_name);
    /* @brief Fetch value of mass of body in SI units.
       @param name Name of celestial body
       @return Mass of body in SI units (kg).
    */
    double get_orbit_ecc(std::string body_name); 
    /* @brief Fetch value of eccentricity of body's orbit.
       @param name Name of celestial body
       @return Eccentricity of body's orbit.
    */
    double get_orbit_semi_major_AU(std::string body_name);
     /* @brief Fetch value of mass of body in SI units .
       @param name Name of celestial body
       @return Mass of body in SI units (kg).
    */
    double get_mass_SM(std::string body_name);

     /* @brief Convert given mass in kg to Solar mass.
    */
    double to_solarmass(double mass_in_kg);
     
     /* @brief Convert given length in m to Astronomical units.
    */
    double to_au(double dist_in_m);
}