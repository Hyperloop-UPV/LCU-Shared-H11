#ifndef CONFIG_SHARED_HPP
#define CONFIG_SHARED_HPP

#include "FrameShared.hpp"
#include "LPUShared.hpp"
#include "AirgapShared.hpp"
#include "CommunicationsShared.hpp"


// ============================================
// Frame Definitions
// ============================================

using Downlink = std::tuple<CommunicationsBase, LPUBase>; // Example usage
using Uplink = std::tuple<CommunicationsBase, LPUBase, AirgapBase>;   // Example usage

template <bool IsMaster>
using SystemFrame = DuplexFrame<IsMaster, Downlink, Uplink>;



#endif // CONFIG_SHARED_HPP