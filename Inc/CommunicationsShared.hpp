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
    ENABLE_LPU_BUFFER = 1 << 2
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
    bool ramping;
};

struct CurrentControlParams {
    float desired_current;
    uint16_t lpu_id_bitmask;
};

struct ForceEnableLpuBufferParams {
    uint16_t lpu_buffer_id_bitmask;
};

// ============================================
// Command Packet (Master -> Slave)
// ============================================

struct CommandPacket {
    static constexpr uint8_t START_BYTE = 0xAB;
    static constexpr uint8_t END_BYTE = 0xCD;

    uint8_t start_byte;
    CommandFlags flags; // Active commands bitmask
    
    LevitateParams levitate;
    CurrentControlParams current_control;
    ForceEnableLpuBufferParams force_enable_lpu_buffer;

    uint8_t end_byte;
    
    CommandPacket() 
        : start_byte(START_BYTE)
        , flags(CommandFlags::NONE)
        , levitate{0.0f, false}
        , current_control{0.0f, 0}
        , force_enable_lpu_buffer{0}
        , end_byte(END_BYTE)
    {}
};

// ============================================
// Status Packet (Slave -> Master)
// ============================================

enum class SlaveState : uint8_t { SPI_CONNECTING = 0, IDLE = 1, LEVITATING = 2, FAULT = 3, CURRENT_CONTROL = 4 };

struct StatusPacket {
    static constexpr uint8_t START_BYTE = 0xAB;
    static constexpr uint8_t END_BYTE = 0xCD;

    uint8_t start_byte;

    float desired_current1;
    float desired_current2;
    float desired_current3;
    float desired_current4;

    float state0;
    float state1;
    float state2;
    float state3;
    float state4;

    float airgap_local_1;
    float airgap_local_2;
    float airgap_local_3;
    float airgap_local_4;

    float Fe[3];
    float Fa[4];
    float Ef[3];
    float P[3];
    float R[3];
    float Zz[3];
    float Fe_L[3];

    float desired_voltage_1;
    float desired_voltage_2;
    float desired_voltage_3;
    float desired_voltage_4;

    float target_distance;

    SlaveState slave_state;   // SystemStates enum value
    uint16_t error_code;            // Detailed error code if fault

    uint8_t end_byte;
    
    StatusPacket() 
        : start_byte(START_BYTE)
        , desired_current1(0.0f)
        , desired_current2(0.0f)
        , desired_current3(0.0f)
        , desired_current4(0.0f)
        , state0(0.0f)
        , state1(0.0f)
        , state2(0.0f)
        , state3(0.0f)
        , state4(0.0f)
        , airgap_local_1(0.0f)
        , airgap_local_2(0.0f)
        , airgap_local_3(0.0f)
        , airgap_local_4(0.0f)
        , target_distance(0.0f)
        , Fe{0.0f, 0.0f, 0.0f}
        , Fa{0.0f, 0.0f, 0.0f, 0.0f}
        , Ef{0.0f, 0.0f, 0.0f}
        , P{0.0f, 0.0f, 0.0f}
        , R{0.0f, 0.0f, 0.0f}
        , Zz{0.0f, 0.0f, 0.0f}
        , Fe_L{0.0f, 0.0f, 0.0f}
        , desired_voltage_1(0.0f)
        , desired_voltage_2(0.0f)
        , desired_voltage_3(0.0f)
        , desired_voltage_4(0.0f)
        , slave_state(SlaveState::SPI_CONNECTING)
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