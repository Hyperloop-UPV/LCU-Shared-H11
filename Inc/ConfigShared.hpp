#ifndef CONFIG_SHARED_HPP
#define CONFIG_SHARED_HPP

#include "FrameShared.hpp"
#include "LPUShared.hpp"
#include "AirgapShared.hpp"
#include "StateMachineShared.hpp"
#include "ControlShared.hpp"
#include "ReportShared.hpp"

// ============================================
// Frame type aliases
// ============================================

template <bool isMaster, typename LPUArray, typename AirgapArray>
using FrameType = Frame<isMaster, LPUArray, AirgapArray, StateMachineBase, ControlBase, ReportBase>;

#endif // CONFIG_SHARED_HPP
