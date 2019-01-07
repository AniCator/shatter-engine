#version 330

layout (location = 0) in vec3 LocationPosition;
layout (location = 1) in vec3 LocationNormal;

out vec3 WorldPosition;
out vec3 Normal;

uniform mat4 Model;
uniform mat4 ModelViewProjection;

void main()
{
	WorldPosition = (Model * vec4(LocationPosition, 1.0)).xyz;
	Normal = LocationNormal;
    gl_Position = ModelViewProjection * vec4( LocationPosition, 1.0 );
}