// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>

namespace SoLoud
{
	class Queue;
}

struct SoundQueue
{
	void Construct();
	void Destroy();

	void Add( const class CSound* Sound );
	uint32_t Count() const;
	bool Playing() const;
protected:
	SoLoud::Queue* Instance = nullptr;

	friend class SoLoudSound;
};
