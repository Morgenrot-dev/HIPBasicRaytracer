#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "hittable_list.hpp"
#include "png.h"
#include "vec3.hpp"
#include "../includes/color.hpp"
#include <cstddef>
#include <hip/driver_types.h>
#include <hip/hip_runtime.h>
#include "common.hpp"

class Camera{

public:

    vec3 camera_center;
    vec3 pixel_delta_u;
    vec3 pixel_delta_v;
    vec3 viewport_upper_left;
    vec3 pixel00_loc;
    unsigned int sample_count = 300;
    double focal_length = 1.0;
    double aspect_ratio;
    double viewport_height = 2.0;
    double viewport_width;
    int image_width;
    int image_height;
    float sample_scale;



public:
    Camera(int output_width, double viewport_aspect_ratio, vec3 camera_center)
    {

        this->image_height = int(output_width/viewport_aspect_ratio);
        this->image_width = output_width;
        this->image_height = (this->image_height < 1) ? 1 : image_height;
        this->viewport_width = viewport_height * (double(image_width)/image_height);
        this->pixel_delta_u = vec3(viewport_width, 0, 0) / image_width;
        this->pixel_delta_v = vec3(0,-viewport_height,0) / image_height;
        this->camera_center = camera_center;

        viewport_upper_left = camera_center - vec3(0, 0, focal_length) - (vec3(viewport_width, 0, 0)/2) - (vec3(0, -viewport_height,0)/2);

        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        this->sample_scale = 1.0 / this->sample_count;

    }

    int get_image_height()
    {

        return image_height;

    }



    void init_render_image(vec3** image_array, int* image_size, hittable_list* world);

    


    void sync()
    {

        int function_code = hipDeviceSynchronize();

    }


};


__device__ ray get_ray(int i, int j, Camera& camera, hiprandStateXORWOW_t* state);
__device__ vec3 sample_square(hiprandStateXORWOW_t* state);

/*

        dim3* image = new dim3[image_width * image_height];

        for(int image_horizontal_pixel = 0; image_horizontal_pixel < image_width; image_horizontal_pixel++)
        {

            for(int image_vertical_pixel = 0; image_vertical_pixel < image_height; image_vertical_pixel++)
            {

                dim3 pixel_center = pixel00_loc + (image_vertical_pixel * pixel_delta_u) + (image_horizontal_pixel * pixel_delta_v);

                dim3 ray_direction = pixel_center - camera_center;


            }

        }




*/

#endif
