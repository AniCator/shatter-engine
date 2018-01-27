varying vec2 vTexCoord;
 
// remember that you should draw a screen aligned quad
void main(void)
{
	gl_Position = ftransform();

	// Clean up inaccuracies
	vec2 Pos;
	Pos = sign(gl_Vertex.xy);

	gl_Position = vec4(Pos, 0.0, 1.0);
	// Image-space
	vTexCoord = Pos * 0.5 + 0.5;
}