#ifndef AIRGAP_SHARED_HPP
#define AIRGAP_SHARED_HPP

#include "C++Utilities/CppImports.hpp"

class AirgapBase {
   public:
    volatile float airgap_v = 0.01f;

    auto getUpLinkLayout() {
        return std::make_tuple(&airgap_v); 
    }
};

#endif // AIRGAP_SHARED_HPP