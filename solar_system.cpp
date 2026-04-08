#include <cmath>
#include <iostream>
#include<iomanip>
#include<vector>
#include<fstream>
#include<string>
#include<thread>
#include<cmath>

#include<cstdlib>

struct Vector3{
    public:
        double x;
        double y;
        double z;
        
        Vector3(){
            x=0;
            y=0;
            z=0;
        }

        Vector3 (double x1, double x2, double x3)
        {
            x = x1;
            y = x2;
            z = x3;
        }

    static Vector3 zero;

    Vector3 operator+(const Vector3& rhs) const {
        return {x+rhs.x, y+rhs.y, z+rhs.z};
    }

    Vector3 operator+=(const Vector3& rhs) {
        x+=rhs.x;
        y+=rhs.y;
        z+=rhs.z;

        return {x,y,z};
    }
    Vector3 operator-=(const Vector3& rhs) {
        x-=rhs.x;
        y-=rhs.y;
        z-=rhs.z;

        return {x,y,z};
    }

    Vector3 operator-(const Vector3& rhs) const {
        return {x-rhs.x, y-rhs.y, z-rhs.z};
    }

    Vector3 operator*(double scalar) const {
        return {x*scalar, y*scalar, z*scalar};
    }
    
    Vector3 operator/(double scalar) const {
        return {x/scalar, y/scalar, z/scalar};
    }

    friend std::ostream& operator<<(std::ostream& os, const Vector3& vec) {
        os << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]";
        return os;
    }

    bool operator==(const Vector3& rhs) const {
        const double tolerance = 1e-9;
        return (abs(x-rhs.x)<tolerance && abs(y-rhs.y)<tolerance && abs(z-rhs.z)<tolerance);
    }

    double magnitude() const {
        return sqrt((x*x)+(y*y)+(z*z));
    }

    double magnitude_squared() const {
        return (x*x + y*y + z*z);
    }

    Vector3 normalized() const {
        double mag = magnitude();
        return {x/mag, y/mag, z/mag};
    }
};
Vector3 Vector3::zero={0,0,0};

// global operator for scalar * vector
Vector3 operator*(double scalar, const Vector3& rhs)
{
    return rhs*scalar;
}

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

struct CelestialBody {
    public:
    // position and velocity
    std::string name;
    char shape;
    double mass;
    double radius;
    Vector3 r;
    Vector3 v;
    Vector3 force;
    bool draw;
        CelestialBody(std::string body_name, double body_mass, double body_radius, char body_shape, Vector3 init_position, Vector3 init_velocity, bool draw_body) {
            name = body_name;
            mass = body_mass;
            radius = body_radius;
            shape = body_shape;
            r = init_position;
            v = init_velocity;
            force = Vector3::zero;
            draw = draw_body;
        }
};

const double G = 2.95e-4;

//simulation parameters
// total number of days to simulate
double simulation_runtime=2500.00;
// simulation timestep (days)
double dt=0.1;

double to_solarmass(double mass_in_kg)
{
    return mass_in_kg/1.989e30;
}

double to_au(double dist_in_m)
{
    return dist_in_m/149597870700;
}


Vector3 calculate_gravitational_force(const CelestialBody& body, const CelestialBody& attractor)
{
    Vector3 r = (attractor.r-body.r);
    double r_sqrd = r.magnitude_squared();

    if (r_sqrd<1e-9) {
        return Vector3::zero;
    }

    return (r.normalized()) * (G*body.mass*attractor.mass)/r_sqrd;
}

void simulation_step(std::vector<CelestialBody>& celestial_bodies, double dt)
{
    for (auto&body : celestial_bodies)
    {
        body.force=Vector3::zero;
    }

    for (int i = 0; i < celestial_bodies.size(); i++)
    {
        for (int j = 0; j < celestial_bodies.size(); j++)
        {
            if (i==j)
                continue;
            
            celestial_bodies[i].force += calculate_gravitational_force(celestial_bodies[i], celestial_bodies[j]);
            //std::cout<<"\nForce Applied: "<< calculate_gravitational_force(celestial_bodies[i], celestial_bodies[j]);
        }
    }

    for (auto& body:celestial_bodies)
    {
        Vector3 acc = body.force/body.mass;
        body.v += acc*dt;
        body.r += body.v*dt;
    }
}

CelestialBody launch_rocket(std::string rocket_name, double rocket_mass, double launch_day, Vector3 launch_pos, Vector3 launch_velocity)
{
    return CelestialBody(rocket_name, rocket_mass, 2, '^', launch_pos, launch_velocity, true);
}

