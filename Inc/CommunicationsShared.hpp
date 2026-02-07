#ifndef COMMUNICATIONS_SHARED_HPP
#define COMMUNICATIONS_SHARED_HPP

#include "C++Utilities/CppImports.hpp"

// ============================================
// Command Bitmask (Master -> Slave)
// ============================================

enum class CommandFlags : uint16_t {
    NONE            = 0x0 << 0,
    ENABLE          = 0x1 << 0,
    DISABLE         = 0x1 << 1,
    START_LEVITATE  = 0x1 << 2,
    STOP_LEVITATE   = 0x1 << 3,
    STICK_DOWN      = 0x1 << 4,
    RELEASE_STICK   = 0x1 << 5,
};

// Bitwise operators for CommandFlags
constexpr CommandFlags operator|(CommandFlags a, CommandFlags b) {
    return static_cast<CommandFlags>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

constexpr CommandFlags operator&(CommandFlags a, CommandFlags b) {
    return static_cast<CommandFlags>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
}

constexpr bool has_flag(CommandFlags flags, CommandFlags flag) {
    return (flags & flag) == flag;
}

// ============================================
// Status Bitmask (Slave -> Master)
// ============================================

enum class StatusFlags : uint16_t {
    NONE                = 0x0000,
    IDLE                = 0x1 << 0,
    READY               = 0x1 << 1,
    RUNNING             = 0x1 << 2,
    STICK_DOWN_ACTIVE   = 0x1 << 3,
};

// Bitwise operators for StatusFlags
constexpr StatusFlags operator|(StatusFlags a, StatusFlags b) {
    return static_cast<StatusFlags>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

constexpr StatusFlags operator&(StatusFlags a, StatusFlags b) {
    return static_cast<StatusFlags>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
}

// ============================================
// Command Packet (Master -> Slave)
// ============================================

struct CommandPacket {
    CommandFlags commands;        // Bitmask of commands
    float gap_reference;          // Desired gap for levitation (mm)
    float current_limit;          // Max current limit (A)
    uint32_t timestamp_ms;        // Master timestamp for synchronization
    
    CommandPacket() 
        : commands(CommandFlags::NONE)
        , gap_reference(0.0f)
        , current_limit(0.0f)
        , timestamp_ms(0)
    {}
};

// ============================================
// Status Packet (Slave -> Master)
// ============================================

struct StatusPacket {
    StatusFlags status;           // Bitmask of status flags
    uint8_t system_state;         // SystemStates enum value
    uint8_t control_state;        // ControlStates enum value
    uint16_t error_code;          // Detailed error code if fault
    uint32_t timestamp_ms;        // Slave timestamp
    
    StatusPacket() 
        : status(StatusFlags::NONE)
        , system_state(0)
        , control_state(0)
        , error_code(0)
        , timestamp_ms(0)
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
    
    CommandPacket command_packet;
    StatusPacket status_packet;
};

#endif // COMMUNICATIONS_SHARED_HPP