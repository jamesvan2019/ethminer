
#ifndef ETHMINER_PROGPOWCLMINER_H
#define ETHMINER_PROGPOWCLMINER_H

#include "CLMiner.h"
#include <libethcore/ProgPowAux.h>
#include <libprogpow/ProgPow.h>


namespace dev
{
namespace eth
{

class ProgPowCLMiner : public CLMiner
{
public:

    ProgPowCLMiner(unsigned _index, CLSettings _settings, DeviceDescriptor& _device);
    ~ProgPowCLMiner() override;

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

    /// The local work size for the search
    static unsigned s_workgroupSize;
    /// The initial global work size for the searches
    static unsigned s_initialGlobalWorkSize;

};

}  // namespace eth
}  // namespace dev


#endif  // ETHMINER_PROGPOWCLMINER_H
