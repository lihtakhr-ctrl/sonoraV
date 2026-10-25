#pragma once

struct vec2_t {
    float x;
    float y;

    vec2_t() : x(0.f), y(0.f) {}
    vec2_t(float _x, float _y) : x(_x), y(_y) {}

    vec2_t& operator=(const vec2_t& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
        }
        return *this;
    }

    vec2_t operator+(const vec2_t& other) const {
        return vec2_t(x + other.x, y + other.y);
    }

    vec2_t operator-(const vec2_t& other) const {
        return vec2_t(x - other.x, y - other.y);
    }

    vec2_t operator*(const vec2_t& other) const {
        return vec2_t(x * other.x, y * other.y);
    }

    vec2_t operator/(const vec2_t& other) const {
        return vec2_t(x / other.x, y / other.y);
    }

    vec2_t& operator+=(const vec2_t& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    vec2_t& operator-=(const vec2_t& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    vec2_t& operator*=(const vec2_t& other) {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    vec2_t& operator/=(const vec2_t& other) {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    vec2_t operator*(float scalar) const {
        return vec2_t(x * scalar, y * scalar);
    }

    vec2_t operator/(float scalar) const {
        return vec2_t(x / scalar, y / scalar);
    }

    vec2_t& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    vec2_t& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    vec2_t operator+() const {
        return *this;
    }

    vec2_t operator-() const {
        return vec2_t(-x, -y);
    }

    bool operator==(const vec2_t& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const vec2_t& other) const {
        return !(*this == other);
    }

    float& operator[](int index) {
        switch (index) {
        case 0: return x;
        case 1: return y;
        default: throw std::out_of_range("Index out of range");
        }
    }

    const float& operator[](int index) const {
        switch (index) {
        case 0: return x;
        case 1: return y;
        default: throw std::out_of_range("Index out of range");
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const vec2_t& vec) {
        os << "(" << vec.x << ", " << vec.y << ")";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, vec2_t& vec) {
        is >> vec.x >> vec.y;
        return is;
    }

    bool is_zero() const {
        return x == 0.f && y == 0.f;
    }

    float length() const {
        return std::sqrt(x * x + y * y);
    }

    float length_squared() const {
        return x * x + y * y;
    }

    float distance(const vec2_t& other) const {
        return (*this - other).length();
    }

    float distance_squared(const vec2_t& other) const {
        return (*this - other).length_squared();
    }
};

inline vec2_t operator*(float scalar, const vec2_t& vec) {
    return vec * scalar;
}

inline vec2_t operator/(float scalar, const vec2_t& vec) {
    return vec2_t(scalar / vec.x, scalar / vec.y);
}
