#pragma once

#include <vector>

#include <Emux/Compiler/AST/Node.hpp>
#include <Emux/Compiler/AST/NodePtr.hpp>
#include <Emux/Compiler/Token.hpp>

namespace Emux
{

class SectionNode final : public Node
{
public:

    SectionNode(
        const SourceLocation& location
    )
        :
        Node(
            NodeType::Section,
            location
        )
    {
    }

    std::vector<Token> Dependencies;
};

}