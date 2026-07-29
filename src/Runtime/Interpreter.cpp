#include <Emux/Runtime/Interpreter.hpp>
#include <Emux/Compiler/AST/VariableNode.hpp>
#include <Emux/Compiler/AST/SectionNode.hpp>
#include <Emux/Compiler/AST/Program.hpp>
#include <Emux/Compiler/Memory/MemoryLayout.hpp>
#include <vector>

namespace Emux
{

Interpreter::Interpreter(RuntimeContext& context):
	m_Context(context),
    m_Evaluator(context)
{
	MemoryLayout layout;
    layout.Build(
        *context.AST
    );

    context.Machine.Initialize(layout);
    for (const auto& var : layout.GetVariables())
    {
        context.Variables.emplace(var.Name, var);
    }

    Program& program = *context.AST;
    SectionNode* varsSection = context.AST->FindSection("Vars");
    if (varsSection){
        std::erase_if(program.Children, [&varsSection](auto& child){
            return dynamic_cast<SectionNode*>(child.get()) == varsSection;
        });
    }
}

Interpreter::~Interpreter()
{
}

void Interpreter::Interprete()
{
	m_Evaluator.Evaluate();
}

};