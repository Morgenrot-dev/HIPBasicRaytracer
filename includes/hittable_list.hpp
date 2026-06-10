#ifndef HITTABLE_LIST_HPP
#define HITTABLE_LIST_HPP


#include "hittable.hpp"
#include "ray.hpp"
#include "sphere.hpp"
#include <cstddef>
#include <hip/driver_types.h>
#include <vector>


/*
class hittable_list : public hittable {
  public:
    std::vector<shared_ptr<hittable>> objects;

    hittable_list() {}
    hittable_list(shared_ptr<hittable> object) { add(object); }

    void clear() { objects.clear(); }

    __device__ __host__ void add(shared_ptr<hittable> object) {
        objects.push_back(object);
    }

    __device__ __host__ bool hit(const ray& r, double ray_tmin, double ray_tmax, hit_record& rec) const override {
        hit_record temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_tmax;

        for (const auto& object : objects) {
            if (object->hit(r, ray_tmin, closest_so_far, temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }
};
*/

class hittable_list {

    public:
    sphere* sphere_objects = NULL;
    size_t objects_size = 0;
    unsigned int frontier = 0;
    sphere* sphere_gpu_objects = NULL;

    hittable_list() {}
    hittable_list(sphere object) { add(object); }

    ~hittable_list() { 
        
        if(sphere_objects != NULL)
            delete[] sphere_objects; 
    
    
        if(sphere_gpu_objects != NULL)
            hipFree(sphere_gpu_objects);
    
    }

    //void clear() {  }

    __device__ __host__ void add(sphere object) {
        if(sphere_objects == NULL)
        {
            sphere_objects = new sphere[(frontier + 1)];
            sphere_objects[0] = object;
            frontier++;
            objects_size++;
            return;
        }

        sphere* temp_array = new sphere[(frontier + 1)];
        temp_array[frontier] = object;
        
        for(int index = 0; index < objects_size; index++)
        {
            temp_array[index] = sphere_objects[index];
        }
        std::cout << "deleting sphere objects\n";
        delete[] sphere_objects;
        sphere_objects = temp_array;
        
        frontier++;
        objects_size++;
    }
    
    void generate_gpu_objects()
    {

        hipError_t error_code = hipMalloc(&sphere_gpu_objects, sizeof(sphere) * objects_size);

        std::cout << "Code for allocating gpu objects: " << error_code << std::endl;

        error_code = hipMemcpy(sphere_gpu_objects, sphere_objects, objects_size * sizeof(sphere), hipMemcpyHostToDevice);

        std::cout << "Code for copying gpu objects: " << error_code << std::endl;

    }

    __device__ inline bool hit(const ray& r, interval ray_t, hit_record& rec) const {
        hit_record temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;
        if(sphere_gpu_objects == NULL)
            return false;

        for(int index = 0; index < objects_size; index++)
        {
            auto object = sphere_gpu_objects[index];
            if (object.hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }


};


#endif
