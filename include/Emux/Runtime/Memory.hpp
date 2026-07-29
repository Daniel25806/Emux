#pragma once

#include <cstddef>
#include <cstdint>


namespace Emux
{

class Memory
{
public:

    Memory() = default;

    ~Memory();


    bool Allocate(
        std::size_t size
    );


    void Free();


    uint8_t* Data();


    std::size_t Size() const;


private:

    uint8_t* m_Data = nullptr;

    std::size_t m_Size = 0;
};

}