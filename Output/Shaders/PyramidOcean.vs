#version 330

layout (location = 0) in vec3 LocationPosition;
layout (location = 1) in vec3 LocationNormal;

out vec3 WorldPosition;
out vec3 Normal;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
	WorldPosition = (Model * vec4(LocationPosition, 1.0)).xyz;
	Normal = LocationNormal;
    gl_Position = Projection * View * Model * vec4( LocationPosition, 1.0 );
}