#pragma once

#include <Emux/Compiler/AST/Node.hpp>
#include <Emux/Compiler/Token.hpp>
#include <cstdint>

namespace Emux
{

class ReturnNode final : public Node
{

public:

    ReturnNode(
        const SourceLocation& location
    ):
        Node(
            NodeType::Variable,
            location
        )
    {
    }


    Token Type;

    size_t Size = 0;

    size_t Offset = 0;

};

}