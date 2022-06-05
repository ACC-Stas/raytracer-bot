#include <vector.h>
#include <sphere.h>
#include <ray.h>
#include <intersection.h>
#include <triangle.h>

#include <optional>

std::optional<Intersection> GetIntersection(const Ray& ray, const Sphere& sphere) {
    Vector origin_center;  // from origin to sphere center
    origin_center = sphere.GetCenter() - ray.GetOrigin();
    double tc;  // origin_center between origin and center of p1 p2 line. p1 and p2 - ray-sphere
                // crossing
    tc = DotProduct(origin_center, ray.GetDirection());
    if (tc < 0) {
        return {};
    }

    double d;  // origin_center between sphere center and center of p1 p2 line
    double l_origin_center = Length(origin_center);
    if (l_origin_center * l_origin_center < tc * tc) {
        return {};
    }

    d = sqrt(l_origin_center * l_origin_center - tc * tc);
    if (d > sphere.GetRadius()) {
        return {};
    }

    double t1c;  // half line length of p1 p2 line
    t1c = sqrt(sphere.GetRadius() * sphere.GetRadius() - d * d);

    bool origin_in_center = false;

    if (Length(ray.GetOrigin() - sphere.GetCenter()) < sphere.GetRadius()) {
        origin_in_center = true;
    }

    Vector p1;  // first interception
    if (origin_in_center) {
        p1 = ray.GetOrigin() + ray.GetDirection() * (tc + t1c);
    } else {
        p1 = ray.GetOrigin() + ray.GetDirection() * (tc - t1c);
    }

    Vector normal;
    normal = p1 - sphere.GetCenter();
    normal.Normalize();
    if (origin_in_center) {
        normal *= -1;
    }
    double dist = Length(ray.GetOrigin() - p1);
    return Intersection(p1, normal, dist);
}

std::optional<Intersection> GetIntersection(const Ray& ray, const Triangle& triangle) {
    const double epsilon = 0.000'000'1;
    Vector edge1 = triangle.GetVertex(1) - triangle.GetVertex(0);
    Vector edge2 = triangle.GetVertex(2) - triangle.GetVertex(0);
    Vector h = CrossProduct(ray.GetDirection(), edge2);
    double a = DotProduct(edge1, h);
    if (a > -epsilon && a < epsilon) {
        return {};
    }
    double f = 1.0 / a;
    Vector s = ray.GetOrigin() - triangle.GetVertex(0);
    double u = DotProduct(s, h) * f;
    if (u < 0.0 || u > 1.0) {
        return {};
    }
    Vector q = CrossProduct(s, edge1);
    double v = DotProduct(ray.GetDirection(), q) * f;
    if (v < 0.0 || u + v > 1.0) {
        return {};
    }
    double t = DotProduct(edge2, q) * f;
    if (t < epsilon) {
        return {};
    }
    Vector insertion_point = ray.GetOrigin() + ray.GetDirection() * t;
    double dist = Length(insertion_point - ray.GetOrigin());
    Vector normal = CrossProduct(edge1, edge2);
    normal.Normalize();
    if (DotProduct(normal, ray.GetDirection()) > 0) {
        normal *= -1;
    }
    return Intersection(insertion_point, normal, dist);
}

std::optional<Vector> Refract(const Vector& ray, Vector normal, double eta) {
    double cos_incidence = DotProduct(normal, ray);
    if (cos_incidence > 1 || cos_incidence < -1) {
        return {};
    }

    double eta_ratio;
    if (cos_incidence < 0) {
        cos_incidence *= -1;
        eta_ratio = 1 / eta;
    } else {
        eta_ratio = eta;
        normal *= -1;
    }

    double cos_refraction = 1 - (std::pow(eta_ratio, 2)) * (1 - (cos_incidence * cos_incidence));
    if (cos_refraction <= 0) {
        return {};
    }

    return ray * (eta_ratio) + normal * ((eta_ratio)*cos_incidence - std::sqrt(cos_refraction));
}

Vector Reflect(const Vector& ray, const Vector& normal) {
    return ray - normal * DotProduct(ray, normal) * 2;
}

Vector GetBarycentricCoords(const Triangle& triangle, const Vector& point) {
    double main_area = triangle.Area();
    Vector result;
    Triangle sub_triangle_1 = {triangle.GetVertex(1), triangle.GetVertex(2), point};
    Triangle sub_triangle_2 = {triangle.GetVertex(2), triangle.GetVertex(0), point};
    Triangle sub_triangle_3 = {triangle.GetVertex(0), triangle.GetVertex(1), point};
    result[0] = sub_triangle_1.Area() / main_area;
    result[1] = sub_triangle_2.Area() / main_area;
    result[2] = sub_triangle_3.Area() / main_area;
    return result;
}
