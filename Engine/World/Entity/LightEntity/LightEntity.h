// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/Display/Rendering/Light/Light.h>
#include <Engine/Display/Rendering/StorageBuffer.h>

class LightEntity : public CPointEntity
{
public:
	void Construct() override;
	void Tick() override;
	void Frame() override;
	void Destroy() override;

	void Reload() override;
	void Load( const JSON::Vector& Objects ) override;
	void Import( CData& Data ) override;
	void Export( CData& Data ) override;

	void Debug() override;

	static void Initialize();
	static void UploadToGPU();
	static void Bind();

	// Fetch nearby lights.
	static LightIndices Fetch( const Vector3D& Position );
	static const Light& Get( int32_t Index );
	
protected:
	Light Information;
	int32_t LightIndex = -1;

	static int32_t AllocateLight();

	static constexpr int32_t LightMaximum = 64;
	static Light Lights[LightMaximum];
	static int32_t AllocationIndex;

	static ShaderStorageBuffer<Light> LightBuffer;
};
