#ifndef COMMUNICATIONS_SHARED_HPP
#define COMMUNICATIONS_SHARED_HPP

#include "C++Utilities/CppImports.hpp"

// ============================================
// Order ID (For translation)
// ============================================

enum class OrderID : uint16_t {
    NONE            = 0,
    LEVITATE        = 9000,
    STOP_LEVITATE = 9001,
    CURRENT_CONTROL = 9100,
    START_PWM       = 9101,
    STOP_PWM        = 9102
};

// ============================================
// Command Flags (Master -> Slave) & Acks (Slave -> Master)
// ============================================

enum class CommandFlags : uint16_t {
    NONE            = 0,
    LEVITATE        = 1 << 0,
    STOP_LEVITATE   = 1 << 1,
    CURRENT_CONTROL = 1 << 2,
    START_PWM       = 1 << 3,
    STOP_PWM        = 1 << 4
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
// Command Packet (Master -> Slave)
// ============================================

struct CommandPacket {
    static constexpr uint16_t START_BYTE = 0xABCD;
    static constexpr uint16_t END_BYTE = 0xDCBA;

    volatile uint16_t start_byte;
    volatile CommandFlags commands;        // Bitmask of commands
    
    // Levitation Parameters (ID 9000)
    volatile float desired_distance;

    // Current Control Parameters (ID 9100)
    volatile float desired_current;
    volatile uint8_t current_control_lpu_id;

    // Start PWM Parameters (ID 9101)
    volatile float pwm_frequency;
    volatile float pwm_duty_cycle;
    volatile uint8_t start_pwm_lpu_id;

    // Stop PWM Parameters (ID 9102)
    volatile uint8_t stop_pwm_lpu_id;

    volatile uint16_t end_byte;
    
    CommandPacket() 
        : start_byte(START_BYTE)
        , commands(CommandFlags::NONE)
        , desired_distance(0.0f)
        , desired_current(0.0f)
        , current_control_lpu_id(0)
        , pwm_frequency(0.0f)
        , pwm_duty_cycle(0.0f)
        , start_pwm_lpu_id(0)
        , stop_pwm_lpu_id(0)
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
    volatile CommandFlags acks;            // Bitmask of acknowledgements (mirrors commands)
    volatile uint8_t system_state;         // SystemStates enum value
    volatile uint8_t control_state;        // ControlStates enum value
    volatile uint16_t error_code;          // Detailed error code if fault

    volatile uint16_t end_byte;
    
    StatusPacket() 
        : start_byte(START_BYTE)
        , acks(CommandFlags::NONE)
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

     void send_order(OrderID id, float arg1 = 0.0f, float arg2 = 0.0f, uint8_t lpu_id = 0) {
        
        CommandFlags flag = CommandFlags::NONE;
        switch(id) {
            case OrderID::LEVITATE: 
                flag = CommandFlags::LEVITATE; 
                command_packet.desired_distance = arg1;
                break;
            case OrderID::CURRENT_CONTROL: 
                flag = CommandFlags::CURRENT_CONTROL; 
                command_packet.desired_current = arg1;
                command_packet.current_control_lpu_id = lpu_id;
                break;
            case OrderID::START_PWM: 
                flag = CommandFlags::START_PWM; 
                command_packet.pwm_frequency = arg1;
                command_packet.pwm_duty_cycle = arg2;
                command_packet.start_pwm_lpu_id = lpu_id;
                break;
            case OrderID::STOP_PWM: 
                flag = CommandFlags::STOP_PWM; 
                command_packet.stop_pwm_lpu_id = lpu_id;
                break;
            default: break;
        }

        command_packet.commands = command_packet.commands | flag;
    }

    void update_flag_synchronization() {
        // Clear commands that have been acknowledged
        command_packet.commands = command_packet.commands & ~status_packet.acks;
    }
};

#endif // COMMUNICATIONS_SHARED_HPP