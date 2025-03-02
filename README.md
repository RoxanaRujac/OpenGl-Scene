# 3D Graphics Project – Deserted Island Scene  

<img width="590" alt="{D603DE1A-59A6-4ED0-BAF1-ED274EDBC45A}" src="https://github.com/user-attachments/assets/bd2b533e-575a-425f-947e-5dd20ba5262a" />


## Table of Contents  
1. [Project Overview](#project-overview)  
2. [Technologies Used](#technologies-used)  
3. [Features](#features)  
4. [Implementation Details](#implementation-details)  
5. [User Interaction](#user-interaction)  
6. [Future Improvements](#future-improvements)  

## Project Overview  
This project is a **3D graphical scene** created using **C++** and **OpenGL**, depicting a **deserted island** with various interactive elements. The scene includes environmental effects such as **fog, rain, dynamic lighting, and real-time shadows**, and allows users to explore and modify rendering settings for an immersive experience.  

## Technologies Used  
- **C** – Core programming language for logic and rendering.  
- **OpenGL** – Graphics API for rendering 3D objects and effects.  
- **Blender** – Used for modeling 3D objects included in the scene.  

## Features  
- **3D Environment**: A detailed island with objects like palm trees, treasure, and a pirate shipwreck.  
- **Environmental Effects**: Implementations for **fog**, **rain**, and **transparent objects**.  
- **Lighting System**: Supports **directional and point lights**, with real-time shadow generation using **Shadow Mapping**.  
- **Rendering Modes**: Users can switch between **wireframe, polygonal, and smooth** visualizations.  
- **Interactive Camera**: Navigation using **keyboard and mouse controls**.  

<img width="378" alt="{4006FE6A-0057-4B4D-B964-C326E228B246}" src="https://github.com/user-attachments/assets/94a82543-8f50-49b3-afe9-2d82d16be72e" />


## Implementation Details  
- **Fog Effect**: Uses an exponential quadratic formula to interpolate colors based on depth.  
- **Rain Simulation**: Individual raindrops are rendered at random positions and reset when falling out of view.  
- **Transparency & Fragment Discarding**: Objects use an **alpha channel** to control visibility.  
- **Shadow Mapping**: Depth buffering technique to generate realistic shadows from a light source.  
- **3D Models & Textures**: Imported and rendered using OpenGL shaders.  

## User Interaction  
- **Camera Controls**:  
  - `W, A, S, D` – Move forward, left, backward, right  
  - `Q, E` – Rotate camera left, right  
- **Rendering Controls**:  
  - `X` – Wireframe mode  
  - `Y` – Polygonal mode  
  - `Z` – Solid mode  
- **Lighting & Effects**:  
  - `O` – Toggle directional light  
  - `P` – Toggle point light  
  - `F` – Toggle fog  
  - `T` – Toggle transparency  
  - `M` – Show depth map  

## Future Improvements  
- **Sound Effects & Background Music** for a more immersive experience.  
- **More Dynamic Objects** such as animated creatures and water waves.  
- **Improved Shadows** with soft shadowing techniques.  
- **Enhanced Navigation** with better path-following for the automatic tour mode.  

This project demonstrates fundamental **graphics programming concepts** and serves as a practical application of **real-time rendering techniques** in OpenGL.   

