#ifndef LPU_SHARED_HPP
#define LPU_SHARED_HPP

#include "C++Utilities/CppImports.hpp"

class LPUBase {
   protected:

   public:
    auto getDownLinkLayout() {
        return std::make_tuple(&fault, &ready, &is_fixed_vbat, &fixed_vbat, &is_fixed_duty_cycle, &fixed_duty_cycle, &is_reset); 
    }

    auto getUpLinkLayout() {
        return std::make_tuple(&vbat_v, &shunt_v, &duty_cycle, &is_enabled); 
    }

    volatile float vbat_v = 0.0f;
    volatile float shunt_v = 0.0f;
    volatile float duty_cycle = 0.0f;
    volatile bool is_enabled = false;

    volatile bool fault = false;
    volatile bool ready = true;
    volatile bool is_fixed_vbat = false;
    volatile float fixed_vbat = 0.0f;
    volatile bool is_fixed_duty_cycle = false;
    volatile float fixed_duty_cycle = 0.0f;
    volatile bool is_reset = false;
};

#endif // LPU_SHARED_HPP