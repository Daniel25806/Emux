#pragma once

#include <cstdint>

#include <Emux/Runtime/Memory.hpp>
#include <Emux/Compiler/Memory/MemoryVariable.hpp>


namespace Emux
{

class MemoryView
{

public:

    MemoryView(Memory& memory);

    template<typename T>
    T& Get(
        const MemoryVariable& variable
    )
    {
        return *reinterpret_cast<T*>(
            m_Memory.Data()
            +
            variable.Offset
        );
    }

    uint64_t Read(
        const MemoryVariable& variable
    ) const;


    void Write(
        const MemoryVariable& variable,
        uint64_t value
    );


    uint8_t* GetBuffer(
        const MemoryVariable& variable
    );


private:

    Memory& m_Memory;

};

}