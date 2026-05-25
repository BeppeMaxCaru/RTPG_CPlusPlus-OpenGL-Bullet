# Documentation

## Description

Soft bodies simulation using OpenGL graphics engine and Bullet Physics physics engine.

## Classes overview

- **Main**: Main file of the project.
- **Model**: Uses Assimp to import a model from an external file.
- **Mesh**: Defines a mesh by storing all the vertiecs and indices.
- **Shader**: Shader loading class.
- **Camera**: Flying omni-directional camera.
- **VAO**: Vertex Array Object class.
- **VBO**: Vertex Buffer Object class.
- **IBO**: Index Buffer Object class.
- **Physics**: Bullet physics simulation.

## Execution flow

1. Initialization

    - OpenGL window and context setup
    - Load vertex and fragment shader
    - Initialize camera
    - Load external models
    - Bullet physics simulation setup
    - Generate world plane
    - Simulation's frame rate setup
    - GUI initialization

2. Rendering loop

    - React to keyboard input
    - Calculate current frame
    - Move forward physics simulation
    - Activate shader program
    - Update camera position
    - Draw world plane
    - Collect GUI input from user
    - If “Generate” button is clicked then create a soft body using the - GUI collected input
    - Draw the present soft bodies
    - Swap window’s front and back buffers
    - Process events in the queue

3. Free resources
    - Delete GUI
    - Delete shader program
    - Delete physics simulation
    - Delete OpenGL window and context
