#include "../includes/camera.hpp"
#include "color.hpp"
#include "hittable.hpp"
#include "hittable_list.hpp"
#include "ray.hpp"
#include "vec3.hpp"
#include <cstddef>
#include <cstdint>
#include <hip/driver_types.h>
#include <hip/hip_runtime.h>
#include <hiprand_kernel.h>
#include <hiprand_kernel_rocm.h>
#include <rocrand.h>
#include <sys/select.h>
#include <sys/types.h>
#include <common.hpp>
#include <rocrand.hpp>

__device__ vec3 sample_square(hiprandStateXORWOW_t* state) {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double(state) - 0.5, random_double(state) - 0.5, 0);
    }

__device__ ray get_ray(int i, int j, Camera& camera, hiprandStateXORWOW_t* state)
{

    auto offset = sample_square(state);
    auto pixel_sample = camera.pixel00_loc + ((i + offset.x) * camera.pixel_delta_u) + ((j + offset.y) * camera.pixel_delta_v);
    auto ray_origin = camera.camera_center;
    auto ray_direction = pixel_sample - ray_origin;

    return ray(ray_origin, ray_direction);
}

__device__ vec3 nr_ray_color(const ray& r, const hittable_list& world, hiprandStateXORWOW_t* generatorState)
{
    
    hit_record rec;
    ray new_ray;
    vec3 color_product =  ray_color(r, world, rec);
    vec3 direction = random_on_hemisphere(rec.normal, generatorState);

    new_ray = ray(rec.point, direction);
    
    //"Bouncing" too much
    for(int index = 0; index < 10; index++){
        if(!rec.hit)
        {break;}

        vec3 bounce_color = ray_color(new_ray, world, rec);
        color_product.x *= bounce_color.x;
        color_product.y *= bounce_color.y;
        color_product.z *= bounce_color.z;
        vec3 direction = random_on_hemisphere(rec.normal, generatorState);

        new_ray = ray(rec.point, direction);
        

    }
    
    return color_product;


}

__device__ vec3 nr_ray_color(const ray& r, const hittable_list& world, hiprandStateXORWOW_t* generatorState, int idx, int idy)
{
        
    hit_record rec;
    bool hit = false;
    ray new_ray;
    vec3 color_product =  ray_color(r, world, rec, &hit);
    vec3 direction = random_unit_vector(generatorState) + rec.normal;
    new_ray = ray(rec.point, direction);

        
    //"Bouncing" too much
    for(int index = 0; index < 10; index++){
        if(hit == 0 )
        {break;}

                

        vec3 bounce_color = ray_color(new_ray, world, rec, &hit);
        color_product.x *= bounce_color.x;
        color_product.y *= bounce_color.y;
        color_product.z *= bounce_color.z;
        vec3 direction = random_unit_vector(generatorState) + rec.normal;

		
		new_ray = ray(rec.point, direction);
		

	    }
	    
	    return color_product;


	}

	__global__ static void render_image(vec3* image, int total_image_width, int total_image_height, Camera camera_class, hittable_list* world, hiprandStateXORWOW_t* generator)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    int idy = blockIdx.y * blockDim.y + threadIdx.y;

    
    
    if(idx >= total_image_width || idy >= total_image_height)
        return;    


    //vec3 pixel_center = camera_class.pixel00_loc + (idx * camera_class.pixel_delta_u) + (idy * camera_class.pixel_delta_v);
    vec3 color;
    
    unsigned int pixelnum = 0;
    if(idx + (idy * total_image_width)  < total_image_width * total_image_height){
        pixelnum = idx + (idy * total_image_width);
        }
    
    //vec3 ray_direction = pixel_center - camera_class.camera_center;
    
    hiprandStateXORWOW_t localstate;
    if(idx == 0 && idy == 0)
        printf("Loop Starting\n");
    for(int index = 0; index < camera_class.sample_count; index++)
    {
        localstate = generator[pixelnum];
        ray r = get_ray(idx, idy, camera_class, &localstate);

        

        vec3 temp_color = nr_ray_color(r, *world, &localstate, idx,idy);

        

        color = color + temp_color;
        //color += ray_color(r,*world,generator, 10);

    

        generator[pixelnum] = localstate;
        
    }


    
    color = color * camera_class.sample_scale;

    if(idx + (idy * total_image_width)  < total_image_width * total_image_height)
        image[idx + (idy * total_image_width)] = color;

    
}





__global__ void init_rand(hiprandStateXORWOW_t* state, int width, int height)
{

    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    int idy = blockIdx.y * blockDim.y + threadIdx.y;
    if(idx < width && idy < height)
        hiprand_init(0,0, 0,&state[idx + (idy * width)]);

}

//void init_all_hiprand(hiprandStateXORWOW_t* state, int width)


    void Camera::init_render_image(vec3** image_array, int* image_size, hittable_list* world)
    {


        dim3 block_size = {32, 32,1};

        dim3 grid_size = {(uint32_t)ceil((float)image_width/32), (uint32_t)ceil((float)image_height/32), 1};

        vec3* gpu_image = NULL;

        world->generate_gpu_objects();

        int function_code  = hipMalloc(&gpu_image, image_height * image_width * sizeof(vec3));

        hittable_list* gpu_world = NULL;

        hipMalloc(&gpu_world, sizeof(hittable_list));

        hipMemcpy(gpu_world, world, sizeof(hittable_list), hipMemcpyHostToDevice);

        rocrand_generator gen;

        rocrand_create_generator(&gen, ROCRAND_RNG_PSEUDO_DEFAULT);

        hiprandStateXORWOW_t* generator = NULL;

        hipMalloc(&generator, image_height * image_width * sizeof(hiprandStateXORWOW_t));

        init_rand<<<grid_size,block_size>>>(generator, image_width, image_height);
        
        hipDeviceSynchronize();

        render_image<<<grid_size,block_size>>>(gpu_image, image_width, image_height, *this, gpu_world, generator);

        //hipLaunchKernelGGL(render_image, grid_size, block_size, 0, 0, gpu_image, image_width, image_height, *this);

        hipDeviceSynchronize();

        std::cout << function_code << std::endl;

        *image_array = new vec3[image_height * image_width];

        function_code = hipMemcpy(*image_array, gpu_image, image_height * image_width * sizeof(vec3), hipMemcpyDeviceToHost);

        std::cout << function_code << std::endl;

        function_code  = hipFree(gpu_image);

        std::cout << function_code << std::endl;

        *image_size = image_height * image_width;

        hipFree(generator);

        rocrand_destroy_generator(gen);

        return;
    }
