#pragma once
#include <Instances/Instance.h>

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

inline Vec3 makeVec3FromVertex(const Vertex& v) {
    return Vec3{ v.pos.x, v.pos.y, v.pos.z };
}

inline Vec3 add(const Vec3& a, const Vec3& b) { return Vec3{ a.x + b.x, a.y + b.y, a.z + b.z }; }
inline Vec3 sub(const Vec3& a, const Vec3& b) { return Vec3{ a.x - b.x, a.y - b.y, a.z - b.z }; }
inline Vec3 mul(const Vec3& a, const Vec3& b) { return Vec3{ a.x * b.x, a.y * b.y, a.z * b.z }; }
inline Vec3 mul(const Vec3& a, float s) { return Vec3{ a.x * s, a.y * s, a.z * s }; }
inline float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

class Colission {
public:
    bool AABBOverlap(const Instance& a, const Instance& b);
    bool CheckColission(const Instance& inst, const Instance& inst2, Vec3& outNormal);
private:
};