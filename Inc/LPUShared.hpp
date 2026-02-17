#ifndef LPU_SHARED_HPP
#define LPU_SHARED_HPP

#include "C++Utilities/CppImports.hpp"

class LPUBase {
   protected:
    enum class State : uint8_t { 
        Idle,
        Ready,
        Running,
        Fault
    };

   public:
    auto getDownLinkLayout() {
        return std::make_tuple(&fault, &ready); 
    }

    auto getUpLinkLayout() {
        return std::make_tuple(&vbat_v, &shunt_v, &state); 
    }

    float vbat_v = 0.0f;
    float shunt_v = 0.0f;
    bool fault = false;
    bool ready = true;
    State state = State::Idle;
};

#endif // LPU_SHARED_HPP