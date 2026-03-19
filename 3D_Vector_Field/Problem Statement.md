Problem Statement — GPU Particle Advection in a Vector Field

Background - Visualizing fluid flow, electromagnetic fields, or particle trajectories requires advecting massless particles through a vector field — computing where they travel over time based on the field's velocity at each point. This is a core task in CFD and scientific visualization.


Core (required)
1. Define a 3D vector field — Analytically, e.g., a curl noise field, a Lorenz-attractor-style field, or something like velocity(x,y,z) = (sin(y), cos(x), sin(z+t)). No need to load external data.
2. GPU-side particle simulation — Simulate at least 100,000 particles using a compute shader. Each frame, advect each particle by one RK2 or RK4 step. Particle state (position, age, velocity) must live entirely in a GPU buffer.
3. Render the particles — Render as points or short line segments (current position → previous position). Color by speed magnitude or particle age.
4. Spawn / reset — Particles that leave the bounding box or exceed their maximum age are respawned at random positions.

Level 2
1. Streamlines or pathlines — In addition to (or instead of) particles, render a set of streamlines from fixed seed points tracing forward through the field. Compute these on the GPU.
2. Time-varying field — Animate the vector field by varying a time uniform. Observe and demonstrate how the particles respond.

Level 3
1. Ping-pong buffer architecture — Structure your compute pass to avoid read/write hazards. Explain clearly in your README how you approached this.
2. Acceleration structure — If you grid-sample the field, implement a 3D texture for field lookup with trilinear interpolation on the GPU.
