/// OpenCL Ethash miner implementation.
///
/// @file
/// @copyright GNU General Public License
//

#include <libethcore/Farm.h>
#include "ProgPowCLMiner.h"

using namespace dev;
using namespace eth;

#define LOG2_MAX_MINERS 5u
#define MAX_MINERS (1u << LOG2_MAX_MINERS)

constexpr size_t c_maxSearchResults = 1;
std::chrono::high_resolution_clock::time_point workSwitchStart;

struct CLSwitchChannel: public LogChannel
{
    static const char* name() { return EthOrange " cl"; }
    static const int verbosity = 6;
    static const bool debug = false;
};
#define clswitchlog clog(CLSwitchChannel)

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

uint64_t get_start_nonce(uint _index)
{
    // Each GPU is given a non-overlapping 2^40 range to search
    return Farm::f().get_nonce_scrambler() + ((uint64_t) _index << 40);
}

void ProgPowCLMiner::workLoop()
{
    // Memory for zero-ing buffers. Cannot be static because crashes on macOS.
    uint32_t const c_zero = 0;

    uint64_t startNonce = 0;

    // The work package currently processed by GPU.
    WorkPackage current;
    current.header = h256{1u};
    uint64_t old_period_seed = -1;

    try {
        while (!shouldStop())
        {
            const WorkPackage w = work();
            uint64_t period_seed = w.block / PROGPOW_PERIOD;

            if (current.header != w.header || current.epoch != w.epoch || old_period_seed != period_seed)
            {
                // New work received. Update GPU data.
                if (!w)
                {
                    cllog << "No work. Pause for 3 s.";
                    std::this_thread::sleep_for(std::chrono::seconds(3));
                    continue;
                }

                //cllog << "New work: header" << w.header << "target" << w.boundary.hex();

                if (current.epoch != w.epoch || old_period_seed != period_seed)
                {
                    if (s_dagLoadMode == DAG_LOAD_MODE_SEQUENTIAL)
                    {
                        while (s_dagLoadIndex < m_index)
                            this_thread::sleep_for(chrono::seconds(1));
                        ++s_dagLoadIndex;
                    }

                    cllog << "New epoch " << w.epoch << "/ period " << period_seed;
                    //init(w.epoch, w.block, current.epoch != w.epoch, old_period_seed != period_seed);
                }

                // Upper 64 bits of the boundary.
                const uint64_t target = (uint64_t)(u64)((u256)w.boundary >> 192);
                assert(target > 0);

                // Update header constant buffer.
                m_queue.enqueueWriteBuffer(m_header, CL_FALSE, 0, w.header.size, w.header.data());
                m_queue.enqueueWriteBuffer(m_searchBuffer, CL_FALSE, 0, sizeof(c_zero), &c_zero);

                m_searchKernel.setArg(0, m_searchBuffer);  // Supply output buffer to kernel.
                m_searchKernel.setArg(4, target);

                // FIXME: This logic should be move out of here.
                if (w.exSizeBytes >= 0)
                {
                    // This can support up to 2^c_log2MaxMiners devices.
                    startNonce = w.startNonce | ((uint64_t)index << (64 - LOG2_MAX_MINERS - w.exSizeBytes));
                }
                else
                    startNonce = get_start_nonce(m_index);

                clswitchlog << "Switch time"
                            << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - workSwitchStart).count()
                            << "ms.";
            }

            // Read results.
            // TODO: could use pinned host pointer instead.
            uint32_t results[c_maxSearchResults + 1];
            m_queue.enqueueReadBuffer(m_searchBuffer, CL_TRUE, 0, sizeof(results), &results);

            uint64_t nonce = 0;
            if (results[0] > 0)
            {
                // Ignore results except the first one.
                nonce = current.startNonce + results[1];
                // Reset search buffer if any solution found.
                m_queue.enqueueWriteBuffer(m_searchBuffer, CL_FALSE, 0, sizeof(c_zero), &c_zero);
            }

            // Run the kernel.
            m_searchKernel.setArg(3, startNonce);
            m_queue.enqueueNDRangeKernel(m_searchKernel, cl::NullRange, m_globalWorkSize, m_workgroupSize);

            // Report results while the kernel is running.
            // It takes some time because ethash must be re-evaluated on CPU.
            if (nonce != 0) {
                Result r = EthashAux::eval(current.epoch, current.header, nonce);
                if (r.value < current.boundary)
                    Farm::f().submitProof(Solution{nonce, r.mixHash, current, std::chrono::steady_clock::now(), m_index});
                else {
                    cwarn << "FAILURE: GPU gave incorrect result!";
                }
            }

            old_period_seed = period_seed;

            current = w;        // kernel now processing newest work
            current.startNonce = startNonce;
            // Increase start nonce for following kernel execution.
            startNonce += m_globalWorkSize;

            // Report hash count
            updateHashRate(m_globalWorkSize, 1);


            // Make sure the last buffer write has finished --
            // it reads local variable.
            m_queue.finish();
        }
        m_queue.finish();
    }
    catch (cl::Error const& _e)
    {
        string _what = ethCLErrorHelper("OpenCL Error", _e);
        cwarn << _what;
        throw std::runtime_error(_what);
    }
}

