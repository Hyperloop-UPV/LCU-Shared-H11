#ifndef CONFIG_SHARED_HPP
#define CONFIG_SHARED_HPP

#include "FrameShared.hpp"
#include "LPUShared.hpp"
#include "AirgapShared.hpp"
#include "CommunicationsShared.hpp"
#include "FlagsShared.hpp"


// ============================================
// Frame Definitions
// ============================================
#ifdef USE_1_DOF
using Downlink = std::tuple<CommunicationsBase, LPUBase>;
using Uplink = std::tuple<CommunicationsBase, LPUBase, AirgapBase>;

#elif defined(USE_5_DOF)
using Downlink = std::tuple<CommunicationsBase,
                            LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase>;
using Uplink = std::tuple<CommunicationsBase,
                          LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase, LPUBase,
                          AirgapBase, AirgapBase, AirgapBase, AirgapBase, AirgapBase, AirgapBase, AirgapBase, AirgapBase>;
#endif

template <bool IsMaster>
using SystemFrame = DuplexFrame<IsMaster, Downlink, Uplink>;



#endif // CONFIG_SHARED_HPP