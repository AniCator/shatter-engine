// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

namespace Bus
{
	enum Type
	{
		SFX,
		Dialogue,
		Music,
		UI,
		Auxilery3,
		Auxilery4,
		Auxilery5,
		Auxilery6,
		Auxilery7,
		Auxilery8,
		Maximum
	};

	static Type Master = Maximum;

	struct Volume
	{
		float Left = 0.0f;
		float Right = 0.0f;
	};
}
