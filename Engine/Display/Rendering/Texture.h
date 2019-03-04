// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <stdint.h>

#include <ThirdParty/glad/include/glad/glad.h>

#include <Engine/Utility/Data.h>

enum class ETextureSlot : uint16_t
{
	Slot0 = 0,
	Slot1,
	Slot2,
	Slot3,
	Slot4,
	Slot5,
	Slot6,
	Slot7,
	Slot8,
	Slot9,
	Slot10,
	Slot11,
	Slot12,
	Slot13,
	Slot14,
	Slot15,
	Slot16,
	Slot17,
	Slot18,
	Slot19,
	Slot20,
	Slot21,
	Slot22,
	Slot23,
	Slot24,
	Slot25,
	Slot26,
	Slot27,
	Slot28,
	Slot29,
	Slot30,
	Slot31,

	Maximum
};

typedef std::underlying_type<ETextureSlot>::type ETextureSlotType;

static const uint16_t TextureSlots = static_cast<ETextureSlotType>( ETextureSlot::Maximum );
static const char* TextureSlotName[TextureSlots] = {
	"Slot0",
	"Slot1",
	"Slot2",
	"Slot3",
	"Slot4",
	"Slot5",
	"Slot6",
	"Slot7",
	"Slot8",
	"Slot9",
	"Slot10",
	"Slot11",
	"Slot12",
	"Slot13",
	"Slot14",
	"Slot15",
	"Slot16",
	"Slot17",
	"Slot18",
	"Slot19",
	"Slot20",
	"Slot21",
	"Slot22",
	"Slot23",
	"Slot24",
	"Slot25",
	"Slot26",
	"Slot27",
	"Slot28",
	"Slot29",
	"Slot30",
	"Slot31"
};

class CTexture
{
public:
	CTexture( const char* FileLocation );
	CTexture( unsigned char* Pixels, int TextureWidth, int TextureHeight );
	~CTexture();

	bool Load();
	void Bind( ETextureSlot Slot );

private:
	GLuint Handle;
	std::string Location;

	int Width;
	int Height;
	int Channels;

	unsigned char* ImageData;
};
