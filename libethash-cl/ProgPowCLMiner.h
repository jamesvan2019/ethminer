
#ifndef ETHMINER_PROGPOWCLMINER_H
#define ETHMINER_PROGPOWCLMINER_H

#include "CLMiner.h"
#include <libethcore/ProgPowAux.h>
#include <libprogpow/ProgPow.h>


namespace dev
{
namespace eth
{

enum CLKernelName {
    Stable,
    Experimental,
};

#define OPENCL_PLATFORM_UNKNOWN 0
#define OPENCL_PLATFORM_NVIDIA  1
#define OPENCL_PLATFORM_AMD     2
#define OPENCL_PLATFORM_CLOVER  3

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

    cl::Context m_context;
    cl::CommandQueue m_queue;
    cl::Kernel m_searchKernel;
    cl::Kernel m_dagKernel;
    cl::Buffer m_dag;
    cl::Buffer m_light;
    cl::Buffer m_header;
    cl::Buffer m_searchBuffer;
    unsigned m_globalWorkSize = 0;
    unsigned m_workgroupSize = 0;

    static unsigned s_platformId;
    static unsigned s_numInstances;
    static unsigned s_threadsPerHash;
    static CLKernelName s_clKernelName;
    static vector<int> s_devices;

    /// The local work size for the search
    static unsigned s_workgroupSize;
    /// The initial global work size for the searches
    static unsigned s_initialGlobalWorkSize;

};

}  // namespace eth
}  // namespace dev


#endif  // ETHMINER_PROGPOWCLMINER_H
