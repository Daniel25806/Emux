#include <Emux/Compiler/Memory/MemoryLayout.hpp>
#include <Emux/Compiler/Semantic/TypeParser.hpp>
#include <Emux/Compiler/AST/SectionNode.hpp>
#include <Emux/Compiler/AST/VariableNode.hpp>
#include <cstdint>
#include <iostream>

namespace Emux
{

void MemoryLayout::Build(
    const Program& program
)
{
    m_TotalSize = 0;

    m_Variables.clear();


    for(auto& child : program.Children)
    {
        auto* section =
            dynamic_cast<SectionNode*>(
                child.get()
            );


        if(!section)
            continue;


        if(section->Name.Text != "Vars")
            continue;



        for(auto& variableNode :
            section->Children)
        {

            auto* variable =
                dynamic_cast<VariableNode*>(
                    variableNode.get()
                );


            if(!variable)
                continue;

            auto name = variable->Name.Text;

            auto type = TypeParser::Parse(variable->Type.Text);


            if(!type)
                continue;



            std::size_t bytes = (type->Bits + 7) / 8;
			//std::size_t alignment = bytes;

			//m_TotalSize = Align(m_TotalSize, alignment);


			m_Variables.push_back(MemoryVariable{
			    name,
			    m_TotalSize,
			    bytes,
                type->Bits
			});


			m_TotalSize += bytes;
        }
    }
}

OptionalMemoryVariable MemoryLayout::Find(std::string_view name) const
{
	for (auto& var : m_Variables)
	{
		if (var.Name == name) return var;
	}
	return std::nullopt;
}

std::size_t MemoryLayout::GetSize() const
{
	return m_TotalSize;
}

const std::vector<MemoryVariable>& MemoryLayout::GetVariables() const
{
    return m_Variables;
}

void MemoryLayout::Dump() const
{
    for(const auto& variable : m_Variables)
    {
        std::cout
            << variable.Name
            << "\nOffset: "
            << variable.Offset
            << "\nSize: "
            << variable.Size
            << "\n\n";
    }


    std::cout
        << "Total: "
        << m_TotalSize
        << '\n';
}

}