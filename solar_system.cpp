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
    double x;
    double y;
    double z;

    
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

struct CelestialBody {
    // position and velocity
    std::string name;
    char shape;
    double mass;
    double radius;
    Vector3 r;
    Vector3 v;
    Vector3 force;
};

const double G = 2.95e-6;

// dist(matplotlib units)/unit_AU = dist in AU
double unit_AU=1;
// t(s)/unit_days = days passed in simulation
double unit_days=100;

//simulation parameters
double simulation_runtime=2500.00;
//simulation timestep
double dt=0.33;


Vector3 calculate_gravitational_force(const CelestialBody& body, const CelestialBody& attractor)
{
    Vector3 r = (attractor.r-body.r);
    double r_sqrd = r.magnitude_squared();

    if (r_sqrd<1e-9) {
        return {0,0,0};
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

// initial velocity for perfect circular orbit around given primary gravitational body and orbit radius
double get_orbit_init_velocity(double pgb_mass, double orbit_radius, double G)
{
    return sqrt(G*pgb_mass/orbit_radius);
}
// time period for perfect circular orbit around given primary gravitational body and orbit radius
double get_orbit_time(double pgb_mass, double orbit_radius, double G)
{
    return 2*3.141592*sqrt(pow(orbit_radius, 3)/(G*pgb_mass));
}

int main()
{
    double t=0;

    std::vector<CelestialBody> solar_system = {
        { "Sun", 'o', 100, 10, {0,0,0}, { 0, 0, 0 }, {0,0,0} }, 
        { "Earth", 'o', 3e-3, 1, {1,0,0}, {0,-get_orbit_init_velocity(100, 1, G), 0}, {0,0,0} }
        //{ "Mars", 'o', 3.5, 4, {152,0,0}, {0,-get_orbit_init_velocity(3.5, 152, G)*2, 0}, {0,0,0} }, 
        //{ "Asteroid 1", 's', .45, {-200,50,0}, {5,0, 0}, {0,0,0} }, 
        //{ "Asteroid 2", 's', .4, {200,-50,0}, {-10,0, 0}, {0,0,0} }, 
        //{ "Rocket", '^', 0.0085, {-100,11,0}, {2,-5, 0}, {0,0,0} }
    };


    unit_days = get_orbit_time(100, unit_AU, G)/364.25;

    std::ofstream solarsys_data_file ("solar_system_data.csv");
    std::ofstream sim_data_file ("simulation_parameters.csv");

    sim_data_file<<"runtime,timestep,astrounit_scale,dayunit_scale\n";
    sim_data_file<<simulation_runtime<<','<<dt<<','<<unit_AU<<','<<unit_days;
    

    solarsys_data_file<<"Time";
    for (auto& body:solar_system) {
        solarsys_data_file<<","<<body.name<<"_X,"<<body.name<<"_Y,"<<body.name<<"_Z,"<<body.name<<"_mass,"<<body.name<<"_radius,"<<body.name<<"_shape";
    }
    solarsys_data_file<<"\n";

    while (t<simulation_runtime)
    {
        simulation_step(solar_system, dt);
        std::cout<<"----------------\nTime: " << t << "s";
        solarsys_data_file<<t;
        // update parameters in solar system data
        for (auto& body:solar_system) {
            solarsys_data_file<<","<<body.r.x<<","<<body.r.y<<","<<body.r.z<<","<<body.mass<<","<<body.radius<<","<<body.shape;
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
