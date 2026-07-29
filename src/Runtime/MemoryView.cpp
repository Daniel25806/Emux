#include <Emux/Runtime/MemoryView.hpp>
#include <Emux/Core/Logger.hpp>

namespace Emux
{

MemoryView::MemoryView(Memory& memory): m_Memory(memory)
{
}

uint64_t MemoryView::Read(const MemoryVariable& variable) const
{
	if(variable.Bits > 64)
	{
		return 0;
	}

    uint64_t value = 0;

    auto* source = m_Memory.Data() + variable.Offset;

    for(size_t i = 0; i < variable.Size; i++)
    {
        value |= static_cast<uint64_t>(
			source[i]
        ) << (i * 8);
    }


    if(variable.Bits < 64)
    {
        uint64_t mask =
            (1ULL << variable.Bits) - 1;


        value &= mask;
    }


    return value;
}

void MemoryView::Write(
    const MemoryVariable& variable,
    uint64_t value
)
{
	if(variable.Bits < 64)
    {
        uint64_t mask = (1ULL << variable.Bits) - 1;

        value &= mask;
    }


    auto* destination = m_Memory.Data() + variable.Offset;


    for(size_t i = 0; i < variable.Size; i++)
    {
        destination[i] = static_cast<uint8_t>(
            value >> (i * 8)
        );
    }
}

uint8_t* MemoryView::GetBuffer(const MemoryVariable& variable)
{
	return m_Memory.Data() + variable.Offset;
}

}