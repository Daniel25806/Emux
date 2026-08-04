#pragma once

#include <Emux/Compiler/AST/Node.hpp>
#include <Emux/Compiler/Token.hpp>
#include <cstdint>

namespace Emux
{

enum class BinaryOperation
{
    Add,
    Sub,
    Mul,
    Div,
    And,
    Or,
    Xor,
    Not,
    Lshift,
    Rshift
};

class BinaryNode final : public Node
{
public:
    BinaryNode(
        const BinaryOperation& operation,
        const SourceLocation& location
    ):
        Node(
            NodeType::Binary,
            {},
            location
        ),
        Operation(operation)
    {
    }

    BinaryNode(
        const SourceLocation& location
    ):
        Node(
            NodeType::Binary,
            {},
            location
        )
    {
    }

    BinaryOperation Operation = BinaryOperation::Add;
};

}