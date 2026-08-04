include(FetchContent)

FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.4.2
)

FetchContent_MakeAvailable(CLI11)

FetchContent_Declare(
    AsmJIT
    GIT_REPOSITORY https://github.com/asmjit/asmjit.git
    GIT_TAG master
)

FetchContent_MakeAvailable(AsmJIT)
