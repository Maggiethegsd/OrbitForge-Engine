#include <cmath>

#include "Vector3.h"

Vector3 Vector3::zero = {0,0,0};
Vector3::Vector3(): x(0), y(0), z(0) {}
Vector3::Vector3(double x1, double x2, double x3) : x(x1), y(x2), z(x3) {}

bool Vector3::operator==(const Vector3& rhs) const 
{
    const double tolerance = 1e-9;
    return (abs(x-rhs.x)<tolerance && abs(y-rhs.y)<tolerance && abs(z-rhs.z)<tolerance);
}

double Vector3::magnitude() const 
{
    return sqrt(x*x + y*y + z*z);
}
double Vector3::magnitude_squared() const 
{
    return (x*x + y*y + z*z);
}

Vector3 Vector3::normalized() const
{
    return { x/magnitude(), y/magnitude(), z/magnitude()};
}

Vector3 cross (Vector3 v1, Vector3 v2)
{
    return Vector3((v1.y*v2.z) - (v1.z*v2.y),
                    (v1.z*v2.x) - (v1.x*v2.z),
                    (v1.x*v2.y) - (v1.y*v2.x));
}

double dot (Vector3 v1, Vector3 v2)
{
    return (v1.x*v2.x, v1.y*v2.y, v1.z*v2.z);
}


double angle(Vector3 v1, Vector3 v2)
{
    return acos(dot(v1, v2)/(v1.magnitude() * v2.magnitude()));
}


std::ostream& operator<<(std::ostream& os, const Vector3& vec) 
{
    os << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]";
    return os;
}

Vector3 operator*(double scalar, const Vector3& rhs)
{
    return rhs*scalar;
}

