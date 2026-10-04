#include "sketch/ConstraintSolver.h"
#include <cmath>
#include <Eigen/Dense>

using Eigen::Vector2d;
using Eigen::VectorXd;

bool ConstraintSolver::solve(Sketch& sketch, const std::vector<Constraint>& cs) {
    auto& P = sketch.points();
    auto& C = sketch.circles();
    const auto& L = sketch.lines();
    const auto P0 = P;
    const int np = (int)P.size(), nc = (int)C.size(), n = 2 * np + nc;
    if (n == 0 || cs.empty()) return true;

    auto pt = [&](const VectorXd& v, int i) -> Vector2d { return Vector2d(v[2 * i], v[2 * i + 1]); };
    auto dir = [&](const VectorXd& v, int li) -> Vector2d { return pt(v, L[li].pointB) - pt(v, L[li].pointA); };
    auto cross = [](const Vector2d& a, const Vector2d& b) -> double { return a.x() * b.y() - a.y() * b.x(); };

    auto residual = [&](const VectorXd& v) {
        std::vector<double> r;
        for (const auto& c : cs) {
            switch (c.type) {
                case ConstraintType::Coincident: { Vector2d d = pt(v, c.geometryA) - pt(v, c.geometryB); r.push_back(d.x()); r.push_back(d.y()); break; }
                case ConstraintType::Horizontal: r.push_back(dir(v, c.geometryA).y()); break;
                case ConstraintType::Vertical:   r.push_back(dir(v, c.geometryA).x()); break;
                case ConstraintType::Parallel:   r.push_back(cross(dir(v, c.geometryA).normalized(), dir(v, c.geometryB).normalized())); break;
                case ConstraintType::Perpendicular: r.push_back(dir(v, c.geometryA).normalized().dot(dir(v, c.geometryB).normalized())); break;
                case ConstraintType::Equal:      r.push_back(dir(v, c.geometryA).norm() - dir(v, c.geometryB).norm()); break;
                case ConstraintType::Distance:   r.push_back((pt(v, c.geometryA) - pt(v, c.geometryB)).norm() - c.value); break;
                case ConstraintType::Angle: {
                    Vector2d a = dir(v, c.geometryA), b = dir(v, c.geometryB);
                    r.push_back(std::atan2(cross(a, b), a.dot(b)) - c.value);
                    break;
                }
                case ConstraintType::Radius:   r.push_back(v[2 * np + c.geometryA] - c.value); break;
                case ConstraintType::Diameter: r.push_back(2.0 * v[2 * np + c.geometryA] - c.value); break;
                case ConstraintType::Tangent: {
                    Vector2d a = pt(v, L[c.geometryA].pointA), d = dir(v, c.geometryA).normalized();
                    Vector2d ctr = pt(v, C[c.geometryB].centerPoint);
                    r.push_back(std::abs(cross(d, ctr - a)) - v[2 * np + c.geometryB]);
                    break;
                }
                case ConstraintType::Fixed: {
                    Vector2d d = pt(v, c.geometryA) - P0[c.geometryA].position;
                    r.push_back(d.x()); r.push_back(d.y());
                    break;
                }
            }
        }
        VectorXd out(static_cast<Eigen::Index>(r.size()));
        for (size_t i = 0; i < r.size(); ++i) out[static_cast<Eigen::Index>(i)] = r[i];
        return out;
    };

    VectorXd x(n);
    for (int i = 0; i < np; ++i) { x[2 * i] = P[i].position.x(); x[2 * i + 1] = P[i].position.y(); }
    for (int i = 0; i < nc; ++i) x[2 * np + i] = C[i].radius;

    double lambda = 1e-3;
    VectorXd r = residual(x);
    if (r.size() == 0) return true;
    for (int iter = 0; iter < 200 && r.norm() > 1e-10; ++iter) {
        Eigen::MatrixXd J(r.size(), n);
        for (int j = 0; j < n; ++j) {
            VectorXd xp = x; xp[j] += 1e-7;
            J.col(j) = (residual(xp) - r) / 1e-7;
        }
        Eigen::MatrixXd A = J.transpose() * J + lambda * Eigen::MatrixXd::Identity(n, n);
        VectorXd step = A.ldlt().solve(-J.transpose() * r);
        VectorXd rn = residual(x + step);
        if (rn.norm() < r.norm()) { x += step; r = rn; lambda = std::max(1e-12, lambda * 0.3); }
        else { lambda *= 10.0; if (lambda > 1e12) break; }
    }
    for (int i = 0; i < np; ++i) P[i].position = Vector2d(x[2 * i], x[2 * i + 1]);
    for (int i = 0; i < nc; ++i) C[i].radius = x[2 * np + i];
    return r.norm() < 1e-6;
}
