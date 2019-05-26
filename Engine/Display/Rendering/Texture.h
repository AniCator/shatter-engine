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

static const GLenum EnumToSlot[ETextureSlot::Maximum]
{
	GL_TEXTURE0,
	GL_TEXTURE1,
	GL_TEXTURE2,
	GL_TEXTURE3,
	GL_TEXTURE4,
	GL_TEXTURE5,
	GL_TEXTURE6,
	GL_TEXTURE7,
	GL_TEXTURE8,
	GL_TEXTURE9,
	GL_TEXTURE10,
	GL_TEXTURE11,
	GL_TEXTURE12,
	GL_TEXTURE13,
	GL_TEXTURE14,
	GL_TEXTURE15,
	GL_TEXTURE16,
	GL_TEXTURE17,
	GL_TEXTURE18,
	GL_TEXTURE19,
	GL_TEXTURE20,
	GL_TEXTURE21,
	GL_TEXTURE22,
	GL_TEXTURE23,
	GL_TEXTURE24,
	GL_TEXTURE25,
	GL_TEXTURE26,
	GL_TEXTURE27,
	GL_TEXTURE28,
	GL_TEXTURE29,
	GL_TEXTURE30,
	GL_TEXTURE31
};

class CTexture
{
public:
	CTexture();
	CTexture( const char* FileLocation );
	~CTexture();

	bool Load();
	void Bind( ETextureSlot Slot );

	const int GetWidth() const;
	const int GetHeight() const;

protected:
	GLuint Handle;
	std::string Location;

	int Width;
	int Height;
	int Channels;

	unsigned char* ImageData;
};
