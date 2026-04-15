<img width="1200" height="800" alt="output-onlinegiftools (2)" src="https://github.com/user-attachments/assets/74597c73-8f00-48b0-aaf3-d5dbe788a079" />


-------
# **OrbitForge Engine** 🛰️

### *High-Fidelity Orbital Mechanics & Trajectory Solver*

**OrbitForge** is a hybrid C++/Python simulation suite designed for precise celestial mechanics. It bridges the gap between high-speed numerical integration and scientific visualization, providing a solid framework for solving both Boundary Value Problems (Lambert's Problem) and Initial Value Problems (Orbital Propagation).

## **Physics Engine and Tech**

### **Core Solvers**

  * **Singularity-Resistant Lambert's BVP Solver:** High-precision solver using the **Universal Variables** method (Lancaster & Blanchard, NASA 1960) and Newton-Raphson iteration for pinpoint targeting. Reinforced against 180° colinear singularities and Type-II divergence using hybrid Newton-Raphson root finding.
  * **Encke's Method for Perturbations:** Implements Encke's perturbations to integrate only the difference from a nominal path, significantly reducing numerical error for long durations.
  * **Symplectic Integration:** Uses **Velocity-Verlet** integration to maintain energy conservation and stability in long-term orbital propagation.
  * **Multibody Dynamics:** Built-in support for true planetary ephemeris data and gravitational influence from multiple celestial bodies.
  * **Celestial Geometry:** Comprehensive suite and helpers for elliptical-to-Cartesian conversions, Keplerian manipulations, and Eccentric/Mean anomaly solvers. Also tracks pericentre times.

-----

## **Architecture**

### **C++ Backend**

  * **Mission Control Architecture:** Fully-object oriented `MissionPlanner` and `MissionManifest` systems to design custom missions with ease. Engine dynamically calculates phase-angles, injection delta V's and other parameters required for the launch.
  * **Memory Efficiency:** Pass-by-reference and pre-allocated memory structures to minimize overhead during large integration loops.
  * **Telemetry Engine:** Telemetry strictly separated into its own logic, logging `static` (radius, mass, etc. of objects) and `dynamic` (positions, velocities, anomalies, etc.) data.
  * **State Management:** Handling of nominal vs. perturbed vectors to prevent precision loss in astronomical simulations.

### **Python Visualization Pipeline**
  * **Adaptive Telemetry HUD:** Renderer dynamically parses the `_manifest.csv` to intuitively generate contextual Heads-Up Displays (HUDs), tracking the true/eccentric anomalies of both the origin and target bodies in real-time.
  * **Parallel Rendering:** Multiprocessed engine that generates frame-by-frame visualizations in parallel (of the likes of Blender), drastically reducing total render times.
  * **Data Ingestion:** Automated timestep-by-timestep telemetry export to CSV with **Pandas/NumPy** optimized ingestion for post-processing and analysis.
  * **Scientific Plotting:** High-fidelity visualization of trajectories and telemetry data using **Matplotlib** and **SciPy**.

-----

## **Build Instructions**

> **Note**: The repository is made to ignore build artifacts (`.obj`, `.exe`) to maintain tidiness.
1.  **Backend:** Compile the C++ source using **MSVC** or **GCC**.
2.  **Data:** Ensure a `simulation_data/` directory exists for the CSV telemetry output.
3.  **Frontend:** Run the `solar_system_viz_MP.py` script. Ensure an `orbit_frames/` folder is present for rendering the actual output frames.

-----
## **Current Roadmap**

OrbitForge is actively being expanded by me (as of April 2026) from a small side project into a full-scale astrodynamics engine. 
Upcoming features include:

- [ ] **Hill Spheres & Spheres of Influence (SOI):** Dynamic transitions between heliocentric and planetocentric reference frames. Basically switching satellites from orbiting the sun to orbiting planets.
- [ ] **Atmospheric Drag Models:** Exponential density modeling for aerobraking and re-entry simulations.
- [ ] **Orbital Capture Burns:** Computation and execution of retrograde Target Orbital Insertion maneuvers using the Oberth effect.
- [ ] **Surface-to-Orbit Launching:** Pitch programs, gravity turns, and active thrust vectors for realistic planetary departures.

----- 

## **Academic Context**

- This engine was developed as a solo project in my 2nd semester as a Math major, focusing on the computational application of **Linear Algebra** and **Ordinary Differential Equations (ODEs)** to celestial mechanics. It also served as a learning project for transitioning from me personally being experienced with C\# game dev to the realms of C++!
- It started off only to visualise and solve Lamberts problem using the NASA papaer [1]

-----

## **References**

  1. [**A Unified Form of Lambert’s Theorem**](https://www.google.co.in/books/edition/A_Unified_Form_of_Lambert_s_Theorem/lTj2Wfu84MsC?hl=en&gbpv=0) – _Lancaster, E. R., & Blanchard, R. C. (NASA 1969)_.
  2. **Bate, R. R., Mueller, D. D., & White, J. E. (1971).** *Fundamentals of Astrodynamics.* _Dover Publications_.
  3. **Encke, J. F. (1854).** *Über die Bestimmung der Störungen durch die Correction der Coordinaten.* (for Encke's Method).

-----
