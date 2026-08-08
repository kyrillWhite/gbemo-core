#include "data_bus.h"

DataBus::DataBus() : memory(nullptr)
{}

u8 DataBus::readMemory()
{
    if (!memory) {
        throw std::runtime_error("Memory is not initialized");
    }
    return memory->read();
}

void DataBus::writeMemory(u8 _data)
{
    if (!memory) {
        throw std::runtime_error("Memory is not initialized");
    }
    memory->write(_data);
}

void DataBus::setMemory(Memory* _memory)
{
    memory = _memory;
}
