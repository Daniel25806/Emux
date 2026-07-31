#pragma once

namespace Emux
{

enum class TokenType
{
    // palavra-chave
    Keyword,

    // Identificadores
    Identifier,

    // Literais
    Number,
    String,

    // Símbolos
    LeftParen, // (
    RightParen, // )

    LeftBracket, // [
    RightBracket, // ]

    LeftBrace, // {
    RightBrace, // }

    Colon, // :
    Comma, // ,
    Pointer, // ->

    Plus, // +
    Minus, // -

    Equal, // = 

    // Controle
    NewLine, // \n
    EndOfFile, // \0

    Unknown
};

}