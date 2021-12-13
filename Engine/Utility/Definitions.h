// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <set>
#include <vector>
#include <memory>

template<typename T>
using PointerShared = std::shared_ptr<T>;

template<typename T>
using PointerWeak = std::weak_ptr<T>;

template<typename T>
using MakeShared = std::make_shared<T>();
