#version 330

layout (location = 0) in vec3 LocationPosition;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
    gl_Position = Projection * View * Model * vec4( LocationPosition, 1.0 );
}