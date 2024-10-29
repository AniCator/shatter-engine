// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <vector>

#include <Engine/Resource/Asset.h>
#include <Engine/Utility/Structures/JSON.h>

#include <ThirdParty/glm/glm.hpp>

struct Particle
{
	glm::vec4 Position = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	glm::vec4 PreviousPosition = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	glm::vec4 Parameters = glm::vec4( -1.0f );
	glm::vec4 ScratchpadA = glm::vec4( -1.0f );
	glm::vec4 ScratchpadB = glm::vec4( -1.0f );
};

constexpr uint32_t MaximumParticleTextures = 8;

struct ParticleSystem
{
	void Load();
	void Configure( const JSON::Vector& Objects );

	std::string Name;
	std::string ComputePath;
	std::string RenderPath;

	std::string TexturePath[MaximumParticleTextures];

	class CShader* Compute = nullptr;
	class CShader* Render = nullptr;
	class CTexture* Texture[MaximumParticleTextures]{};
	uint32_t Count = 8192;

	static std::vector<ParticleSystem> LoadDefinitions( const std::string& Location );

	static void RegisterAssetLoader();
};

class ParticleAsset : public Asset
{
public:
	~ParticleAsset() override = default;

	const std::string& GetType() const override
	{
		static const std::string Type = "particle_system";
		return Type;
	}

	ParticleSystem System;
};
