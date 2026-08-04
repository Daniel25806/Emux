#pragma once

#include <Emux/CLI/Arguments.hpp>
#include <Emux/Compiler/Diagnostics.hpp>
#include <Emux/Compiler/SourceFile.hpp>
#include <Emux/Compiler/Token.hpp>
#include <Emux/Compiler/AST/Program.hpp>
#include <memory>

namespace Emux
{

class CompilerContext
{

public:

    Arguments Arguments;

    Diagnostics Diagnostics;

    SourceFile Source;

    std::vector<Token> Tokens;
	std::unique_ptr<Program> AST;
};

}