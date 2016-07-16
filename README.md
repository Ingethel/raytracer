# raytracer

Ray Tracing rendering implementation for Visual Studio (v 2013).

Contains a scene with primitive objects (sphere/plane/triangle). 
Objects are declared in a mathematical format (no mesh readers/handlers) and the lighting follows the standard Ambient/Diffuse/Specular lighting with hard(normal) shadows. 
The materials can vary as: Normal, Reflective, Refractive.

There is a pthread implementation which can be switched on/off by commenting appropriate sections.

Currently it runs in ~1 sec on 4 threads in a scene 640x480 with 125 objects (release mode)
         
         
