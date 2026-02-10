#ifndef AIRGAP_SHARED_HPP
#define AIRGAP_SHARED_HPP

#include "C++Utilities/CppImports.hpp"

class AirgapBase {
   protected:
    volatile float airgap_v = 0.0f;

   public:
    auto getUpLinkLayout() {
        return std::make_tuple(&airgap_v); 
    }

    auto getDownLinkLayout() {
        return std::make_tuple();
    }
};

#endif // AIRGAP_SHARED_HPP