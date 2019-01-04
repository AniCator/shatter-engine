#version 330

in vec3 WorldPosition;

uniform mat4 Model;
uniform vec4 ObjectColor;
uniform vec3 CameraPosition;
uniform vec3 MousePositionWorldSpace;

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

void main()
{
	vec3 LightPosition = vec3( 0, 0, 0 );
	LightPosition.z = 0;
	
	vec3 LightColor = vec3( 0.4, 1, 0.2 ) * 100;
	
	vec3 WorldNormal = vec3( 0, 0, -1 );
	vec3 ViewVector = normalize( CameraPosition - WorldPosition );
	vec3 LightDirection = normalize( LightPosition - WorldPosition );
	vec3 HalfVector = normalize( LightDirection + ViewVector );
	
	float NdotL = clamp( dot( WorldNormal, LightDirection ), 0, 1 );
	float NdotH = clamp( dot( WorldNormal, HalfVector ), 0, 1 );
	float VdotH = clamp( dot( ViewVector, HalfVector ), 0, 1 );
	
	vec3 FresnelTerm = Fresnel( vec3( 0 ), VdotH );
	vec3 DiffuseTerm = vec3( NdotL );
	vec3 SpecularTerm = D_GGX( 0.2, NdotH ) * FresnelTerm;
	
	vec3 DiffuseAlbedo = DiffuseTerm * ObjectColor.rgb;
	
	vec3 Lighting = ( SpecularTerm + ( ( DiffuseAlbedo * ( 1.0 - FresnelTerm ) ) * PiInverse ) ) * DiffuseTerm * LightColor;
	Lighting = pow( Lighting, vec3( 0.45 ) );
	
    FragColor = vec4( Lighting, ObjectColor.a );
	FragColor = ObjectColor;
}
