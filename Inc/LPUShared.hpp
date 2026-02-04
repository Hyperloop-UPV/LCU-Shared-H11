#ifndef LPU_SHARED_HPP
#define LPU_SHARED_HPP

#include "C++Utilities/CppImports.hpp"

class LPUBase {
    enum class State : uint8_t { 
        Ready,
        Running,
        Fault
    };

   public:
    template <typename Self>
    void init(this Self&& self) {
        self.initImpl(); 
    }

    template <typename Self>
    void update(this Self&& self) {
        self.updateImpl();
    }

    auto getDownLinkLayout() {
        return std::make_tuple(&state); 
    }

    auto getUpLinkLayout() {
        return std::make_tuple(&vbat_v, &shunt_v); 
    }

   protected:
    float vbat_v = 0.0f;
    float shunt_v = 0.0f;
    State state = State::Ready;
};

#endif // LPU_SHARED_HPP