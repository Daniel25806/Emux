#include <Emux/Runtime/Machine.hpp>


namespace Emux
{

bool Machine::Initialize(
    const MemoryLayout& layout
)
{
    if(m_State != MachineState::Created &&
       m_State != MachineState::Stopped)
    {
        return false;
    }
    m_State = MachineState::Ready;

    if (layout.GetSize() > 0) {
        if(!m_Memory.Allocate(layout.GetSize()))
        {
            return false;
        }
    }


    m_View = std::make_unique<MemoryView>(m_Memory);


    return true;
}

void Machine::Shutdown()
{
    if(m_State == MachineState::Stopped)
        return;


    m_View.reset();

    m_Memory.Free();


    m_State = MachineState::Stopped;
}

MachineState Machine::State() const
{
    return m_State;
}

Memory& Machine::GetMemory()
{
    return m_Memory;
}

MemoryView& Machine::GetMemoryView()
{
    return *m_View;
}

}