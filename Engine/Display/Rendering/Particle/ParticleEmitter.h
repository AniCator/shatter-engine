// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Display/Rendering/StorageBuffer.h>
#include <Engine/Display/Rendering/Particle/Particle.h>
#include <Engine/Utility/Math/Vector.h>

#include <unordered_map>
#include <string>

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

class ParticleEmitter
{
public:
	ParticleEmitter();

	void Initialize( class CShader* Compute = nullptr, class CShader* Render = nullptr, const size_t& Count = 0 );
	void Destroy();
	
	void Tick();
	void Frame();

	// Starting location of the emitter.
	Vector3D Location = Vector3D::Zero;

	void SetShader( class CShader* Compute, class CShader* Render );
	
protected:
	class CShader* Compute = nullptr;
	class CShader* Render = nullptr;
	class CRenderable* Renderable = nullptr;

	size_t Count = 8192;
	double Time = 0.0;

	friend class ParticleRenderable;
};
