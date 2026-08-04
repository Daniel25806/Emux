#pragma once

#include <unordered_map>
#include <memory>
#include <string>

#include <Emux/Runtime/Memory.hpp>
#include <Emux/Runtime/MemoryView.hpp>
#include <Emux/Runtime/MachineState.hpp>

#include <Emux/Compiler/Memory/MemoryLayout.hpp>
#include <Emux/Compiler/AST/SectionNode.hpp>
#include <Emux/Compiler/AST/VariableNode.hpp>


namespace Emux
{

class Machine
{
public:

    Machine() = default;


    bool Initialize(
        const MemoryLayout& layout
    );


    void Shutdown();

    MachineState State() const;

    Memory& GetMemory();
    MemoryView& GetMemoryView();

private:

    Memory m_Memory;
    std::unique_ptr<MemoryView> m_View;

    MachineState m_State = MachineState::Created;
};

}