#ifndef CONTROL_SHARED_HPP
#define CONTROL_SHARED_HPP

#include "C++Utilities/CppImports.hpp"

class ControlBase {
public:
    auto get_uplink_layout() {
        return std::make_tuple(&output);
    }

    auto get_downlink_layout() {
        return std::make_tuple(&input);
    }

    struct Output {
        float Voltages[10];
        float GapsLocales[4];
        float Estados[5];
        float CorrienteReferencia[4];
        float Fe[3];
        float Fa[4];
        float Ef[3];
        float P[3];
        float R[3];
        float Zz[3];
        float Fe_L[3];
        float Referencia;
        float I_HEMS_OUT_LOG[4];
        float A[8];
        float Ak[4];
        float Bk[3];

        void clear() volatile {
            std::fill(std::begin(Voltages), std::end(Voltages), 0.0f);
            std::fill(std::begin(GapsLocales), std::end(GapsLocales), 0.0f);
            std::fill(std::begin(Estados), std::end(Estados), 0.0f);
            std::fill(std::begin(CorrienteReferencia), std::end(CorrienteReferencia), 0.0f);
            std::fill(std::begin(Fe), std::end(Fe), 0.0f);
            std::fill(std::begin(Fa), std::end(Fa), 0.0f);
            std::fill(std::begin(Ef), std::end(Ef), 0.0f);
            std::fill(std::begin(P), std::end(P), 0.0f);
            std::fill(std::begin(R), std::end(R), 0.0f);
            std::fill(std::begin(Zz), std::end(Zz), 0.0f);
            std::fill(std::begin(Fe_L), std::end(Fe_L), 0.0f);
            Referencia = 0.0f;
            std::fill(std::begin(I_HEMS_OUT_LOG), std::end(I_HEMS_OUT_LOG), 0.0f);
            std::fill(std::begin(A), std::end(A), 0.0f);
            std::fill(std::begin(Ak), std::end(Ak), 0.0f);
            std::fill(std::begin(Bk), std::end(Bk), 0.0f);
        }
    };

    struct Input {
        float RefZ;
        float RefCurrent;
        bool ramping;
        bool cinema;
        float cinema_current;
    };

    volatile Output output{};
    volatile Input input{};
};

#endif // CONTROL_SHARED_HPP
