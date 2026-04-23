#pragma once 

#include <string>
#include <vector>
#include<map>
#include<unordered_map>

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

      /* @brief Fetch value of semi-major axis of body in SI units .
         @param name Name of celestial body
         @return Semi-major axis of body in SI units (m).
      */
      double get_orbit_semi_major_SI(std::string body_name);

      /* @brief Fetch value of mass of body's orbit in SI units .
         @param name Name of celestial body
         @return Semi-major axis of body in SI units (m).
      */
      double get_mass_SI(std::string body_name);

      /* @brief Fetch value of mass of body in SI units.
         @param name Name of celestial body
         @return Mass of body in SI units (kg).
      */
      double get_orbit_ecc(std::string body_name, std::string pgb_name); 

      /* @brief Fetch value of semi major axis of body's orbit in AU.
         @param name Name of celestial body
         @return Angular velocity (degrees/day) of body's orbit
      */
      double get_orbit_semi_major_AU(std::string body_name, std::string pgb_name);

      /* @brief Fetch value of mass of body in SM.
         @param name Name of celestial body
         @return Mass of body in SI units (kg).
      */
      double get_mass_SM(std::string body_name);

      /* @brief Convert given mass in kg to Solar mass.
      */
      double to_solarmass(double mass_in_kg);

      /* @brief Convert given mass in Solar mass to kg.
      */
      double to_kg(double mass_in_sm);
      
      /* @brief Convert given length in m to Astronomical units.
      */
      double to_au(double dist_in_m);
   }
}