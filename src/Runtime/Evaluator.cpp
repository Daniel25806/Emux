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
		const NodeType type = child->GetType();
		if (type == NodeType::Function)
		{
			auto function = static_cast<FunctionNode*>(child.get());
			EvaluateFunctionDefinition(*function, node);
		} else if (type == NodeType::FunctionCall)
		{
			auto functionCall = static_cast<FunctionCallNode*>(child.get());
			EvaluateFunctionCall(*functionCall);
		}
	}
}

void Evaluator::EvaluateExpression(Node& node)
{
	const NodeType type = node.GetType();

	if (type == NodeType::FunctionCall)
	{
		auto functionCall = static_cast<FunctionCallNode*>(&node);
		EvaluateFunctionCall(*functionCall);
	} else if (type == NodeType::Assign)
	{
		auto assign = static_cast<AssignmentNode*>(&node);
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
		if (child->GetType() == NodeType::Return) return;
		
		EvaluateExpression(*child.get());
	}
}

void Evaluator::EvaluateVariableCall(VariableCallNode& node, uint8_t* buffer)
{
	MemoryView& mem = m_Context.Machine.GetMemoryView();


	auto it = m_Context.Variables.find(node.Name.Text);
	if (it == m_Context.Variables.end())
	{
		return;
	}

	MemoryVariable& var = it->second;
	uint8_t* data = mem.GetBuffer(var);

	if (var.Bits <= 8)
	{
		for (std::size_t i = 0; i < var.Size; i++) {
			buffer[i] = data[i];
		}
	} else if (var.Bits <= 16)
	{
		for (std::size_t i = 0; i < var.Size; i++) {
			uint16_t* cbuffer = reinterpret_cast<uint16_t*>(buffer);
			cbuffer[i] = reinterpret_cast<uint16_t*>(data)[i];
		}
	} else if (var.Bits <= 32)
	{
		for (std::size_t i = 0; i < var.Size; i++) {
			uint32_t* cbuffer = reinterpret_cast<uint32_t*>(buffer);
			cbuffer[i] = reinterpret_cast<uint32_t*>(data)[i];
		}
	} else if (var.Bits <= 64)
	{
		for (std::size_t i = 0; i < var.Size; i++) {
			uint64_t* cbuffer = reinterpret_cast<uint64_t*>(buffer);
			cbuffer[i] = reinterpret_cast<uint64_t*>(data)[i];
		}
	} else {
		throw std::runtime_error("Overwrite detected");
	}
}

void Evaluator::EvaluateAssignment(AssignmentNode& node)
{
	Node* valueNode = node.Children[0].get();
	
	if (!valueNode)
	{
		throw std::runtime_error("Corruption detected");
	}
	auto& memView = m_Context.Machine.GetMemoryView();

	NodeType type = valueNode->GetType();
	const auto& var = m_Context.Variables[node.Name.Text];
	
	if (type == NodeType::Literal) {
		auto literalNode = static_cast<LiteralNode*>(valueNode);

		if (!literalNode)
		{
			throw std::runtime_error("Corruption detected");
		}

		auto& value = literalNode->Value;
		auto& valueText = value.Text;
		

		if (value.Type == TokenType::Number)
		{
			std::uint64_t amount = 0;

		    auto result = std::from_chars(
		        valueText.data(),
		        valueText.data() + valueText.size(),
		        amount
		    );

		    if(result.ec != std::errc{})
		    {
		        throw std::runtime_error("Invalid allocation");
		    }

			memView.Write(var, amount);

		} else if (value.Type == TokenType::String) {
			uint8_t* buffer = memView.GetBuffer(var);
			for (std::size_t i = 0; i < value.Location.Length; ++i)
			{
				if (i >= valueText.size()) break;
				buffer[i] = static_cast<uint8_t>(valueText[i]);
			}
		}
	} else if (type == NodeType::VariableCall)
	{
		auto varCallNode = static_cast<VariableCallNode*>(valueNode);
		
		if (!varCallNode)
		{
			throw std::runtime_error("Corruption detected");
		}

		uint8_t* buffer = memView.GetBuffer(var);

		EvaluateVariableCall(*varCallNode, buffer);
	}
}

void Evaluator::EvaluateFunctionDefinition(FunctionNode& node, SectionNode& section)
{
	std::string name = section.Name.Text + "::" + node.Name.Text; 
	m_Context.Functions.emplace(name, std::ref(node));
}

}