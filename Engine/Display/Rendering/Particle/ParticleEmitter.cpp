// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "ParticleEmitter.h"

#include <Engine/Audio/SoLoudSound.h>
#include <Engine/Display/Rendering/Shader.h>
#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/Window.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Utility/Math.h>

#include <ThirdParty/glad/include/glad/glad.h>

#include <Game/Game.h>
#include <Engine/Display/UserInterface.h>

static constexpr size_t WorkSize = 256;

class ParticleRenderable : public CRenderable
{
public:	
	ParticleRenderable( ParticleEmitter* Emitter )
	{
		this->Emitter = Emitter;
	}

	~ParticleRenderable() override
	{
		if( VAO )
		{
			glDeleteVertexArrays( 1, &VAO );
		}
	}

	void Initialize( Particle* Data, const size_t& Count )
	{
		if( Emitter && Emitter->Render )
		{
			Shader = Emitter->Render;
		}

		Buffer.Initialize( Data, Count );
		
		glGenVertexArrays( 1, &VAO );
		glBindVertexArray( VAO );
		Buffer.Bind();
		
		glEnableVertexAttribArray( 0 );
		glBindBuffer( GL_ARRAY_BUFFER, Buffer.Handle() );
		glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, sizeof( Particle ), nullptr );

		glBindVertexArray( 0 );
	}
	
	void Draw( FRenderData& RenderData, const FRenderData& PreviousRenderData, EDrawMode DrawModeOverride ) override
	{
		if( !Emitter || !Emitter->Render || !VAO )
			return;

		CRenderable::Draw( RenderData, PreviousRenderData, DrawModeOverride );

		/*const auto* FFT = CSoLoudSound::GetBusFFT( Bus::Music );
		std::memcpy( CurrentFFT, FFT, sizeof( float ) * 256 );

		for( size_t Index = 0; Index < 256; Index++ )
		{
			CurrentFFT[Index] = Math::Lerp( PreviousFFT[Index], CurrentFFT[Index], Math::Saturate( GameLayersInstance->GetFrameTime() * 6.0f ) );
		}
		
		std::memcpy( PreviousFFT, CurrentFFT, sizeof( float ) * 256 );
		
		const auto Frequencies = Uniform( Vector4D( CurrentFFT[5], CurrentFFT[64], CurrentFFT[96], CurrentFFT[128] ) );
		Frequencies.Bind( Emitter->Render->GetHandles().Program, "Frequencies" );*/

		// Emitter->Render->Activate();
		glBindVertexArray( VAO );
		glDrawArrays( GL_POINTS, 0, Buffer.Count() );
		glBindVertexArray( 0 );
	}

	ParticleEmitter* Emitter = nullptr;
	GLuint VAO = 0;
	ShaderStorageBuffer<Particle> Buffer;
	float CurrentFFT[256];
	float PreviousFFT[256];
};

ParticleEmitter::ParticleEmitter()
{
	
}

void ParticleEmitter::Initialize( CShader* ComputeIn, CShader* RenderIn, const size_t& CountIn )
{
	if( ComputeIn && RenderIn )
	{
		Compute = ComputeIn;
		Render = ComputeIn;
	}
	else
	{
		Compute = CAssets::Get().CreateNamedShader( "particle_generic_compute", "Shaders/Particle/GenericParticle", EShaderType::Compute );
		Render = CAssets::Get().CreateNamedShader( "particle_generic_render", "Shaders/Particle/GenericParticle", EShaderType::Geometry );
	}

	Count = CountIn > 0 ? CountIn : 1000;
	
	auto* Particles = new Particle[Count];
	for( size_t Index = 0; Index < Count; Index++ )
	{
		const auto X = Location.X + Math::RandomRange( -100.0f, 100.0f );
		const auto Y = Location.Y + Math::RandomRange( -100.0f, 100.0f );
		const auto Z = Location.Z + Math::RandomRange( -100.0f, 100.0f );
		
		Particles[Index].Position = glm::vec4( X, Y, Z, 1.0f );
		Particles[Index].PreviousPosition = Particles[Index].Position;
	}

	auto* Particle = new ParticleRenderable( this );
	Particle->Initialize( Particles, Count );
	Renderable = Particle;
	
	delete[] Particles;

	Time = GameLayersInstance->GetCurrentTime();
}

void ParticleEmitter::Destroy()
{
	delete Renderable;
}

void ParticleEmitter::Tick()
{
	if( Count == 0 || !Compute || !Render )
		return;

	auto& RenderData = Renderable->GetRenderData();
	RenderData.Transform = FTransform(
		Location,
		Vector3D::Zero,
		Vector3D::One
	);

	RenderData.Color = Vector4D( 1.0f, 1.0f, 1.0f, 1.0f );
	
	const auto Program = Compute->Activate();

	const auto* Particle = Cast<ParticleRenderable>( Renderable );
	Particle->Buffer.Bind();

	const auto Volume = CSoLoudSound::GetBusOutput( Bus::Music );
	const auto MaxVolume = Math::Max( Volume.Left, Volume.Right );

	Time += GameLayersInstance->GetDeltaTime();// +std::powf( MaxVolume, 7.0f ) * 3.0f;
	const auto X = 0.0f + sinf( Time * 0.4f ) * 500.0f;
	const auto Y = 0.0f + cosf( Time * 0.39f ) * 500.0f;
	const auto Z = 400.0f + cosf( Time * 0.11f ) * 200.0f;
	const auto W = sinf( Time * 13.0f ) * 0.5f + 0.5f;

	// UI::AddCircle( Vector3D( X, Y, Z ), 10.0f );

	const auto Attractor = Uniform( Vector4D( X, Y, Z, MaxVolume ) );
	Attractor.Bind( Program, "Attractor" );

	const auto ParticleCount = Uniform( Count );
	ParticleCount.Bind( Program, "ParticleCount" );

	Vector4D Time;
	Time.X = StaticCast<float>( GameLayersInstance->GetCurrentTime() );
	Time.Y = StaticCast<float>( GameLayersInstance->GetDeltaTime() );
	Time.Z = StaticCast<float>( GameLayersInstance->GetPreviousTime() );
	Time.W = StaticCast<float>( GameLayersInstance->GetTimeScale() );
	const auto TimeUniform = Uniform( Time );
	TimeUniform.Bind( Program, "Time" );
	
	glDispatchCompute( Count / WorkSize + 1, 1, 1 );
	glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT );
}

void ParticleEmitter::Frame()
{
	if( Count == 0 || !Compute || !Render )
		return;

	CWindow::Get().GetRenderer().QueueRenderable( Renderable );
}

void ParticleEmitter::SetShader( CShader* Compute, CShader* Render )
{
	if( !Compute || !Render )
		return;
	
	this->Compute = Compute;
	this->Render = Render;
}
