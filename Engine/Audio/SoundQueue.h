// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>

namespace SoLoud
{
	class Queue;
}

struct SoundQueue
{
	SoundQueue();
	~SoundQueue();

	void Add( const std::string& ResourcePath );
	uint32_t Count() const;
	bool Playing() const;
protected:
	SoLoud::Queue* Instance = nullptr;
};
