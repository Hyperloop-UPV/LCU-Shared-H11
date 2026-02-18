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
    PWM             = 1 << 2,
    CONTROL_LOOP    = 1 << 3,
    FIXED_VBAT      = 1 << 4,
    RESET_SLAVE     = 1 << 5
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
    float desired_distance;
};

struct CurrentControlParams {
    float desired_current;
    uint16_t lpu_id_bitmask;
};

struct PWMParams {
    uint32_t frequency;
    float duty_cycle;
    uint16_t lpu_id_bitmask;
};

struct FixedVbatParams {
    float fixed_vbat;
};

// ============================================
// Command Packet (Master -> Slave)
// ============================================

struct CommandPacket {
    static constexpr uint16_t START_BYTE = 0xABCD;
    static constexpr uint16_t END_BYTE = 0xDCBA;

    uint16_t start_byte;
    CommandFlags flags; // Active commands bitmask
    
    LevitateParams levitate;
    CurrentControlParams current_control;
    PWMParams pwm;
    FixedVbatParams fixed_vbat;

    uint16_t end_byte;
    
    CommandPacket() 
        : start_byte(START_BYTE)
        , flags(CommandFlags::NONE)
        , levitate{0.0f}
        , current_control{0.0f, 0}
        , pwm{0, 0.0f, 0}
        , fixed_vbat{0.0f}
        , end_byte(END_BYTE)
    {}
};

// ============================================
// Status Packet (Slave -> Master)
// ============================================

struct StatusPacket {
    static constexpr uint16_t START_BYTE = 0xABCD;
    static constexpr uint16_t END_BYTE = 0xDCBA;

    uint16_t start_byte;
    
    uint8_t system_state;         // SystemStates enum value
    uint8_t control_state;        // ControlStates enum value
    uint16_t error_code;          // Detailed error code if fault

    uint16_t end_byte;
    
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