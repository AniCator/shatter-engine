#version 330

in vec3 WorldPosition;
in vec3 Normal;

uniform mat4 Model;
uniform vec4 ObjectColor;
uniform vec3 CameraPosition;
uniform float Time;

out vec4 FragColor;

#define Pi 3.14159265
#define PiInverse 0.31830988
#define FourPiInverse 1.2732395447351626861510701069801
#define PowerTwoPi 9.8696044010893586188344909998762

#define f0 0.2315

#define MipScale 2.0
#ifndef SunDiskRadius
//#define SunDiskRadius 0.087
#define SunDiskRadius 0.06525
#endif

#define AmbientDiskRadius 0.1

#define BoxSize 1024
#define CubeMapSize 32

#define sqr( x ) ( ( x ) * ( x ) )
#define rcp( x ) 1.0 / x

vec3 Fresnel( vec3 SpecularColor, float VdotH )
{
	return SpecularColor + ( 1.0 - SpecularColor ) * pow( 1.0 - clamp( VdotH, 0, 1 ), 5.0 );
}

float D_GGX( float Roughness, float NdotH )
{
	float a = Roughness * Roughness;
	float a2 = a * a;
	float d = ( NdotH * a2 - NdotH ) * NdotH + 1.0;	// 2 mad
	return a2 / ( Pi * d * d ); // 4 mul, 1 rcp
}

//	3.2404542, 	-1.5371385, -0.4985314,
//	-0.9692660,  1.8760108,  0.0415560,
//	0.0556434, 	-0.2040259,  1.0572252
mat3 XYZtoRGB = mat3(
	3.2404542, -0.9692660, 0.0556434,
	-1.5371385, 1.8760108, -0.2040259,
	-0.4985314, 0.0415560, 1.0572252
	);

vec3 BlackBody( float Temperature )
{
	float u = ( 0.860117757 + 1.54118254e-4 * Temperature + 1.28641212e-7 * Temperature * Temperature ) / ( 1.0 + 8.42420235e-4 * Temperature + 7.08145163e-7 * Temperature * Temperature );
	float v = ( 0.317398726 + 4.22806245e-5 * Temperature + 4.20481691e-8 * Temperature * Temperature ) / ( 1.0 - 2.89741816e-5 * Temperature + 1.61456053e-7 * Temperature * Temperature );

	float x = 3 * u / ( 2 * u - 8 * v + 4 );
	float y = 2 * v / ( 2 * u - 8 * v + 4 );
	float z = 1 - x - y;

	float Y = 1;
	float X = Y / y * x;
	float Z = Y / y * z;

	return XYZtoRGB * vec3( X, Y, Z ) * pow( 0.0004 * Temperature, 4 );
}

float GetLightIntensity( float Lumens )
{
	return Lumens * 100.0 * 100.0 / 4 / Pi;
}

void main()
{
	vec3 LightPosition = vec3( 0, 0, 1000 );
	
	float TimeMultiplier = sin( Time ) * 0.5 + 0.5;
	
	float LightIntensity = GetLightIntensity( 3000 );
	vec3 LightColor = BlackBody( 6500 * TimeMultiplier ) * LightIntensity;
	
	vec3 WorldNormal = normalize( vec3( Model * vec4( Normal, 1.0 ) ) );
	WorldNormal = vec3( 0, 0, 1 ); // Normal input is still incorrect, use this one for now.
	vec3 ViewVector = normalize( CameraPosition - WorldPosition );
	vec3 LightDirection = normalize( LightPosition - WorldPosition );
	vec3 HalfVector = normalize( LightDirection + ViewVector );
	
	float NdotL = max( dot( WorldNormal, LightDirection ), 0 );
	float NdotH = max( dot( WorldNormal, HalfVector ), 0 );
	float VdotH = max( dot( ViewVector, HalfVector ), 0 );
	
	vec3 FresnelTerm = Fresnel( vec3( 0 ), VdotH );
	vec3 DiffuseTerm = vec3( NdotL );
	vec3 SpecularTerm = D_GGX( 0.65, NdotH ) * FresnelTerm;
	
	vec3 DiffuseAlbedo = DiffuseTerm * pow( ObjectColor.rgb, vec3( 2.2 ) );
	
	vec3 Lighting = ( SpecularTerm + ( ( DiffuseAlbedo * ( 1.0 - FresnelTerm ) ) * PiInverse ) ) * DiffuseTerm * LightColor;
	
	float Distance = length( LightPosition - WorldPosition );
	Lighting *= 1.0 / max( Distance * Distance, 0.01 * 0.01 );
	
	Lighting += vec3( 0.05, 0.05, 0.05 ) * ObjectColor.rgb; // Ambient
	
    FragColor = vec4( Lighting, ObjectColor.a );
}
