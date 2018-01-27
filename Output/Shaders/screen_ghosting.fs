#version 120
uniform sampler2D Framebuffer;
uniform sampler2D FramebufferPrevious;
uniform float DeltaTime;

varying vec2 vTexCoord;

void main(void)
{
	vec4 color = vec4(0.0);

	color = texture2D( Framebuffer, vTexCoord );
    color += texture2D( FramebufferPrevious, vTexCoord ) * clamp( 1.0 - DeltaTime, 0.0, 1.0 ) * 0.95;

	gl_FragColor = color;
}