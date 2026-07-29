#include <Emux/Runtime/Memory.hpp>

#include <cstdlib>


namespace Emux
{


Memory::~Memory()
{
    Free();
}



bool Memory::Allocate(
    std::size_t size
)
{
    Free();


    m_Data =
        static_cast<uint8_t*>(
            std::malloc(size)
        );


    if(!m_Data)
        return false;


    m_Size = size;


    return true;
}



void Memory::Free()
{
    if(m_Data)
    {
        std::free(m_Data);

        m_Data = nullptr;
        m_Size = 0;
    }
}



uint8_t* Memory::Data()
{
    return m_Data;
}



std::size_t Memory::Size() const
{
    return m_Size;
}


}