#version 120
uniform sampler2D Framebuffer; // the texture with the scene you want to blur
varying vec2 vTexCoord;

// const vec2 offsets[16] = vec2[]
// (
	// vec2(+0.12493, -0.02234),
	// vec2(-0.07200, +0.24339),
	// vec2(-0.20764, +0.41428),
	// vec2(+0.35551, -0.70931),
	// vec2(+0.63864, -0.11421),
	// vec2(-0.18405, +0.62211),
	// vec2(+0.11000, -0.21948),
	// vec2(+0.23508, +0.31470),
	// vec2(-0.29001, +0.05186),
	// vec2(+0.09750, -0.32959),
	// vec2(-0.27733, -0.37126),
	// vec2(+0.53418, +0.71511),
	// vec2(-0.87866, +0.15713),
	// vec2(+0.14067, -0.47551),
	// vec2(-0.07961, +0.15884),
	// vec2(-0.07595, -0.10167)
// );

int nSamples = 8; // 54 - ultra
float flIntensity = 0.08;

float flBlurSize = 0.01;
float flSamples = float(nSamples);
float flSamplesInverse = 1.0 / flSamples;
float flBlurWeight = flBlurSize / flSamples;

void main(void)
{
	vec4 sum = vec4(0.0);

	sum = texture2D( Framebuffer, vTexCoord );

	vec4 glow = vec4( 0.0 );
	vec2 vBlurCoord = vTexCoord - flBlurWeight * (flSamples * 0.5);
	for( int i = 0; i < nSamples; i++ )
	{
		for( int j = 0; j < nSamples; j++ )
		{
			float flOffset = flBlurWeight;
			float flWeight = 1.0 - (abs(-flSamples * 0.5 + float(i)) * flSamplesInverse);
			flWeight *= 1.0 - (abs(-flSamples * 0.5 + float(j)) * flSamplesInverse);
			flWeight *= flWeight * flWeight * flWeight;
			vec2 offset = vec2( i * flOffset, j * flOffset );
			glow += texture2D( Framebuffer, vBlurCoord + offset ) * flWeight;
		}
	}

	sum += glow * flSamplesInverse * flIntensity;

	gl_FragColor = sum;
}