#pragma once
#include <Emux/Runtime/RuntimeContext.hpp>
#include <Emux/Runtime/Evaluator.hpp>

namespace Emux
{

class Interpreter
{
public:
	Interpreter(RuntimeContext& context);
	~Interpreter();

	void Interprete();
	
private:
	RuntimeContext& m_Context;
	Evaluator m_Evaluator;
};	
	
}