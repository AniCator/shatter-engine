// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "ParticleEmitter.h"

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
		if( !Emitter && Emitter->Render && VAO )
			return;

		CRenderable::Draw( RenderData, PreviousRenderData, DrawModeOverride );

		// Emitter->Render->Activate();
		glBindVertexArray( VAO );
		glDrawArrays( GL_POINTS, 0, Buffer.Count() );
		glBindVertexArray( 0 );
	}

	ParticleEmitter* Emitter = nullptr;
	GLuint VAO = 0;
	ShaderStorageBuffer<Particle> Buffer;
};

ParticleEmitter::ParticleEmitter()
{
	
}

void ParticleEmitter::Initialize()
{
	Compute = CAssets::Get().CreateNamedShader( "particle_compute", "Shaders/Particle/Particle", EShaderType::Compute );
	Render = CAssets::Get().CreateNamedShader( "particle_render", "Shaders/Particle/Particle", EShaderType::Geometry );

	Count = 2000000;
	
	auto* Particles = new Particle[Count];
	for( size_t Index = 0; Index < Count; Index++ )
	{
		const auto X = Math::RandomRange( -100.0f, 100.0f );
		const auto Y = Math::RandomRange( -100.0f, 100.0f );
		const auto Z = Math::RandomRange( -100.0f, 100.0f );
		
		Particles[Index].Position = glm::vec4( X, Y, Z, 1.0f );
		Particles[Index].PreviousPosition = Particles[Index].Position;
	}

	auto* Particle = new ParticleRenderable( this );
	Particle->Initialize( Particles, Count );
	Renderable = Particle;
	
	delete[] Particles;
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
		Vector3D::Zero,
		Vector3D::Zero,
		Vector3D::One
	);

	RenderData.Color = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
	
	const auto Program = Compute->Activate();

	const auto* Particle = Cast<ParticleRenderable>( Renderable );
	Particle->Buffer.Bind();

	const auto X = 0.0f + sinf( GameLayersInstance->GetCurrentTime() * 0.4f ) * 500.0f;
	const auto Y = 0.0f + cosf( GameLayersInstance->GetCurrentTime() * 0.39f ) * 500.0f;
	const auto Z = 400.0f + cosf( GameLayersInstance->GetCurrentTime() * 0.11f ) * 200.0f;
	const auto W = sinf( GameLayersInstance->GetCurrentTime() * 13.0f ) * 0.5f + 0.5f;

	// UI::AddCircle( Vector3D( X, Y, Z ), 10.0f );

	const auto Attractor = Uniform( Vector4D( X, Y, Z, W ) );
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