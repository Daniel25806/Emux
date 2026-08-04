#include <Emux/CLI/Arguments.hpp>

#include <CLI/CLI.hpp>


namespace Emux
{


Arguments ParseArguments(
    int argc,
    char** argv
)
{
    Arguments args;

    CLI::App app{"Emux - Emulate X"};


    app.add_option(
        "file",
        args.file,
        "Emux source file"
    );


    app.add_flag(
        "-v,--version",
        args.version,
        "Show version"
    );


    try
    {
        app.parse(argc, argv);
    }
    catch(const CLI::ParseError& e)
    {
        app.exit(e);
    }


    return args;
}


}