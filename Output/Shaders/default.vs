#version 330

layout (location = 0) in vec3 LocationPosition;

out vec3 WorldPosition;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
	WorldPosition = (Model * vec4(Position, 1.0)).xyz;
    gl_Position = Projection * View * Model * vec4( LocationPosition, 1.0 );
}