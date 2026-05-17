#ifndef AIRGAP_SHARED_HPP
#define AIRGAP_SHARED_HPP

#include "C++Utilities/CppImports.hpp"

class AirgapBase {
   public:
    volatile float airgap_v = 0.01f;

    auto get_uplink_layout() {
        return std::make_tuple(&airgap_v);
    }

    auto get_downlink_layout() {
        return std::tuple<>{};
    }
};

template <typename AirgapTuple>
class AirgapArrayBase;

template <typename... Airgaps>
class AirgapArrayBase<std::tuple<Airgaps...>> {
public:
    explicit AirgapArrayBase(std::tuple<Airgaps...>& airgap_tuple) : airgaps(airgap_tuple) {}

    auto get_uplink_layout() {
        return std::apply([](auto&... airgap) {
            return std::tuple_cat(airgap.get_uplink_layout()...);
        }, airgaps);
    }

    auto get_downlink_layout() {
        return std::apply([](auto&... airgap) {
            return std::tuple_cat(airgap.get_downlink_layout()...);
        }, airgaps);
    }

protected:
    static constexpr size_t count = sizeof...(Airgaps);
    std::tuple<Airgaps...>& airgaps;
};

#endif // AIRGAP_SHARED_HPP
