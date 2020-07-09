//
// Created by Alex Wu on 2020/7/9.
//

#include "CLMiner.h"
#include <libprogpow/ProgPow.h>

#ifndef ETHMINER_PROGPOWCLMINER_H
#define ETHMINER_PROGPOWCLMINER_H

namespace dev
{
namespace eth
{
class ProgPowCLMiner : public CLMiner
{
public:

    ProgPowCLMiner(unsigned _index, PowType _powType, CLSettings _settings, DeviceDescriptor& _device);
    ~ProgPowCLMiner() override;

//    static void enumDevices(std::map<string, DeviceDescriptor>& _DevicesCollection);

protected:
//    bool initDevice() override;

//    bool initEpoch_internal() override;

//    void kick_miner() override;

private:

    void workLoop() override;
    bool init(int epoch, uint64_t block_number, bool new_epoch, bool new_period);

    vector<cl::Context> m_context;
    vector<cl::CommandQueue> m_queue;
    vector<cl::CommandQueue> m_abortqueue;
    cl::Kernel m_searchKernel;
    cl::Kernel m_dagKernel;
    cl::Device m_device;

    vector<cl::Buffer> m_dag;
    vector<cl::Buffer> m_light;
    vector<cl::Buffer> m_header;
    vector<cl::Buffer> m_searchBuffer;

    PowType    m_powType;
    CLSettings m_settings;

    unsigned m_dagItems = 0;
    uint64_t m_lastNonce = 0;


    /// The local work size for the search
    static unsigned s_workgroupSize;
    /// The initial global work size for the searches
    static unsigned s_initialGlobalWorkSize;

};

}  // namespace eth
}  // namespace dev


#endif  // ETHMINER_PROGPOWCLMINER_H
