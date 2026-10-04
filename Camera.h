#pragma once
#include <Eigen/Dense>
#include <algorithm>
#include <cmath>

// Orbit camera (Z-up). position/target/up are kept in sync by updateFromOrbit().
struct Camera {
    Eigen::Vector3f position{0, 0, 10};
    Eigen::Vector3f target{0, 0, 0};
    Eigen::Vector3f up{0, 0, 1};
    float fov = 45.0f;
    float aspect = 1.777f;
    float nearPlane = 0.1f;
    float farPlane = 10000.f;
    float yaw = -45.f, pitch = 30.f, distance = 300.f;   // degrees / world units

    void updateFromOrbit() {
        const float d2r = 3.14159265f / 180.f;
        float cp = std::cos(pitch * d2r);
        position = target + distance * Eigen::Vector3f(cp * std::cos(yaw * d2r), cp * std::sin(yaw * d2r), std::sin(pitch * d2r));
        nearPlane = std::max(0.01f, distance * 0.01f);
        farPlane = distance * 50.f;
    }
    void orbit(float dx, float dy) {
        yaw -= dx * 0.4f;
        pitch = std::clamp(pitch + dy * 0.4f, -89.f, 89.f);
        updateFromOrbit();
    }
    void pan(float dx, float dy) {
        Eigen::Vector3f f = (target - position).normalized();
        Eigen::Vector3f s = f.cross(up).normalized();
        Eigen::Vector3f u = s.cross(f);
        target += (-s * dx + u * dy) * distance * 0.0015f;
        updateFromOrbit();
    }
    void zoom(float factor) { distance = std::clamp(distance * factor, 1e-2f, 1e7f); updateFromOrbit(); }

    Eigen::Matrix4f view() const {
        Eigen::Vector3f f = (target - position).normalized();
        Eigen::Vector3f s = f.cross(up).normalized();
        Eigen::Vector3f u = s.cross(f);
        Eigen::Matrix4f m = Eigen::Matrix4f::Identity();
        m.block<1, 3>(0, 0) = s.transpose();  m(0, 3) = -s.dot(position);
        m.block<1, 3>(1, 0) = u.transpose();  m(1, 3) = -u.dot(position);
        m.block<1, 3>(2, 0) = -f.transpose(); m(2, 3) = f.dot(position);
        return m;
    }
    Eigen::Matrix4f projection() const {
        float t = 1.0f / std::tan(fov * 0.5f * 3.14159265f / 180.f);
        Eigen::Matrix4f p = Eigen::Matrix4f::Zero();
        p(0, 0) = t / aspect;
        p(1, 1) = t;
        p(2, 2) = (farPlane + nearPlane) / (nearPlane - farPlane);
        p(2, 3) = 2.f * farPlane * nearPlane / (nearPlane - farPlane);
        p(3, 2) = -1.f;
        return p;
    }
};
