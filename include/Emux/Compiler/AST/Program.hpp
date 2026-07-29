
#pragma once

#include <unordered_map>
#include <vector>

#include <Emux/Compiler/AST/Node.hpp>
#include <Emux/Compiler/AST/NodePtr.hpp>
#include <Emux/Compiler/AST/SectionNode.hpp>

namespace Emux
{

using Sections = std::unordered_map<std::string, SectionNode*>;

class Program final : public Node
{
public:

    Program()
        :
        Node(
            NodeType::Program,
            {}
        )
    {
    }


    bool AddSection(
        std::unique_ptr<SectionNode> section
    );


    SectionNode* FindSection(
        std::string_view name
    );

    const Sections& GetSections();


    std::vector<NodePtr> Children;


private:

    Sections m_Sections;

};

}