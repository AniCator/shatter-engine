#version 330

uniform vec4 ObjectColor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 1.0, 1.0, 1.0) * ObjectColor;
}
