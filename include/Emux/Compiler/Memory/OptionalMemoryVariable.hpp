#pragma once
#include <optional>
#include <functional>

namespace Emux
{
	using MemoryVariableRef = std::reference_wrapper<const MemoryVariable>;
	using OptionalMemoryVariable = std::optional<MemoryVariableRef>;

}