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

		for( uint32_t Index = 0; Index < MaximumParticleTextures; Index++ )
		{
			SetTexture( Emitter->Texture[Index], static_cast<ETextureSlot>( Index ) );
		}

		// Calculate rough world bounds based on the initial positions.
		auto* Positions = new Vector3D[Count];
		for( size_t Index = 0; Index < Count; Index++ )
		{
			Positions[Index].X = Data[Index].Position.x;
			Positions[Index].Y = Data[Index].Position.y;
			Positions[Index].Z = Data[Index].Position.z;
		}

		RenderData.WorldBounds = Math::AABB( Positions, Count );
		delete[] Positions;

		Buffer.Initialize( Data, Count );
		
		glGenVertexArrays( 1, &VAO );
		glBindVertexArray( VAO );
		Buffer.Bind();
		
		glEnableVertexAttribArray( 0 );
		glEnableVertexAttribArray( 1 );
		glEnableVertexAttribArray( 2 );
		glEnableVertexAttribArray( 3 );
		glEnableVertexAttribArray( 4 );

		glBindBuffer( GL_ARRAY_BUFFER, Buffer.Handle() );
		glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, sizeof( Particle ), reinterpret_cast<void*>( offsetof( Particle, Position ) ) );
		// glVertexAttribDivisor( 0, 1 );
		glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, sizeof( Particle ), reinterpret_cast<void*>( offsetof( Particle, PreviousPosition ) ) );
		// glVertexAttribDivisor( 1, 1 );
		glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE, sizeof( Particle ), reinterpret_cast<void*>( offsetof( Particle, Parameters ) ) );
		// glVertexAttribDivisor( 2, 1 );
		glVertexAttribPointer( 3, 4, GL_FLOAT, GL_FALSE, sizeof( Particle ), reinterpret_cast<void*>( offsetof( Particle, ScratchpadA ) ) );
		// glVertexAttribDivisor( 3, 1 );
		glVertexAttribPointer( 4, 4, GL_FLOAT, GL_FALSE, sizeof( Particle ), reinterpret_cast<void*>( offsetof( Particle, ScratchpadB ) ) );
		// glVertexAttribDivisor( 4, 1 );

		glBindVertexArray( 0 );
	}
	
	void Draw( FRenderData& RenderDataIn, const CRenderable* PreviousRenderable, EDrawMode DrawModeOverride ) override
	{
		if( !Emitter || !Emitter->Render || !VAO )
			return;

		CRenderable::Draw( RenderDataIn, PreviousRenderable, DrawModeOverride );

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
	auto* ErrorTexture = CAssets::Get().FindTexture( "error" );
	for( uint32_t Index = 0; Index < MaximumParticleTextures; Index++ )
	{
		Texture[Index] = ErrorTexture;
	}
}

void ParticleEmitter::Initialize( const ParticleSystem& System )
{
	Initialize( &System );
}

void ParticleEmitter::Initialize( const ParticleSystem* System )
{
	if( !System )
	{
		Initialize( nullptr, nullptr );
		return;
	}

	for( uint32_t Index = 0; Index < MaximumParticleTextures; Index++ )
	{
		Texture[Index] = System->Texture[Index];
	}

	Initialize( System->Compute, System->Render, System->Count );
}

void ParticleEmitter::Initialize( CShader* ComputeIn, CShader* RenderIn, const size_t& CountIn )
{
	if( ComputeIn && RenderIn )
	{
		Compute = ComputeIn;
		Render = RenderIn;
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
		const auto X = Location.X + Math::RandomRange( -10.0f, 10.0f );
		const auto Y = Location.Y + Math::RandomRange( -10.0f, 10.0f );
		const auto Z = Location.Z + Math::RandomRange( -10.0f, 10.0f );
		
		Particles[Index].Position = glm::vec4( X, Y, Z, 1.0f );
		Particles[Index].PreviousPosition = Particles[Index].Position;
		Particles[Index].Parameters = glm::vec4( -1.0f ); // Initialize with -1.0 to indicate none of the values have been set.
		Particles[Index].ScratchpadA = glm::vec4( -1.0f ); // Initialize with -1.0 to indicate none of the values have been set.
		Particles[Index].ScratchpadB = glm::vec4( -1.0f ); // Initialize with -1.0 to indicate none of the values have been set.
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

	const auto Volume = SoLoudSound::GetBusOutput( Bus::Music );
	const auto MaxVolume = Math::Max( Volume.Left, Volume.Right );

	Time += GameLayersInstance->GetDeltaTime();// +std::powf( MaxVolume, 7.0f ) * 3.0f;
	// UI::AddCircle( Vector3D( Location.X, Location.Y, Location.Z ), 10.0f );
	// UI::AddAABB( RenderData.WorldBounds.Minimum, RenderData.WorldBounds.Maximum, Color::Green );

	for( uint32_t Index = 0; Index < TotalControlPoints; Index++ )
	{
		const auto Point = Index == 0 ? Location : ControlPoints[Index - 1];
		auto ControlPoint = Uniform( Point );
		ControlPoint.Bind( Program, "ControlPoint" + std::to_string( Index ) );
	}

	auto ParticleCount = Uniform( Count );
	ParticleCount.Bind( Program, "ParticleCount" );

	Vector4D Time;
	Time.X = StaticCast<float>( GameLayersInstance->GetCurrentTime() );
	Time.Y = StaticCast<float>( GameLayersInstance->GetDeltaTime() );
	Time.Z = StaticCast<float>( GameLayersInstance->GetPreviousTime() );
	Time.W = StaticCast<float>( GameLayersInstance->GetTimeScale() );
	auto TimeUniform = Uniform( Time );
	TimeUniform.Bind( Program, "Time" );
	
	glDispatchCompute( Count / WorkSize + 1, 1, 1 );
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

void ParticleEmitter::SetBounds( const BoundingBox& Bounds )
{
	if( !Renderable )
		return;

	Renderable->GetRenderData().WorldBounds = Bounds;
}
