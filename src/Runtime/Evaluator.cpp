#include <Emux/Runtime/Evaluator.hpp>
#include <Emux/Compiler/SourceLocation.hpp>
#include <Emux/Compiler/Token.hpp>
#include <Emux/Core/Logger.hpp>

#include <cctype>
#include <charconv>

namespace Emux
{
	
Evaluator::Evaluator(RuntimeContext& context):
	m_Context(context)
{
}

Evaluator::~Evaluator()
{
}

void Evaluator::Evaluate()
{
	auto& program = *m_Context.AST;
	for (auto& child : program.Children) 
	{
		if (child->GetType() == NodeType::Section)
		{
			SectionNode* section = static_cast<SectionNode*>(child.get());
			EvaluateSection(*section);
		}
	}

	auto& Main = *program.FindSection("Main");

	FunctionCallNode entryCall(
		Token{
			.Type { TokenType::Identifier },
			.Location { Main.Name.Location }
		},
		{}
	);

	for (const auto& dependency : Main.Dependencies)
	{
		entryCall.Name.Text = dependency.Text + "::_Start";
		EvaluateFunctionCall(entryCall);
	}

	entryCall.Name.Text = "Main::_Start";
	EvaluateFunctionCall(entryCall);
}

void Evaluator::EvaluateSection(SectionNode& node)
{
	for (auto& child : node.Children)
	{
		if (
			auto function = dynamic_cast<FunctionNode*>(child.get());
			function != nullptr
		)
		{
			EvaluateFunctionDefinition(*function, node);
		} else if (
			auto functionCall = dynamic_cast<FunctionCallNode*>(child.get());
			functionCall != nullptr
		)
		{
			EvaluateFunctionCall(*functionCall);
		}
	}
}

void Evaluator::EvaluateExpression(Node& node)
{
	if (
		auto functionCall = dynamic_cast<FunctionCallNode*>(&node);
		functionCall != nullptr
	)
	{
		EvaluateFunctionCall(*functionCall);
	} else if (
		auto assign = dynamic_cast<AssignmentNode*>(&node);
		assign != nullptr
	)
	{
		EvaluateAssignment(*assign);
	}
}

void Evaluator::EvaluateFunctionCall(FunctionCallNode& node)
{
	std::string& funcName = node.Name.Text;

	auto it = m_Context.Functions.find(funcName);
	if (it == m_Context.Functions.end())
	{
		return;
	}

	FunctionNode& function = it->second.get(); 
	for (auto& child : function.Children)
	{
		EvaluateExpression(*child.get());
	}
}

void Evaluator::EvaluateVariableCall(Node& node)
{
}

void Evaluator::EvaluateAssignment(AssignmentNode& node)
{
	const auto& var = m_Context.Variables[node.Name.Text];
	auto& value = node.Value.Text;
	if (node.Value.Type == TokenType::Number)
	{
		std::uint64_t amount = 0;

	    auto result = std::from_chars(
	        value.data(),
	        value.data() + value.size(),
	        amount
	    );

	    if(result.ec != std::errc{} || amount == 0)
	    {
	        throw std::runtime_error("Invalid allocation");
	    }

		auto& memView = m_Context.Machine.GetMemoryView();
		memView.Write(var, amount);
	}
}

void Evaluator::EvaluateFunctionDefinition(FunctionNode& node, SectionNode& section)
{
	std::string name = section.Name.Text + "::" + node.Name.Text; 
	m_Context.Functions.emplace(name, std::ref(node));
}

}