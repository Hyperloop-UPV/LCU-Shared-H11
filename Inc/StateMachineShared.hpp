#ifndef STATE_MACHINE_SHARED_HPP
#define STATE_MACHINE_SHARED_HPP

#include "C++Utilities/CppImports.hpp"

enum class SlaveState : uint8_t { SPI_CONNECTING = 0, IDLE = 1, LEVITATION = 2, CURRENT_CONTROL = 3, DEBUG = 4, FAULT = 5 };

class StateMachineBase {
   public:
    auto get_uplink_layout() {
        return std::make_tuple(&current_state);
    }

    auto get_downlink_layout() {
        return std::make_tuple(&desired_state, &lpu_bitmask);
    }

    volatile SlaveState desired_state = SlaveState::IDLE;
    volatile uint16_t lpu_bitmask = 0;
    volatile SlaveState current_state = SlaveState::SPI_CONNECTING;
};

#endif // STATE_MACHINE_SHARED_HPP
