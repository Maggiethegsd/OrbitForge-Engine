#pragma once 

// true solar data in SI units
const struct TrueSolarData {
    static constexpr double mass_sun = 1.989e30;

    static constexpr double mass_mercury = 3.30e23;
    static constexpr double mass_venus = 4.87e24;
    static constexpr double mass_earth = 5.97e24;
    static constexpr double mass_mars = 6.42e23;
    static constexpr double mass_jupiter = 1.90e27;
    static constexpr double mass_saturn = 5.68e26;
    static constexpr double mass_uranus = 8.68e25;
    static constexpr double mass_neptune = 1.02e26;

    static constexpr double orbit_radius_mercury = 5.8e10;
    static constexpr double orbit_radius_venus = 1.08e11;
    static constexpr double orbit_radius_earth = 1.50e11;
    static constexpr double orbit_radius_mars = 2.28e11;
    static constexpr double orbit_radius_jupiter = 7.78e11;
    static constexpr double orbit_radius_saturn = 1.43e12;
    static constexpr double orbit_radius_uranus = 2.88e12;
    static constexpr double orbit_radius_neptune = 4.50e12;

};

double to_solarmass(double mass_in_kg);
double to_au(double dist_in_m);