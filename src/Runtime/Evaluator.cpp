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
		auto& functionCall = static_cast<FunctionCallNode&>(node);
		EvaluateFunctionCall(functionCall);
	} else if (type == NodeType::Assign)
	{
		auto& assign = static_cast<AssignmentNode&>(node);
		EvaluateAssignment(assign);
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
		if (child->GetType() == NodeType::Return) break;
			
		EvaluateExpression(*child);
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
	auto it = m_Context.Variables.find(node.Name.Text);

	if (it == m_Context.Variables.end())
	{
		throw std::runtime_error("SIGSEV");
		return;
	}

	const MemoryVariable var = it->second;
	
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
		        throw std::runtime_error("Invalid write");
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
	} else if (type == NodeType::Assign) {
		auto assignNode = static_cast<AssignmentNode*>(valueNode);
		EvaluateAssignment(*assignNode);

		auto varCallNode = VariableCallNode(
			assignNode->Name,
			assignNode->GetLocation()
		);

		uint8_t* buffer = memView.GetBuffer(var);

		EvaluateVariableCall(varCallNode, buffer);
	} else if (type == NodeType::Binary) {
		auto binNode = static_cast<BinaryNode*>(valueNode);

		uint8_t* buffer = memView.GetBuffer(var);
		EvaluateBinary(*binNode, buffer, var.Bits);
		
	} else {
		auto assignNode = static_cast<AssignmentNode*>(valueNode);
		EvaluateExpression(*valueNode);

		auto varCallNode = VariableCallNode(
			assignNode->Name,
			assignNode->GetLocation()
		);

		uint8_t* buffer = memView.GetBuffer(var);

		EvaluateVariableCall(varCallNode, buffer);
	}
}

void Evaluator::EvaluateFunctionDefinition(FunctionNode& node, SectionNode& section)
{
	std::string name = section.Name.Text + "::" + node.Name.Text; 
	m_Context.Functions.emplace(name, std::ref(node));
}

void Evaluator::EvaluateBinary(BinaryNode& node, uint8_t* result, size_t bits)
{
	if (node.Children.size() < 2)
	{
		throw std::runtime_error("Invalid binary operation");
		return;
	}

	Node* n0 = node.Children[0].get();

	if (!n0)
	{
		throw std::runtime_error("Invalid binary operation");
		return;
	}

	uint64_t v0 = HelperGetValue(*n0);
	uint64_t value = 0;

	if (node.Operation == BinaryOperation::Not)
	{
		value = ~v0;
		HelperSetValue(value, result, bits);
		return;
	}

	Node* n1 = node.Children[1].get();

	if (!n1)
	{
		throw std::runtime_error("Invalid binary operation");
		return;
	}

	uint64_t v1 = HelperGetValue(*n1);

	if (node.Operation == BinaryOperation::Add)
	{
		value = v0 + v1;
	} else if (node.Operation == BinaryOperation::Sub)
	{
		value = v0 - v1;
	} else if (node.Operation == BinaryOperation::Mul)
	{
		value = v0 * v1;
	} else if (node.Operation == BinaryOperation::Div)
	{
		value = v0 / v1;
	} else if (node.Operation == BinaryOperation::And)
	{
		value = v0 & v1;
	} else if (node.Operation == BinaryOperation::Or)
	{
		value = v0 | v1;
	} else if (node.Operation == BinaryOperation::Xor)
	{
		value = v0 ^ v1;
	} else if (node.Operation == BinaryOperation::Lshift)
	{
		value = v0 << v1;
	} else if (node.Operation == BinaryOperation::Rshift)
	{
		value = v0 >> v1;
	}

	HelperSetValue(value, result, bits);
}

void Evaluator::HelperSetValue(uint64_t value, uint8_t* result, size_t bits)
{
	if(bits < 64)
    {
        uint64_t mask = (1ULL << bits) - 1;

        value &= mask;
    }

    size_t bytes = (bits + 7)/8;
    for(size_t i = 0; i < bytes; i++)
    {
        result[i] = static_cast<uint8_t>(
            value >> (i * 8)
        );
    }
}

uint64_t Evaluator::HelperGetValue(Node& n)
{
	NodeType type = n.GetType(); 

	if (type == NodeType::Literal) 
	{
		auto& literalNode = static_cast<LiteralNode&>(n);

		auto& value = literalNode.Value;
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
		        throw std::runtime_error("SIGSEV");
		        return 0;
		    }

			return amount;
		} else {
			throw std::runtime_error("Undefined Behaviour");
			return;
		}
	} else if (type == NodeType::VariableCall)
	{
		auto& varCallNode = static_cast<VariableCallNode&>(n);
		const MemoryVariable& var = m_Context.Variables[varCallNode.Name.Text];

		MemoryView& memView = m_Context.Machine.GetMemoryView();
		return memView.Read(var);
	} else if (type == NodeType::Binary)
	{
		auto& binNode = static_cast<BinaryNode&>(n);
		uint64_t result;

		EvaluateBinary(binNode, reinterpret_cast<uint8_t*>(&result), 64);

		return result;
	}
	return 0;
}

}