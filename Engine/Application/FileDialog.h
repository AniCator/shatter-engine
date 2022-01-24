// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <unordered_map>

typedef std::unordered_map<std::wstring, std::wstring> DialogFormats;
std::string OpenFileDialog( const DialogFormats& Formats );
