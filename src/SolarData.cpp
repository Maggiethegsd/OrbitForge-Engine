#include "SolarData.h"

double to_solarmass (double mass_in_kg)
{
    return mass_in_kg/1.989e30;
}

double to_au (double dist_in_m)
{
    return dist_in_m/149597870700;
}
