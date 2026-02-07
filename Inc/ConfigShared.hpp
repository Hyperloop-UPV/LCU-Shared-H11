#ifndef CONFIG_SHARED_HPP
#define CONFIG_SHARED_HPP

#include "FrameShared.hpp"


// ============================================
// Frame Definitions
// ============================================

using Downlink = std::tuple<LPUBase>; // Example usage
using Uplink = std::tuple<LPUBase>;   // Example usage

template <bool IsMaster>
using SystemFrame = DuplexFrame<IsMaster, Downlink, Uplink>;



#endif // CONFIG_SHARED_HPP