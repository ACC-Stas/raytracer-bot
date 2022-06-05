#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <initializer_list>
#include <algorithm>

class Vector;

inline double Length(const Vector& vec);

class Vector {
public:
    Vector() {
        for (size_t id = 0; id < 3; ++id) {
            data_[id] = 0;
        }
    };
    Vector(std::initializer_list<double> list) {
        size_t id = 0;
        for (auto it = list.begin(); it != list.end(); ++it) {
            data_[id] = (*it);
            ++id;
        }
    };
    Vector(std::array<double, 3> data) : data_(data){};

    double& operator[](size_t ind) {
        return data_[ind];
    };
    double operator[](size_t ind) const {
        return data_[ind];
    };

    void Normalize() {
        double length = Length(*this);
        if (length == 0.0) {
            return;
        }
        data_[0] /= length;
        data_[1] /= length;
        data_[2] /= length;
    }

    Vector& operator-=(const Vector& rhs) {
        std::array<double, 3> data;
        for (size_t id = 0; id < 3; ++id) {
            data[id] = (*this)[id] - rhs[id];
        }
        data_ = data;
        return (*this);
    }

    bool operator==(const Vector& vec) const {
        if (std::fabs(data_[0] - vec[0]) > 1e-6) {
            return false;
        }
        if (std::fabs(data_[1] - vec[1]) > 1e-6) {
            return false;
        }
        if (std::fabs(data_[2] - vec[2]) > 1e-6) {
            return false;
        }
        return true;
    }

    Vector& operator+=(const Vector& vec) {
        data_[0] += vec[0];
        data_[1] += vec[1];
        data_[2] += vec[2];
        return *this;
    }

    Vector& operator*=(double scalar) {
        data_[0] *= scalar;
        data_[1] *= scalar;
        data_[2] *= scalar;
        return *this;
    }

    Vector& operator*=(const Vector vec) {
        data_[0] = this->data_[0] * vec.data_[0];
        data_[1] = this->data_[1] * vec.data_[1];
        data_[2] = this->data_[2] * vec.data_[2];
        return (*this);
    }

    friend Vector operator*(const Vector& l_vec, const Vector& r_vec) {
        Vector result;
        result.data_[0] = l_vec.data_[0] * r_vec.data_[0];
        result.data_[1] = l_vec.data_[1] * r_vec.data_[1];
        result.data_[2] = l_vec.data_[2] * r_vec.data_[2];
        return result;
    }

    friend Vector operator*(double scalar, const Vector& vec) {
        Vector result;
        result.data_[0] = scalar * vec.data_[0];
        result.data_[1] = scalar * vec.data_[1];
        result.data_[2] = scalar * vec.data_[2];
        return result;
    }

    Vector operator*(double scalar) const {
        Vector result;
        result[0] = this->data_[0] * scalar;
        result[1] = this->data_[1] * scalar;
        result[2] = this->data_[2] * scalar;
        return result;
    }

    friend Vector operator-(const Vector& l_vec, const Vector& r_vec) {
        Vector result;
        result.data_[0] = l_vec.data_[0] - r_vec.data_[0];
        result.data_[1] = l_vec.data_[1] - r_vec.data_[1];
        result.data_[2] = l_vec.data_[2] - r_vec.data_[2];
        return result;
    }

    friend Vector operator+(const Vector& l_vec, const Vector& r_vec) {
        Vector result;
        result.data_[0] = l_vec.data_[0] + r_vec.data_[0];
        result.data_[1] = l_vec.data_[1] + r_vec.data_[1];
        result.data_[2] = l_vec.data_[2] + r_vec.data_[2];
        return result;
    }

private:
    std::array<double, 3> data_;
};

inline double DotProduct(const Vector& lhs, const Vector& rhs) {
    return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
}

inline Vector CrossProduct(const Vector& a, const Vector& b) {
    Vector result;
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
    return result;
}

inline double Length(const Vector& vec) {
    return std::sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}
