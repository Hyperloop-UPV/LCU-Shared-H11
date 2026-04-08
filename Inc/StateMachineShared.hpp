#ifndef STATE_MACHINE_SHARED_HPP
#define STATE_MACHINE_SHARED_HPP

#include "C++Utilities/CppImports.hpp"

// ============================================
// Slave State Definition
// ============================================

enum class SlaveState : uint8_t { 
    SPI_CONNECTING = 0, 
    IDLE = 1, 
    LEVITATING = 2, 
    FAULT = 3, 
    CURRENT_CONTROL = 4 
};

class StateMachineBase {
public:
    auto getUpLinkLayout() {
        return std::make_tuple(&slave_state);
    }
    
    volatile SlaveState slave_state = SlaveState::SPI_CONNECTING;
};

#endif // STATE_MACHINE_SHARED_HPP
