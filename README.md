# HIP Ray Tracer

A GPU-accelerated ray tracer written with AMD's **HIP / ROCm** compute stack. It
started as a port of Peter Shirley's *Ray Tracing in One Weekend* from CPU C++ to
GPU kernels — the interesting work wasn't the ray–sphere math, it was getting a
design built around virtual dispatch and recursion to run correctly on a GPU.

- ADD IMAGE HERE

## Overview

The program builds a small world of spheres on the host, configures a camera, and
launches a HIP kernel that traces rays in parallel — one GPU thread per pixel —
with multi-sample anti-aliasing and diffuse (Lambertian-style) bounces. The final
image is copied back to the host and written out as a PNG via the STB library.



## How it works

```
main.cxx
  ├── build hittable_list world  (spheres)
  ├── construct Camera           (viewport / pixel-delta math)
  └── Camera::init_render_image()
        ├── generate_gpu_objects()      copy sphere array → device
        ├── init_rand<<<grid,block>>>   seed per-pixel RNG
        ├── render_image<<<grid,block>>> trace + sample + shade
        └── hipMemcpy device → host
  └── PNG_Writter → output PNG
```

## Requirements

- **Linux** (only platform supported)
- **AMD ROCm** with the HIP runtime, plus `rocrand` and `hiprand`
- **`hipcc`** as the C++ compiler
- **CMake** ≥ 3.22
- **STB** (`stb_image_write`) — drop the `stb` folder into `libs/` so the build can
  find it (see Building)

## Building

```bash
git clone https://github.com/Morgenrot-dev/HIPBasicRaytracer.git
cd HIPBasicRaytracer

# Place the STB single-header library where the build expects it:
#   libs/stb/stb_image_write.h

mkdir build && cd build
cmake ..
make
```

The build uses `hipcc` and links `hip::host`, `hip::device`, `hip::hiprand`, and
`roc::rocrand`.

## Running

```bash
./build/src/HIPRaytracing
```

On start it prints the detected HIP device count, renders the scene, and writes the
output PNG.

## Project structure

```
includes/
  vec3.hpp        vector math (host + device)
  ray.hpp         ray type
  interval.hpp    [min,max] helper
  hittable.hpp    hit_record + hittable interface
  sphere.hpp      ray–sphere intersection
  hittable_list.hpp  flat sphere array + device-side traversal
  camera.hpp      camera + render entry point
  color.hpp       color helpers
  pngwrapper.hpp  PNG output
  common.hpp      shared includes / RNG helpers
src/
  main.cxx        scene setup + driver
  camera.cxx      HIP kernels (init_rand, render_image) + host launch
```


## Credits

- Scene/algorithm basis: [*Ray Tracing in One Weekend*](https://raytracing.github.io/) by Peter Shirley et al.
- PNG output: [STB](https://github.com/nothings/stb) by Sean Barrett
- GPU stack: AMD [ROCm / HIP](https://rocm.docs.amd.com/)
