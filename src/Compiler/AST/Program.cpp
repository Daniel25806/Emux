#include <Emux/Compiler/AST/Program.hpp>

namespace Emux
{

bool Program::AddSection(
    std::unique_ptr<SectionNode> section
)
{
    std::string name = section->Name.Text;


    if(m_Sections.contains(name))
    {
        return false;
    }


    SectionNode* ptr = section.get();


    m_Sections.emplace(
        name,
        ptr
    );


    Children.push_back(
        std::move(section)
    );


    return true;
}

SectionNode* Program::FindSection(
    std::string_view name
)
{
    auto it = m_Sections.find(
        std::string(name)
    );


    if(it == m_Sections.end())
    {
        return nullptr;
    }


    return it->second;
}

const Sections& Program::GetSections()
{
    return m_Sections;
}

}