#pragma once
#include <Emux/Compiler/AST/Program.hpp>
#include <Emux/Compiler/AST/FunctionNode.hpp>
#include <Emux/Compiler/Memory/MemoryVariable.hpp>
#include <Emux/Runtime/Machine.hpp>

#include <memory>
#include <unordered_map>
#include <functional>

namespace Emux
{
	
class RuntimeContext
{
public:
	Machine Machine;
	std::unique_ptr<Program> AST;
	std::unordered_map<std::string, MemoryVariable> Variables;
	std::unordered_map<std::string, std::reference_wrapper<FunctionNode>> Functions;
};

}