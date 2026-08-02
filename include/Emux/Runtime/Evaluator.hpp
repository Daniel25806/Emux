#pragma once
#include <Emux/Runtime/RuntimeContext.hpp>
#include <Emux/Compiler/AST/Node.hpp>
#include <Emux/Compiler/AST/VariableNode.hpp>
#include <Emux/Compiler/AST/VariableCallNode.hpp>
#include <Emux/Compiler/AST/SectionNode.hpp>
#include <Emux/Compiler/AST/FunctionNode.hpp>
#include <Emux/Compiler/AST/FunctionCallNode.hpp>
#include <Emux/Compiler/AST/AssignmentNode.hpp>
#include <Emux/Compiler/AST/LiteralNode.hpp>
#include <Emux/Compiler/AST/ReturnNode.hpp>
#include <Emux/Compiler/AST/BinaryNode.hpp>

namespace Emux
{
	
class Evaluator
{
public:
	Evaluator(RuntimeContext& context);
	~Evaluator();

	void Evaluate();
private:
	void EvaluateSection(SectionNode& node);
	void EvaluateExpression(Node& node);
	void EvaluateFunctionCall(FunctionCallNode& node);
	void EvaluateVariableCall(VariableCallNode& node, uint8_t* buffer);
	void EvaluateAssignment(AssignmentNode& node);
	void EvaluateFunctionDefinition(FunctionNode& node, SectionNode& section);
	void EvaluateBinary(BinaryNode& node, uint8_t* result, size_t bits);
	void HelperSetValue(uint64_t value, uint8_t* result, size_t bits);
	uint64_t HelperGetValue(Node& n);

private:
	RuntimeContext& m_Context;
};

}