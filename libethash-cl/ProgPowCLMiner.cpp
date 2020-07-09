/// OpenCL Ethash miner implementation.
///
/// @file
/// @copyright GNU General Public License
//

#include "ProgPowCLMiner.h"
using namespace dev;
using namespace eth;

ProgPowCLMiner::ProgPowCLMiner(unsigned _index, PowType _powType, CLSettings _settings, DeviceDescriptor& _device)
    : CLMiner(_index,_powType,_settings,_device)
{
    DEV_BUILD_LOG_PROGRAMFLOW(cllog, "cl-" << m_index << " ProgPowCLMiner::EthashCLMiner() called");
}

ProgPowCLMiner::~ProgPowCLMiner()
{
    DEV_BUILD_LOG_PROGRAMFLOW(cllog, "cl-" << m_index << " ProgPowCLMiner::~EthashCLMiner() begin");
    DEV_BUILD_LOG_PROGRAMFLOW(cllog, "cl-" << m_index << " ProgPowCLMiner::~EthashCLMiner() end");
}

void ProgPowCLMiner::workLoop()
{
}

