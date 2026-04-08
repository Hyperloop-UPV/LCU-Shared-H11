#ifndef CONTROL_SHARED_HPP
#define CONTROL_SHARED_HPP

#include "C++Utilities/CppImports.hpp"

// ============================================
// Control Mode
// ============================================

enum class ControlMode : uint8_t {
    NONE             = 0,
    DISTANCE_CONTROL = 1,
    CURRENT_CONTROL  = 2
};

// ============================================
// Control Parameters
// ============================================

struct DistanceControlParams {
    float desired_distance;
};

struct CurrentControlParams {
    float desired_current;
};

// ============================================
// Control Packet (Master -> Slave)
// ============================================

struct ControlPacket {
    ControlMode mode; // Active control mode
    
    DistanceControlParams distance_control;
    CurrentControlParams current_control;

    ControlPacket() 
        : mode(ControlMode::NONE)
        , distance_control{0.0f}
        , current_control{0.0f}
    {}
};

// ============================================
// Control Base (for Frame integration)
// ============================================

class ControlBase {
public:
    auto getDownLinkLayout() {
        return std::make_tuple(&control_packet);
    }
    
    volatile ControlPacket control_packet;
};

#endif // CONTROL_SHARED_HPP
