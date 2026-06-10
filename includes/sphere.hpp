#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "hittable.hpp"
#include "common.hpp"

class sphere {
  public:
    sphere(const vec3& center, double radius) : center(center), radius(fmax(0,radius)) {}
    sphere() { }

    __device__ __host__ bool hit(const ray& r, interval ray_t, hit_record& rec) const {
        vec3 oc = center - r.origin();
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - radius*radius;

        auto discriminant = h*h - a*c;
        if (discriminant < 0)
            return false;

        auto sqrtd = sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root)){
                
                return false;
            }
        }

        rec.t = root;
        rec.point = r.at(rec.t);
        vec3 outward_normal = (rec.point - center) / radius;
        rec.set_face_normal(r, outward_normal);

        return true;
    }

  private:
    vec3 center;
    double radius;
};

/*
class sphere {
  public:
  sphere() { }
  sphere(const vec3& center, double radius) : center(center), radius(fmax(0,radius)) {}
  __host__ __device__ bool hit(const ray& r, double ray_tmin, double ray_tmax, hit_record& rec) {
        vec3 oc = center - r.origin();
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - radius*radius;

        auto discriminant = h*h - a*c;
        if (discriminant < 0)
            return false;

        auto sqrtd = sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a;
        if (root <= ray_tmin || ray_tmax <= root) {
            root = (h + sqrtd) / a;
            if (root <= ray_tmin || ray_tmax <= root)
                return false;
        }

        rec.t = root;
        rec.point = r.at(rec.t);
        vec3 outward_normal = (rec.point - center) / radius;
        rec.set_face_normal(r, outward_normal);

        return true;
    }

private:
    vec3 center;
    double radius;

};
*/
#endif
