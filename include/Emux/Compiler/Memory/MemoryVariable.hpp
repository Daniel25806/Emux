#pragma once

#include <cstddef>
#include <string>

namespace Emux
{

struct MemoryVariable
{
    std::string Name;
    std::size_t Offset = 0;
    std::size_t Size = 0;
    std::size_t Bits = 0;

    //std::size_t Alignment;
};

}