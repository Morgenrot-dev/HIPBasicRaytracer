#include <hip/amd_detail/amd_hip_runtime.h>
#include <hip/hip_runtime.h>
#include <iostream>
#include "../includes/pngwrapper.hpp"
#include "../includes/camera.hpp"
#include "color.hpp"

#include "common.hpp"
#include "hittable_list.hpp"
#include "sphere.hpp"


/*
__global__ void testKernel()
{

    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    printf("Hellow World from thread %d\n", tid);

}*/

int main()
{


    //testKernel<<<1,64>>>();

    int device_count;

    hipGetDeviceCount(&device_count);

    std::cout << device_count << std::endl;

    hittable_list world;

    world.add(sphere(vec3(0,0,-1), .5));

    world.add(sphere(vec3(0,-100.5,-1), 100));


    Camera main_camera = Camera(1080, 16.0 / 9, {0,0,0});

    vec3* image = NULL;

    int image_size;

    main_camera.init_render_image(&image, &image_size, &world);

    hipDeviceSynchronize();

    PNG_Writter writter = PNG_Writter(image, 3, main_camera.get_image_height(), 512);

    writter.create_png_file();
    
    std::cout << "Deleting Image\n";
    if(image != NULL)
        delete[] image;
    
    return 0;

}
