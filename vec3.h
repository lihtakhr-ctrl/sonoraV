#pragma once

struct vec3_t {
    float x;
    float y;
    float z;

    vec3_t() : x(0.f), y(0.f), z(0.f) {}
    vec3_t(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    vec3_t& operator=(const vec3_t& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
            z = other.z;
        }
        return *this;
    }

    vec3_t operator+(const vec3_t& other) const {
        return vec3_t(x + other.x, y + other.y, z + other.z);
    }

    vec3_t operator-(const vec3_t& other) const {
        return vec3_t(x - other.x, y - other.y, z - other.z);
    }

    vec3_t operator*(const vec3_t& other) const {
        return vec3_t(x * other.x, y * other.y, z * other.z);
    }

    vec3_t operator/(const vec3_t& other) const {
        return vec3_t(x / other.x, y / other.y, z / other.z);
    }

    vec3_t& operator+=(const vec3_t& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    vec3_t& operator-=(const vec3_t& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    vec3_t& operator*=(const vec3_t& other) {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    vec3_t& operator/=(const vec3_t& other) {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    vec3_t operator*(float scalar) const {
        return vec3_t(x * scalar, y * scalar, z * scalar);
    }

    vec3_t operator/(float scalar) const {
        return vec3_t(x / scalar, y / scalar, z / scalar);
    }

    vec3_t& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    vec3_t& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    vec3_t operator+() const {
        return *this;
    }

    vec3_t operator-() const {
        return vec3_t(-x, -y, -z);
    }

    bool operator==(const vec3_t& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const vec3_t& other) const {
        return !(*this == other);
    }

    float& operator[](int index) {
        switch (index) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default: throw std::out_of_range("Index out of range");
        }
    }

    const float& operator[](int index) const {
        switch (index) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default: throw std::out_of_range("Index out of range");
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const vec3_t& vec) {
        os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, vec3_t& vec) {
        is >> vec.x >> vec.y >> vec.z;
        return is;
    }

    bool is_zero() const {
        return x == 0.f && y == 0.f && z == 0.f;
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    float length_squared() const {
        return x * x + y * y + z * z;
    }

    float distance(const vec3_t& other) const {
        return (*this - other).length();
    }

    float distance_squared(const vec3_t& other) const {
        return (*this - other).length_squared();
    }

    float dot(const vec3_t& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    vec3_t cross(const vec3_t& other) const {
        return vec3_t(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
};

inline vec3_t operator*(float scalar, const vec3_t& vec) {
    return vec * scalar;
}

inline vec3_t operator/(float scalar, const vec3_t& vec) {
    return vec3_t(scalar / vec.x, scalar / vec.y, scalar / vec.z);
}

class __declspec(align(16)) vec3_aligned_t : public vec3_t {
public:
    float w;

    vec3_aligned_t& operator=(const vec3_t& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
            z = other.z;
            w = 0.f;
        }
        return *this;
    }

    vec3_aligned_t& operator=(const vec3_aligned_t& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
            z = other.z;
            w = other.w;
        }
        return *this;
    }

    void zero() {
        x = y = z = w = 0.0f;
    }
};
