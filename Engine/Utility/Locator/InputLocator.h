// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Locator/Locator.h>
#include <Engine/Input/InputInterface.h>
#include <Engine/Input/NullInput.h>

class CInputLocator : public TLocator<IInput, CNullInput>
{

};
