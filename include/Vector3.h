#pragma once

#include <iostream>

namespace OrbitForge 
{
    namespace Dynamics
    {
        struct Vector3{
            double x;
            double y;
            double z;
                
            // default constructor and custom constructor
            Vector3();
            Vector3 (double x1, double x2, double x3);
            static Vector3 zero;

            Vector3 operator+(const Vector3& rhs) const { return {x+rhs.x, y+rhs.y, z+rhs.z}; }
            Vector3 operator+=(const Vector3& rhs) { x+=rhs.x; y+=rhs.y; z+=rhs.z; return {x,y,z}; }
            Vector3 operator-=(const Vector3& rhs) { x-=rhs.x; y-=rhs.y; z-=rhs.z; return {x,y,z}; }
            Vector3 operator-(const Vector3& rhs) const { return {x-rhs.x, y-rhs.y, z-rhs.z}; }
            Vector3 operator*(double scalar) const { return {x*scalar, y*scalar, z*scalar}; }
            Vector3 operator/(double scalar) const { return {x/scalar, y/scalar, z/scalar}; }

            friend std::ostream& operator<<(std::ostream& os, const Vector3& vec);

            bool operator==(const Vector3& rhs) const;

            double magnitude() const;
            double magnitude_squared() const;
            Vector3 normalized() const;

            static double dot(Vector3 v1, Vector3 v2);
            static Vector3 cross(Vector3 v1, Vector3 v2);
            static double angle_acos(Vector3 v1, Vector3 v2);
            static double angle(Vector3 v1, Vector3 v2);
            static double distance(Vector3 v1, Vector3 v2);
        };

        // global operator for scalar * vector
        Vector3 operator*(double scalar, const Vector3& rhs);
        double rad2deg(double angle_in_rad);
        double deg2rad(double angle_in_deg);
    }
}