#include <cmath>
#include <iostream>

#include "Vector3.h"
#include "Dynamics.h"

using namespace OrbitForge;

namespace OrbitForge {
    namespace Dynamics
    {
        Vector3 Vector3::zero = {0,0,0};
        Vector3::Vector3(): x(0), y(0), z(0) {}
        Vector3::Vector3(double x1, double x2, double x3) : x(x1), y(x2), z(x3) {}

        bool Vector3::operator==(const Vector3& rhs) const 
        {
            const double tolerance = 1e-9;
            return (std::abs(x-rhs.x)<tolerance && std::abs(y-rhs.y)<tolerance && std::abs(z-rhs.z)<tolerance);
        }

        double Vector3::magnitude() const 
        {
            return sqrt(x*x + y*y + z*z);
        }

        double Vector3::magnitude_squared() const 
        {
            return (x*x + y*y + z*z);
        }

        // normalized vector: preserves the direction but has unit magnitude
        Vector3 Vector3::normalized() const
        {
            return { x/magnitude(), y/magnitude(), z/magnitude()};
        }

        // cross product of two vectors by determinant form
        Vector3 Vector3::cross (Vector3 v1, Vector3 v2)
        {
            return Vector3((v1.y*v2.z) - (v1.z*v2.y),
                            (v1.z*v2.x) - (v1.x*v2.z),
                            (v1.x*v2.y) - (v1.y*v2.x));
        }

        // dot product of two vectors

        double Vector3::dot (Vector3 v1, Vector3 v2)
        {
            return ( (v1.x*v2.x) + 
                    (v1.y*v2.y) + 
                    (v1.z*v2.z) );
        }

        // angle between two vectors in radians, v2 is the 'base' vector
        double Vector3::angle(Vector3 v1, Vector3 v2)
        {
            if (v1==Vector3::zero || v2 == Vector3::zero)
                return 0;

            double theta = std::atan2(cross(v2, v1).z,  dot(v1, v2));
            if (theta < 0)
                theta += 2.0 * Dynamics::PI;

            return theta;
        }
        // angle between two vectors in radians, v2 is the 'base' vector. Uses dot product method.
        double Vector3::angle_acos(Vector3 v1, Vector3 v2)
        {
            if (v1==Vector3::zero || v2 == Vector3::zero) return 0;
            double arg = dot(v1, v2) / (v1.magnitude() * v2.magnitude());
            if (arg < -1.0) arg = -1.0;
            else if (arg > 1.0) arg = 1.0;

            double theta = acos( arg );
            return theta;
        }

        double Vector3::distance(Vector3 v1, Vector3 v2)
        {
            return (v1-v2).magnitude();
        }


        double rad2deg(double angle_in_rad)
        {
            return (angle_in_rad/3.141592)*180;
        }

        double deg2rad(double angle_in_deg)
        {
            return (angle_in_deg/180)*3.141592;
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
    }
}

