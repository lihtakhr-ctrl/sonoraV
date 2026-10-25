#pragma once

struct qangle {
	float x, y, z;

    qangle() : x(0.f), y(0.f), z(0.f) {}
    qangle(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    qangle& operator=(const qangle& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
            z = other.z;
        }
        return *this;
    }

    qangle operator+(const qangle& other) const {
        return qangle(x + other.x, y + other.y, z + other.z);
    }

    qangle operator-(const qangle& other) const {
        return qangle(x - other.x, y - other.y, z - other.z);
    }

    qangle operator*(const qangle& other) const {
        return qangle(x * other.x, y * other.y, z * other.z);
    }

    qangle operator/(const qangle& other) const {
        return qangle(x / other.x, y / other.y, z / other.z);
    }

    qangle& operator+=(const qangle& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    qangle& operator-=(const qangle& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    qangle& operator*=(const qangle& other) {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    qangle& operator/=(const qangle& other) {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    qangle operator*(float scalar) const {
        return qangle(x * scalar, y * scalar, z * scalar);
    }

    qangle operator/(float scalar) const {
        return qangle(x / scalar, y / scalar, z / scalar);
    }

    qangle& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    qangle& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    qangle operator+() const {
        return *this;
    }

    qangle operator-() const {
        return qangle(-x, -y, -z);
    }

    bool operator==(const qangle& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const qangle& other) const {
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

    friend std::ostream& operator<<(std::ostream& os, const qangle& vec) {
        os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, qangle& vec) {
        is >> vec.x >> vec.y >> vec.z;
        return is;
    }

    bool is_zero() const {
        return x == 0.f && y == 0.f && z == 0.f;
    }

    void normalize() {
        while (y <= -180.f) y += 360.f;
        while (y > 180.f) y -= 360.f;

        if (x > 89.f) x = 89.f;
        if (x < -89.f) x = -89.f;
    }
};

inline qangle operator*(float scalar, const qangle& vec) {
    return vec * scalar;
}

inline qangle operator/(float scalar, const qangle& vec) {
    return qangle(scalar / vec.x, scalar / vec.y, scalar / vec.z);
}
