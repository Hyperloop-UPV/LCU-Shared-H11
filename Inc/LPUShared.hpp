#ifndef LPU_SHARED_HPP
#define LPU_SHARED_HPP

#include "C++Utilities/CppImports.hpp"

class LPUBase {
   protected:

   public:
    auto get_downlink_layout() {
        return std::make_tuple(&is_fixed_vbat, &fixed_vbat, &fixed_duty_cycle);
    }

    auto get_uplink_layout() {
        return std::make_tuple(&vbat_v, &shunt_v, &duty_cycle);
    }

    volatile float vbat_v = 0.0f;
    volatile float shunt_v = 0.0f;
    volatile float duty_cycle = 0.0f;

    volatile bool is_fixed_vbat = false;
    volatile float fixed_vbat = 0.0f;
    volatile float fixed_duty_cycle = 0.0f;
};

template <typename LpuTuple>
class LpuArrayBase;

template <typename... Lpus>
class LpuArrayBase<std::tuple<Lpus...>> {
public:

    explicit LpuArrayBase(std::tuple<Lpus...>& lpu_tuple) : lpus(lpu_tuple) {}

    auto get_uplink_layout() {
        return std::apply([](auto&... lpu) {
            return std::tuple_cat(lpu.get_uplink_layout()...);
        }, lpus);
    }

    auto get_downlink_layout() {
        return std::apply([](auto&... lpu) {
            return std::tuple_cat(lpu.get_downlink_layout()...);
        }, lpus);
    }

protected:
    static constexpr size_t count = sizeof...(Lpus);
    std::tuple<Lpus...>& lpus;
};

#endif // LPU_SHARED_HPP
