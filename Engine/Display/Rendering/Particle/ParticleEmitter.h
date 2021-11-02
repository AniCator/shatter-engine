// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Display/Rendering/StorageBuffer.h>
#include <Engine/Display/Rendering/Particle/Particle.h>
#include <Engine/Utility/Math/Vector.h>

#include <unordered_map>
#include <string>

struct BoundingBox;

// TODO: Add support for slotting in snippets like modules.
// A snippet contains header and code information that can be injected into a file.
struct ParticleSnippet
{
	std::vector<std::string> Header;
	std::string Code;
};

struct ParticleInformation
{
	std::unordered_map<std::string, ParticleSnippet> Snippets;
};

// Maximum amount of particle control points.
constexpr uint32_t MaximumControlPoints = 9;

// All control points including the source location.
constexpr uint32_t TotalControlPoints = MaximumControlPoints + 1;

class ParticleEmitter
{
public:
	ParticleEmitter();

	void Initialize( const struct ParticleSystem& System );
	void Initialize( const struct ParticleSystem* System = nullptr );
	void Initialize( class CShader* Compute = nullptr, class CShader* Render = nullptr, const size_t& Count = 0 );
	void Destroy();
	
	void Tick();
	void Frame();

	// Starting location of the emitter.
	Vector3D Location = Vector3D::Zero;

	// Additional control points.
	Vector3D ControlPoints[MaximumControlPoints]{};

	void SetShader( class CShader* Compute, class CShader* Render );
	void SetBounds( const BoundingBox& Bounds );
	
protected:
	class CShader* Compute = nullptr;
	class CShader* Render = nullptr;
	class CTexture* Texture[MaximumParticleTextures]{};
	class CRenderable* Renderable = nullptr;

	uint32_t Count = 8192;
	double Time = 0.0;

	friend class ParticleRenderable;
};
