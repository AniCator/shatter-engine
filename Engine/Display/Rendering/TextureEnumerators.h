// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <type_traits>
#include <stdint.h>

enum class ETextureSlot : uint32_t
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

static const uint32_t TextureSlots = static_cast<ETextureSlotType>( ETextureSlot::Maximum );
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

enum class EFilteringMode : uint32_t
{
	Nearest = 0,
	Linear,

	Maximum
};

typedef std::underlying_type<EFilteringMode>::type EFilteringModeType;

enum class EImageFormat : uint32_t
{
	Unknown = 0,

	R8,
	RG8,
	RGB8,
	RGBA8,

	R16,
	RG16,
	RGB16,
	RGBA16,

	R16F,
	RG16F,
	RGB16F,
	RGBA16F,

	R32F,
	RG32F,
	RGB32F,
	RGBA32F,

	Maximum
};

typedef std::underlying_type<EImageFormat>::type EImageFormatType;
