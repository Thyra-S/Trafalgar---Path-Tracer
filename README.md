# Ray Tracing
A from-scratch, C++ physically-based ray tracer built to simulate realistic lighting, materials, and shadows. 

![Final Render of Primitive Shapes](https://github.com/Thyra-S/Ray-Tracing/blob/master/Software%20Ray%20Tracer/Renders/1610raw10k.png?raw=true)

## ✨ Features

*   **Primitives:** Spheres, Planes, and Triangles/Meshes.
*   **Materials:** 
    *   *Diffuse (Matte):* Scatters light randomly.
    *   *Metal (Reflective):* Perfectly reflective or slightly fuzzy reflections.
    *   *Dielectric (Glass):* Refraction and reflection based on Snell's law and Schlick's approximation.
*   **Lighting:** Point lights, directional lights, and soft shadows.
*   **Anti-Aliasing:** Multisampled standard AA for smooth edges.
*   **Camera:** Positionable virtual camera with adjustable Field of View (FOV) and Depth of Field (Defocus Blur).
*   **Output:** Generates standard `.ppm` image files.
*   **.obj File Loading:** Imports .obj files using simple OBJ loader
*   **Multithreading:** Dynamically allocates different ray calculations to different threads
*   **BVH:** Simple Binary BVH creation splitting along the longest-axis as well as accurate traversal.

## 🖼️ Gallery
![Glass Teapot](https://github.com/Thyra-S/Ray-Tracing/blob/master/Software%20Ray%20Tracer/Renders/teapotcornell.png?raw=true)
![Nike of Samothrace Glass](https://github.com/Thyra-S/Ray-Tracing/blob/master/Software%20Ray%20Tracer/Renders/nikecornell.png?raw=true)
![Miku texture importing](https://github.com/Thyra-S/Ray-Tracing/blob/master/Software%20Ray%20Tracer/Renders/miku.png?raw=true)
![Spheres only, Final Render](https://github.com/Thyra-S/Ray-Tracing/blob/master/Software%20Ray%20Tracer/Renders/manyballs1440.png?raw=true)
![Early Render](https://github.com/Thyra-S/Ray-Tracing/blob/master/Software%20Ray%20Tracer/Renders/output.png?raw=true)


📚 Acknowledgments

Ray Tracing in One Weekend by Peter Shirley - An incredible resource for learning the fundamentals of ray tracing.
