# GPU Particle Advection in a Vector Field
This project implements a high-performance, real-time particle simulation that visualizes 3D vector fields. The advection physics and integration run entirely on the GPU via OpenGL Compute Shaders, completely eliminating CPU-GPU memory transfer bottlenecks during the simulation loop.

The application is built natively in C++ using OpenGL and the Win32 API.

## Features Implemented
### Core (Completed)

**Analytical 3D Vector Fields:** Supports switching between mathematical fields to drive the simulation:

A standard trigonometric flow field: ```v(x,y,z) = (sin(y), cos(x), sin(z))```

The **Lorenz Attractor**, mapped and scaled to visualize the chaotic "butterfly" system.

**GPU-Side Simulation**: Advects over 100,000 particles simultaneously. The simulation uses Runge-Kutta 4 (RK4) numerical integration inside the compute shader for highly accurate curve following.

**Particle Rendering**: Particles are rendered directly from the compute SSBO as points. They are dynamically colored in the fragment shader based on their calculated speed magnitude.

**Lifecycle Management**: Particles that exceed their maximum lifetime or escape the simulation bounding box are automatically respawned at random coordinates by the compute shader.

### Level 3 (Completed)

**Ping-Pong Buffer Architecture**: Implemented a dual-SSBO system to strictly prevent race conditions and read/write hazards across the 100,000 concurrent threads. (See the Architecture section below for details).

**Acceleration Structure (3D Texture)**: To avoid evaluating complex analytical math (like the Lorenz equations) 4 times per particle per frame for RK4, the vector field is pre-computed into a 3D grid and uploaded to the GPU as a GL_TEXTURE_3D. The compute shader samples this texture utilizing hardware-accelerated trilinear interpolation.

### Architecture: Ping-Pong Buffers & Synchronization
To safely update the particle states without read/write hazards, the compute pipeline utilizes a Ping-Pong buffer approach.

**Dual SSBOs**: Two identical Shader Storage Buffer Objects are allocated on the GPU.

**Dispatch**: During the compute pass, Buffer A is bound to binding point 0 as readonly (providing the current particle state), and Buffer B is bound to binding point 1 as writeonly (receiving the newly integrated positions and velocities).

**Synchronization**: A glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT) is dispatched immediately after the compute pass to ensure all writes to Buffer B are fully flushed and visible before the rendering pipeline attempts to draw them.

**Swap**: The CPU simply swaps the read/write indices at the end of the frame.

This guarantees that no thread is reading a particle's state while another thread is halfway through overwriting it.


## How to Build and Run
### Requirements & Constraints:

**OS**: Windows (This application utilizes the native Win32 windowing system).

**Compiler**: MSVC (Microsoft Visual C++).

### Build Steps:

Open the **x64 Native Tools Command Prompt for VS** (Visual Studio Developer Command Prompt).

Navigate to the root directory of this repository.

Run the provided build script:

```
build.bat
```
(Optional flags: Run build.bat clean to remove object files, or build.bat debug for a build with debug symbols and no optimization).

Upon successful compilation and linking, the script will copy **glew32.dll** and output **App.exe** to the root folder.

Run the executable:

```
App.exe
```
