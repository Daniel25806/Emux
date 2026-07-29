#pragma once

#include <vector>

#include <Emux/Compiler/Memory/MemoryVariable.hpp>
#include <Emux/Compiler/Memory/OptionalMemoryVariable.hpp>
#include <Emux/Compiler/AST/Program.hpp>

namespace Emux
{

class Interpreter;

class MemoryLayout
{
public:

    void Build(
        const Program& program
    );


    OptionalMemoryVariable Find(
        std::string_view name
    ) const;

    std::size_t GetSize() const;
    const std::vector<MemoryVariable>& GetVariables() const;

    void Dump() const;

private:

    std::vector<MemoryVariable> m_Variables;

    std::size_t m_TotalSize = 0;
};

}