// initial velocity for perfect circular orbit around given primary gravitational body and orbit radius
double get_orbit_u(double pgb_mass, double orbit_radius, double G)
{
    return sqrt(G*pgb_mass/orbit_radius);
}
// time period (days) for perfect circular orbit around given primary gravitational body and orbit radius
double get_orbit_time(double pgb_mass, double orbit_radius, double G)
{
    return 2*3.141592*sqrt(pow(orbit_radius, 3)/(G*pgb_mass));
}

int main()
{
    double t=0;

    std::vector<CelestialBody> solar_system = {
        CelestialBody( "Sun", 1, 25, 'o', Vector3::zero, Vector3::zero, true), 
        CelestialBody( "Mercury", to_solarmass(TrueSolarData::mass_mercury), 1.5, 'o', Vector3(to_au(TrueSolarData::orbit_radius_mercury),0,0), Vector3(0,-get_orbit_u(1, to_au(TrueSolarData::orbit_radius_mercury), G), 0), true),
        CelestialBody( "Venus", to_solarmass(TrueSolarData::mass_venus), 2.8, 'o', Vector3(to_au(TrueSolarData::orbit_radius_venus),0,0), Vector3(0,-get_orbit_u(1, to_au(TrueSolarData::orbit_radius_venus), G), 0), true),
        CelestialBody( "Earth", to_solarmass(TrueSolarData::mass_earth), 3, 'o', Vector3(1,0,0), Vector3(0,-get_orbit_u(1, 1, G), 0), true),
        CelestialBody( "Mars", to_solarmass(TrueSolarData::mass_mars), 2, 'o', Vector3(to_au(TrueSolarData::orbit_radius_mars),0,0), Vector3(0,-get_orbit_u(1, to_au(TrueSolarData::orbit_radius_mars), G), 0), true),
        CelestialBody( "Jupiter", to_solarmass(TrueSolarData::mass_jupiter), 6, 'o', Vector3(to_au(TrueSolarData::orbit_radius_jupiter),0,0), Vector3(0,-get_orbit_u(1, to_au(TrueSolarData::orbit_radius_jupiter), G), 0), false),
        CelestialBody( "Saturn", to_solarmass(TrueSolarData::mass_saturn), 5, 'o', Vector3(to_au(TrueSolarData::orbit_radius_saturn),0,0), Vector3(0,-get_orbit_u(1, to_au(TrueSolarData::orbit_radius_saturn), G), 0), false)
        //{ "Uranus", 'o', to_solarmass(TrueSolarData::mass_uranus), 4, {to_au(TrueSolarData::orbit_radius_uranus),0,0}, {0,-get_orbit_init_velocity(1, to_au(TrueSolarData::orbit_radius_neptune), G), 0}, {0,0,0} },
        //{ "Neptune", 'o', to_solarmass(TrueSolarData::mass_neptune), 4.25, {to_au(TrueSolarData::orbit_radius_neptune),0,0}, {0,-get_orbit_init_velocity(1, to_au(TrueSolarData::orbit_radius_neptune), G), 0}, {0,0,0} }
        //{ "Asteroid 1", 's', .45, {-200,50,0}, {5,0, 0}, {0,0,0} }, 
        //{ "Asteroid 2", 's', .4, {200,-50,0}, {-10,0, 0}, {0,0,0} }, 
        //{ "Rocket", '^', 0.0085, {-100,11,0}, {2,-5, 0}, {0,0,0} }
    };

    std::ofstream solarsys_data_file ("solar_system_data.csv");

    solarsys_data_file<<"Time";
    for (auto& body:solar_system) {
        solarsys_data_file<<","<<body.name<<"_X,"<<body.name<<"_Y,"<<body.name<<"_Z,"<<body.name<<"_mass,"<<body.name<<"_radius,"<<body.name<<"_shape,"<<body.name<<"_draw";
    }
    solarsys_data_file<<"\n";

    while (t<simulation_runtime)
    {
        simulation_step(solar_system, dt);
        std::cout<<"----------------\nTime: " << t << "s";
        solarsys_data_file<<t;
        // update parameters in solar system data
        for (auto& body:solar_system) {
            solarsys_data_file<<","<<body.r.x<<","<<body.r.y<<","<<body.r.z<<","<<body.mass<<","<<body.radius<<","<<body.shape<<","<<body.draw;
        }
        solarsys_data_file<<'\n';

        t+=dt;
    }

    solarsys_data_file.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // contact python
    
    //std::cout<<"\n\nContacting Python..."<<std::endl;
    //int result = system("python solar_system_viz_MP.py");

    return 0;
}
