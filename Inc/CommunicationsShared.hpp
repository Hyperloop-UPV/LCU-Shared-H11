#ifndef COMMUNICATIONS_SHARED_HPP
#define COMMUNICATIONS_SHARED_HPP

#include "C++Utilities/CppImports.hpp"

// ============================================
// Command Flags (Bitmask)
// ============================================

enum class CommandFlags : uint16_t {
    NONE            = 0,
    LEVITATE        = 1 << 0,
    CURRENT_CONTROL = 1 << 1,
    PWM             = 1 << 2
};

// Bitwise operators for CommandFlags
constexpr CommandFlags operator|(CommandFlags a, CommandFlags b) {
    return static_cast<CommandFlags>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

constexpr CommandFlags operator&(CommandFlags a, CommandFlags b) {
    return static_cast<CommandFlags>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
}

constexpr CommandFlags operator~(CommandFlags a) {
    return static_cast<CommandFlags>(~static_cast<uint16_t>(a));
}

// ============================================
// Command Structs (Parameters only)
// ============================================

struct LevitateParams {
    volatile float desired_distance;
};

struct CurrentControlParams {
    volatile float desired_current;
    volatile uint16_t lpu_id_bitmask;
};

struct PWMParams {
    volatile float frequency;
    volatile float duty_cycle;
    volatile uint16_t lpu_id_bitmask;
};

// ============================================
// Command Packet (Master -> Slave)
// ============================================

struct CommandPacket {
    static constexpr uint16_t START_BYTE = 0xABCD;
    static constexpr uint16_t END_BYTE = 0xDCBA;

    volatile uint16_t start_byte;
    volatile CommandFlags flags; // Active commands bitmask
    
    volatile LevitateParams levitate;
    volatile CurrentControlParams current_control;
    volatile PWMParams pwm;

    volatile uint16_t end_byte;
    
    CommandPacket() 
        : start_byte(START_BYTE)
        , flags(CommandFlags::NONE)
        , levitate{0.0f}
        , current_control{0.0f, 0}
        , pwm{0.0f, 0.0f, 0}
        , end_byte(END_BYTE)
    {}
};

// ============================================
// Status Packet (Slave -> Master)
// ============================================

struct StatusPacket {
    static constexpr uint16_t START_BYTE = 0xABCD;
    static constexpr uint16_t END_BYTE = 0xDCBA;

    volatile uint16_t start_byte;
    
    volatile uint8_t system_state;         // SystemStates enum value
    volatile uint8_t control_state;        // ControlStates enum value
    volatile uint16_t error_code;          // Detailed error code if fault

    volatile uint16_t end_byte;
    
    StatusPacket() 
        : start_byte(START_BYTE)
        , system_state(0)
        , control_state(0)
        , error_code(0)
        , end_byte(END_BYTE)
    {}
};

// ============================================
// Communication Base Class
// ============================================

class CommunicationsBase {
public:
    auto getDownLinkLayout() {
        return std::make_tuple(&command_packet);
    }

    auto getUpLinkLayout() {
        return std::make_tuple(&status_packet);
    }
    
    volatile CommandPacket command_packet;
    volatile StatusPacket status_packet;
};

#endif // COMMUNICATIONS_SHARED_HPP