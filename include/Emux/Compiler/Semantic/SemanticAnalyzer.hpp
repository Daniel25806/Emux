#pragma once

#include <unordered_set>
#include <unordered_map>

#include <Emux/Compiler/CompilerContext.hpp>
#include <Emux/Compiler/AST/SectionNode.hpp>
#include <Emux/Compiler/AST/VariableNode.hpp>
#include <Emux/Compiler/AST/FunctionNode.hpp>

namespace Emux
{

enum class VisitState
{
    NotVisited,
    Visiting,
    Visited
};

class SemanticAnalyzer
{
public:

    explicit SemanticAnalyzer(
        CompilerContext& context
    );


    void Analyze();


private:

    void AnalyzeSection(
        SectionNode& section
    );    

    void AnalyzeDependencies(
        SectionNode& section
    );

    void AnalyzeDependence(
        SectionNode& section,
        const Token& dependency,
        std::string_view rootName
    );

    void AnalyzeVariable(
        VariableNode& variable
    );

    void AnalyzeFunction(
        FunctionNode& function,
        SectionNode& section
    );

    bool IsVisited(std::string_view name);
    bool IsVisiting(std::string_view name);
    bool IsVisitedOrVisiting(std::string_view name);


private:

    CompilerContext& m_Context;

    std::unordered_set<std::string> m_Variables;
    std::unordered_set<std::string> m_Functions;
    std::unordered_map<std::string_view, VisitState> m_VisitStates;
};

}