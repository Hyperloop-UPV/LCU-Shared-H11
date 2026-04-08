#ifndef CONFIG_SHARED_HPP
#define CONFIG_SHARED_HPP

#include "FrameShared.hpp"
#include "ControlShared.hpp"
#include "LPUShared.hpp"
#include "StateMachineShared.hpp"
#include "AirgapShared.hpp"
#include "FlagsShared.hpp"


// ============================================
// Frame Definition
// ============================================
template <bool IsMaster>
using SystemFrame = DuplexFrame<IsMaster, ControlBase, StateMachineBase, 
#ifdef USE_1_DOF
    LPUBase, AirgapBase
#elif defined(USE_5_DOF)
    LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase,
    AirgapBase, AirgapBase, AirgapBase, AirgapBase, AirgapBase, AirgapBase, AirgapBase, AirgapBase
#endif
    >;

#endif // CONFIG_SHARED_HPP