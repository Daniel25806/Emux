#pragma once

#include <string>
#include <vector>
#include <memory>
#include <Emux/Compiler/AST/NodePtr.hpp>
#include <Emux/Compiler/AST/Node.hpp>
#include <Emux/Compiler/AST/FunctionParameter.hpp>
#include <Emux/Compiler/Token.hpp>

namespace Emux
{

// O nó de função
class FunctionNode : public Node
{
public:
    
    FunctionNode(
        const Token& returnType,
        const Token& name,
        const SourceLocation& location
    );

    FunctionNode(
        const Token& name,
        const SourceLocation& location
    );

    // Metadata
    Token ReturnType;                           // Tipo de retorno                              // Nome da função
    std::vector<FunctionParameter> Parameters;  // Parâmetros
/*
    // Para o visitor pattern (se usar)
    void Accept(class ASTVisitor* visitor) override;
*/
};

} // namespace Emux