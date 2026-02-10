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

   protected:
    volatile float vbat_v = 0.0f;
    volatile float shunt_v = 0.0f;
    volatile bool fault = false;
    volatile bool ready = true;
    volatile State state = State::Idle;
};

#endif // LPU_SHARED_HPP