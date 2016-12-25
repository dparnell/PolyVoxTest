#version 330 core

layout(location = 0) in vec4 position;
layout(location = 2) in uint quantizedColor;

// The usual matrices are provided
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

vec3 decodeColor(uint quantizedColor)
{
    quantizedColor >>= 8;
    float blue = (quantizedColor & 0xffu);
    quantizedColor >>= 8;
    float green = (quantizedColor & 0xffu);
    quantizedColor >>= 8;
    float red = (quantizedColor & 0xffu);
    
    vec3 result = vec3(red, green, blue);
    result *= (1.0 / 255.0);
    return result;
}

out vec4 worldPosition;
out vec3 voxelColor;

void main()
{
    // Extract and decode the color.
    voxelColor = decodeColor(quantizedColor);
    // Standard sequence of OpenGL transformations.
    worldPosition = modelMatrix * position;
    vec4 cameraPosition = viewMatrix * worldPosition;
    gl_Position = projectionMatrix * cameraPosition;
}
