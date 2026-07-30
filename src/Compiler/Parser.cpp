#include <Emux/Compiler/Parser.hpp>
#include <Emux/Compiler/AST/SectionNode.hpp>
#include <Emux/Compiler/AST/VariableNode.hpp>
#include <Emux/Compiler/AST/VariableCallNode.hpp>
#include <Emux/Compiler/AST/FunctionNode.hpp>
#include <Emux/Compiler/AST/FunctionCallNode.hpp>
#include <Emux/Compiler/AST/AssignmentNode.hpp>
#include <Emux/Compiler/AST/LiteralNode.hpp>
#include <Emux/Compiler/AST/ReturnNode.hpp>
#include <stdexcept>
#include <charconv>
#include <cctype>

namespace Emux
{

Parser::Parser(
    CompilerContext& context
):
    m_Context(context)
{
}

bool Parser::IsAtEnd() const
{
    return Current().Type == TokenType::EndOfFile;
}

const Token& Parser::Current() const
{
    return m_Context.Tokens[m_Position];
}

const Token& Parser::Previous() const
{
    return m_Context.Tokens[m_Position - 1];
}

const Token& Parser::Peek(size_t offset) const
{
    if (m_Position + offset >= m_Context.Tokens.size())
        return m_Context.Tokens[m_Context.Tokens.size()-1];
    return m_Context.Tokens[m_Position + offset];
}

void Parser::Advance()
{
    if(!IsAtEnd())
    {
        ++m_Position;
    }
}

bool Parser::Check(TokenType type) const
{
    if(IsAtEnd())
    {
        return false;
    }

    return Current().Type == type;
}

OptionalToken Parser::Match(TokenType type)
{
    if(!Check(type))
    {
        return std::nullopt;
    }

    Advance();

    return Previous();
}

OptionalToken Parser::Consume(
    TokenType type,
    std::string_view message
)
{
    auto token = Match(type);

    if (token)
    {
        return token;
    }

    m_Context.Diagnostics.Add(
    	DiagnosticLevel::Error,
        Current().Location,
        message
    );

    return std::nullopt;
}

void Parser::SkipNewLines()
{
    while(Match(TokenType::NewLine).has_value()){}
}

void Parser::AdvanceAndSNL()
{
    SkipNewLines();
    Advance();
    SkipNewLines();
}

void Parser::Synchronize()
{
    while(!IsAtEnd())
    {
        if(Match(TokenType::LeftBracket))
        {
            return;
        } else if(Match(TokenType::Keyword))
        {
            return;
        } else if(Match(TokenType::RightParen))
        {
            SkipNewLines();
            if (Match(TokenType::LeftBrace))
            {
                continue;
            }
            return;
        } else if(Match(TokenType::RightBrace))
        {
            return;
        }


        Advance();
    }
}

bool Parser::IsSectionStart() const
{
    return Check(
        TokenType::LeftBracket
    );
}

bool Parser::IsVarsSection() const
{
    return Current().Text == "Vars";
}

bool Parser::IsVarsSection(
    const Token& token
) const
{
    return token.Text == "Vars";
}

bool Parser::IsFunctionStart() const
{
    return Current().Text == "func";
}

bool Parser::IsFunctionStart(
    const Token& token
) const
{
    return token.Text == "func";
}

bool Parser::IsLiteral() const
{
    return Check(TokenType::Number) || Check(TokenType::String);
}

bool Parser::IsLiteral(
    const Token& token
) const
{
    return (token.Type == TokenType::Number) || (token.Type == TokenType::String);
}

void Parser::Parse()
{
    m_Context.AST = std::make_unique<Program>();


    while(!IsAtEnd())
    {
        SkipNewLines();


        if(IsAtEnd())
            break;


        if(IsSectionStart())
        {
            ParseSection(*m_Context.AST);
        }
        else
        {
            m_Context.Diagnostics.Add(
            	DiagnosticLevel::Error,
                Current().Location,
                "Expected section."
            );

            Synchronize();
        }
    }
}

void Parser::ParseSection(Program& program)
{
    auto left = Consume(
        TokenType::LeftBracket,
        "Expected '['."
    );


    auto name = Consume(
        TokenType::Identifier,
        "Expected section name."
    );


    if(!left || !name)
    {
        Synchronize();
        return;
    }


    auto section = std::make_unique<SectionNode>(
        name->get().Location
    );


    section->Name = name->get();


    if(Match(TokenType::Colon))
    {
        ParseDependencies(
            *section
        );
    }


    auto right = Consume(
        TokenType::RightBracket,
        "Expected ']'."
    );


    if(!right)
    {
        Synchronize();
        return;
    }

    SkipNewLines();
    if(name->get().Text == "Vars")
    {
        ParseVars(*section);
        if(!program.AddSection(std::move(section)))
        {
            std::string message = "Section '" + name->get().Text + "' already defined.";
            m_Context.Diagnostics.Add(
                DiagnosticLevel::Error,
                name->get().Location,
                message
            );
        }
        return;
    }

    while (Current().Type != TokenType::LeftBracket 
        && Current().Type != TokenType::EndOfFile)
    {
        SkipNewLines();

        if (IsFunctionStart(Current())){
            ParseFunction(*section);
            continue;
        }

        ParseExpression(*section);
    }
    
    if(!program.AddSection(std::move(section)))
    {
        std::string message = "Section '" + name->get().Text + "' already defined.";
        m_Context.Diagnostics.Add(
            DiagnosticLevel::Error,
            name->get().Location,
            message
        );
    }
}

void Parser::ParseDependencies(
    SectionNode& section
)
{

    auto dependency = Consume(
        TokenType::Identifier,
        "Expected dependency name."
    );


    if(!dependency)
    {
        Synchronize();
        return;
    }


    section.Dependencies.push_back(
        dependency->get()
    );


    while(Match(TokenType::Comma))
    {

        dependency = Consume(
            TokenType::Identifier,
            "Expected dependency name."
        );


        if(!dependency)
        {
            Synchronize();
            return;
        }


        section.Dependencies.push_back(
            dependency->get()
        );
    }
}

void Parser::ParseVars(
    Node& node
)
{
    SkipNewLines();


    while(Check(TokenType::Identifier))
    {
        ParseVariable(node);

        SkipNewLines();
    }
}

void Parser::ParseVariable(
    Node& node
)
{
    auto name = Consume(
        TokenType::Identifier,
        "Expected variable name."
    );


    Consume(
        TokenType::Colon,
        "Expected ':'."
    );


    auto type = Consume(
        TokenType::Identifier,
        "Expected type."
    );


    if(!name || !type)
    {
        Synchronize();
        return;
    }


    auto variable = std::make_unique<VariableNode>(
        name->get().Location
    );


    variable->Name = name->get();
    m_Variables.insert(name->get().Text);

    variable->Type = type->get();


    node.Children.push_back(
        std::move(variable)
    );
}

void Parser::ParseFunction(
    SectionNode& node
)
{
    // Espera: "func nomeFuncao ( params ) -> retType { body }"
    
    if (!IsFunctionStart(Current()))
    {
        return;
    }
    
    Advance(); // Consome "func"
    
    // Pega nome da função
    if (!Check(TokenType::Identifier))
    {
        m_Context.Diagnostics.Add(
            DiagnosticLevel::Error,
            Current().Location,
            "Expected function name"
        );
        Synchronize();
        return;
    }
    Token funcName = Current();
    Advance(); // Consome "name"
    
    auto funcNode = std::make_unique<FunctionNode>(
        funcName,
        funcName.Location
    );
    
    // Parâmetros: ( tipo nome, tipo nome, ... )
    if (!Check(TokenType::LeftParen))
    {
        m_Context.Diagnostics.Add(
            DiagnosticLevel::Error,
            Current().Location,
            "Expected '(' after function name"
        );
        Synchronize();
        return;
    }
    AdvanceAndSNL(); // Consome "("
    
    // Parse parâmetros
    while (!Check(TokenType::RightParen) && !IsAtEnd())
    {
        // Tipo do parâmetro
        if (!Check(TokenType::Identifier))
        {
            m_Context.Diagnostics.Add(
                DiagnosticLevel::Error,
                Current().Location,
                "Expected parameter type"
            );
            Advance();
            Synchronize();
            return;
        }

        Token paramType = Current();
        Advance(); // Consome Parameter Type
        
        // Nome do parâmetro
        if (!Check(TokenType::Identifier))
        {
            m_Context.Diagnostics.Add(
                DiagnosticLevel::Error,
                Current().Location,
                "Expected parameter name"
            );
            Advance(),
            Synchronize();
            return;
        }
        Token paramName = Current();
        Advance(); // Consome Parameter Name
        
        // Adiciona ao vetor de parâmetros
        funcNode->Parameters.emplace_back(paramType, paramName);
        
        // Verifica se há mais parâmetros
        if (Check(TokenType::Comma))
        {
            AdvanceAndSNL(); // Consome ","
        }
        else if (!Check(TokenType::RightParen))
        {
            m_Context.Diagnostics.Add(
                DiagnosticLevel::Error,
                Current().Location,
                "Expected ',' or ')' in parameter list"
            );
            Advance();
            Synchronize();
            return;
        }
    }

    if (!Check(TokenType::RightParen))
    {
        m_Context.Diagnostics.Add(
            DiagnosticLevel::Error,
            Current().Location,
            "Expected ')' after parameters"
        );
        return;
    }
    Advance(); // Consome ")"

    std::string scopedName = node.Name.Text + "::" + funcName.Text;
    m_Functions.emplace(scopedName, std::cref(funcNode->Parameters));

    if (!Check(TokenType::Pointer))
    {
        m_Context.Diagnostics.Add(
            DiagnosticLevel::Error,
            Current().Location,
            "Expected symbol -> after 'func'"
        );
        Synchronize();
        return;
    }
    Advance(); // Consome "->"
    // Pega tipo de retorno
    if (!Check(TokenType::Identifier))
    {
        m_Context.Diagnostics.Add(
            DiagnosticLevel::Error,
            Current().Location,
            "Expected return type after '->'"
        );
        Synchronize();
        return;
    }
    Token returnType = Current();
    funcNode->ReturnType = returnType;

    AdvanceAndSNL(); // Consome "Return type"
    
    // Corpo: { ... }
    if (!Check(TokenType::LeftBrace))
    {
        m_Context.Diagnostics.Add(
            DiagnosticLevel::Error,
            Current().Location,
            "Expected '{' before function body"
        );
        Advance();
        Synchronize();
        return;
    }

    SkipNewLines();
    Advance(); // Consome "{"
    
    // Parse statements do corpo
    while (!Check(TokenType::RightBrace) && !IsAtEnd())
    {
        SkipNewLines();

        if (Check(TokenType::RightBrace) || IsAtEnd())
        {
            break;
        }

        ParseStatement(*funcNode);

        SkipNewLines();
        if (Check(TokenType::RightBrace) || IsAtEnd())
        {
            break;
        }
        
        AdvanceAndSNL();
    }
    
    if (!Check(TokenType::RightBrace))
    {
        m_Context.Diagnostics.Add(
            DiagnosticLevel::Error,
            Current().Location,
            "Expected '}' after function body"
        );
        Advance();
        Synchronize();
        return;
    }
    Advance(); // Consome "}"
    
    node.Children.push_back(
        std::move(funcNode)
    );
}

void Parser::ParseExpression(
    Node& node
)
{
    if (Check(TokenType::Identifier))
    {
        if (Peek(1).Type == TokenType::LeftParen)
        {
            m_Context.Diagnostics.Add(
                DiagnosticLevel::Error,
                Current().Location,
                "Expected scope before function call"
            );
            Advance();
            Advance();
            Synchronize();
        } else if (Peek(1).Type == TokenType::Colon)
        {
            ParseFunctionCall(node);
        } else if (Peek(1).Type == TokenType::Equal)
        {
            ParseAssignment(node);
        } else {
            ParseVariableCall(node);
        }
    } else if (Check(TokenType::Keyword))
    {
        Token curr = Current();   
    } else if (IsLiteral())
    {
        ParseLiteral(node);
    }
}

void Parser::ParseFunctionCall(Node& node)
{
    if (!Check(TokenType::Identifier) || Peek(1).Type != TokenType::Colon)
    {
        return;
    }

    auto sectionToken = Consume(
        TokenType::Identifier,
        "Expected section name."
    );

    if (!sectionToken)
    {
        Synchronize();
        return;
    }

    const std::string& section = sectionToken->get().Text;

    if(!Consume(TokenType::Colon, "Expected :: after section name."))
    {
        Synchronize();
        return;
    }

    if(!Consume(TokenType::Colon, "Expected :: after section name."))
    {
        Synchronize();
        return;
    }

    auto nameToken = Consume(
        TokenType::Identifier,
        "Expected function name after '"+
        section +
        "::'."
    );

    if (!nameToken)
    {
        Synchronize();
        return;
    }

    std::string name = nameToken->get().Text;
    std::string scopedName = section + "::" + name;

    Token scopedToken;
    scopedToken.Type = TokenType::Identifier;
    scopedToken.Text = scopedName;
    scopedToken.Location = sectionToken->get().Location;

    auto funcCallNode = std::make_unique<FunctionCallNode>(
        scopedToken,
        scopedToken.Location
    );

    if(!Consume(TokenType::LeftParen, 
        "Expected ( after '"+
        scopedName + "'."
    )) // Consome '('
    {
        Synchronize();
        return;
    }

    auto it = m_Functions.find(scopedName);
    if (it == m_Functions.end())
    {
        m_Context.Diagnostics.Add(
            DiagnosticLevel::Error,
            funcCallNode->GetLocation(),
            "Invalid call, function '" +
            scopedName +
            "' not defined"
        );
        Advance();
        Synchronize();
        return;
    }

    const std::vector<FunctionParameter>& Params = it->second.get();

    SkipNewLines();
    while (Current().Type != TokenType::RightParen && 
           Current().Type != TokenType::EndOfFile)
    {
        Token value = Current();

        FunctionParameter parameter({ .Location = Current().Location},{ .Location = Current().Location });
        // Se 'i' estiver dentro do limite dos parâmetros esperados, configuramos
        if (funcCallNode->Parameters.size() < Params.size())
        {
            parameter = Params[funcCallNode->Parameters.size()];
        }
        parameter.Value = value;

        Advance(); // Consome param value

        funcCallNode->Parameters.emplace_back(std::move(parameter));

        if (Check(TokenType::Comma))  // Consome ,
        {
            AdvanceAndSNL();
            continue;
        } else if(Check(TokenType::RightParen)) // Verifica )
        {
            break;
        } else { // Error
            m_Context.Diagnostics.Add(
                DiagnosticLevel::Error,
                Current().Location,
                "Expected , or ) after parameter value " + 
                value.Text + 
                " in function '" +
                scopedName +
                "'." 
            );
            Synchronize();
            return;
        }
    }

    if (funcCallNode->Parameters.size() != Params.size())
    {
        m_Context.Diagnostics.Add(
            DiagnosticLevel::Error,
            funcCallNode->GetLocation(),
            "Function '" + scopedName + "' expects " +
            std::to_string(Params.size()) + " parameter(s), but received " +
            std::to_string(funcCallNode->Parameters.size())
        );
        Synchronize();
        return;
    }
    
    if (!Consume(TokenType::RightParen, "Expected ')' after parameters"))
    {
        Synchronize();
        return;
    } // Consome ")"

    node.Children.push_back(
        std::move(funcCallNode)
    );
}

void Parser::ParseVariableCall(Node& node)
{
    auto nameToken = Consume(
        TokenType::Identifier,
        "Expected name of variable"
    );

    if (!nameToken) {
        Advance();
        Synchronize();
        return;
    }

    std::string nameText = nameToken->get().Text;

    if (!m_Variables.contains(nameText))
    {
        m_Context.Diagnostics.Add(
            DiagnosticLevel::Error,
            nameToken->get().Location,
            "Variable '" + nameText + "' not defined."
        );
        Synchronize();
        return;
    }

    auto varCallNode = std::make_unique<VariableCallNode>(
        nameToken->get(),
        nameToken->get().Location
    );

    node.Children.push_back(std::move(varCallNode));
}

void Parser::ParseAssignment(Node& node)
{
    if (Peek(1).Type != TokenType::Equal)
    {
        return;
    }

    auto varToken = Consume(
        TokenType::Identifier,
        "Expected name of variable"
    );

    if (!varToken)
    {
        Advance();
        Synchronize();
        return;
    }

    const std::string& varName = varToken->get().Text;

    if(!Consume(TokenType::Equal, "Expected = after '"+varName+"'."))
    {
        Advance();
        Synchronize();
        return;
    }

    auto assignNode = std::make_unique<AssignmentNode>(
        varToken->get(),
        varToken->get().Location
    );

    ParseExpression(*assignNode);

    node.Children.push_back(std::move(assignNode));  
}

void Parser::ParseLiteral(Node& node)
{
    if (!Check(TokenType::Number) && !Check(TokenType::String))
    {
        return;
    }

    Token value { Current() };
    Token type {
        .Type { TokenType::Identifier },
        .Location { value.Location }
    };

    if (Check(TokenType::Number))
    {
        std::int64_t amount = 0;

        auto result = std::from_chars(
            value.Text.data(),
            value.Text.data() + value.Text.size(),
            amount
        );

        if(result.ec != std::errc{})
        {
            m_Context.Diagnostics.Add(
                DiagnosticLevel::Error,
                Current().Location,
                "Invalid number '" + value.Text  + "'."
            );
            Advance();
            Synchronize();
            return;
        }

        if (amount > 0 && amount <= 255) 
        {
            type.Text = "u8";
        }
        else if(amount > 255)
        {
            type.Text = "u16";
        }
        else if(amount >= -128 && amount <= 127)
        {
            type.Text = "i8";
        }
        else if(amount < 0)
        {
            type.Text = "i32";
        }
    } else if (Check(TokenType::String))
    {
        type.Text = "U"+std::to_string(value.Location.Length);
    }

    Advance();

    auto literalNode = std::make_unique<LiteralNode>(
        type,
        value,
        value.Location
    );

    node.Children.push_back(std::move(literalNode));
}

void Parser::ParseReturn(Node& node)
{
    if (Current().Text != "return")
    {
        return;
    }

    auto retNode = std::make_unique<ReturnNode>(Current().Location);
    AdvanceAndSNL();

    ParseExpression(*retNode);

    node.Children.push_back(std::move(retNode));
}

void Parser::ParseStatement(Node& node)
{
    SkipNewLines();

    Token curr = Current();
    if (curr.Text == "return")
    {
        ParseReturn(node);
        return;
    }

    ParseExpression(node);
}

}