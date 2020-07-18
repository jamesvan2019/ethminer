/*
This file is part of ethminer.

ethminer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ethminer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ethminer.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 MetalMiner simulates mining devices but does NOT real mine!
 USE FOR DEVELOPMENT ONLY !
*/

#if defined(__linux__)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE /* we need sched_setaffinity() */
#endif
#include <error.h>
#include <sched.h>
#include <unistd.h>
#endif

#include <libethcore/Farm.h>
#include <ethash/ethash.hpp>

#include <boost/version.hpp>

#if 0
#include <boost/fiber/numa/pin_thread.hpp>
#include <boost/fiber/numa/topology.hpp>
#endif

#include "MetalMiner.h"
#include <mtlpp.hpp>

/* Sanity check for defined OS */
#if defined(__APPLE__) || defined(__MACOSX)
/* MACOSX */
#include <mach/mach.h>
#include <IOkit/IOTypes.h>
#include <IOkit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#else
#error "Invalid OS configuration"
#endif

using namespace std;
using namespace dev;
using namespace eth;

namespace utils
{
template< typename cf_reference_t >
class scoped_cf_ref
{
public:
    scoped_cf_ref() = default;
    scoped_cf_ref( cf_reference_t ref_ )
        : m_ref( ref_ )
    {}

    ~scoped_cf_ref()
    {
        if( m_ref )
            CFRelease( m_ref );
    }

    operator cf_reference_t& () { return m_ref; }
    operator const cf_reference_t& () const { return m_ref; }

    cf_reference_t* operator& () { return &m_ref; }

private:
    cf_reference_t m_ref;
};

template<typename property_t>
inline CFTypeID get_cf_type_id() { return 0; }

template<>
inline CFTypeID get_cf_type_id< CFStringRef >() { return
        7; }

template<>
inline CFTypeID get_cf_type_id< CFDataRef >() { return 20; }

template<>
inline CFTypeID get_cf_type_id< CFNumberRef >() { return 22; }

template<typename property_t>
property_t get_property( io_object_t service_, CFStringRef name_ )
{
    auto cfref = IORegistryEntryCreateCFProperty( service_, name_, kCFAllocatorDefault, 0 );
    if( cfref && ( CFGetTypeID( cfref ) == get_cf_type_id< property_t >() ) )
        return static_cast< property_t >( cfref );
    return NULL;
}

boost::optional< io_service_t > get_io_service_by_property( const io_iterator_t& it_, CFStringRef property_, CFStringRef name_ )
{
    while ( auto service = IOIteratorNext( it_ ) )
    {
        scoped_cf_ref< CFStringRef > ioName = static_cast< CFStringRef >(
            IORegistryEntryCreateCFProperty( service, property_, kCFAllocatorDefault, 0 )
        );
        if ( ioName && CFStringCompare(ioName, name_, 0) == kCFCompareEqualTo )
            return service;
    }
    return boost::none;
}
}

/* ######################## Metal Miner ######################## */

struct MetalMinerChannel : public LogChannel
{
    static const char* name() { return EthOrange "metal"; }
    static const int verbosity = 2;
};
#define mtllog clog(MetalMinerChannel)


MetalMiner::MetalMiner(unsigned _index, MTLSettings _settings, DeviceDescriptor& _device)
  : Miner("mtl-", _index), m_settings(_settings)
{
    m_deviceDescriptor = _device;
}


MetalMiner::~MetalMiner()
{
    DEV_BUILD_LOG_PROGRAMFLOW(mtllog, "mtl-" << m_index << " MetalMiner::~MetalMiner() begin");
    stopWorking();
    kick_miner();
    DEV_BUILD_LOG_PROGRAMFLOW(mtllog, "mtl-" << m_index << " MetalMiner::~MetalMiner() end");
}


/*
 * Bind the current thread to a spcific CPU
 */
bool MetalMiner::initDevice()
{
    DEV_BUILD_LOG_PROGRAMFLOW(mtllog, "mtl-" << m_index << " CPUMiner::initDevice begin");
    mtllog << "Using : " << m_deviceDescriptor.uniqueId << " " << m_deviceDescriptor.mtlName
           << " Memory : " << dev::getFormattedMemory((double)m_deviceDescriptor.totalMemory);

    DEV_BUILD_LOG_PROGRAMFLOW(mtllog, "mtl-" << m_index << " CPUMiner::initDevice end");
    return true;
}


/*
 * A new epoch was receifed with last work package (called from Miner::initEpoch())
 *
 * If we get here it means epoch has changed so it's not necessary
 * to check again dag sizes. They're changed for sure
 * We've all related infos in m_epochContext (.dagSize, .dagNumItems, .lightSize, .lightNumItems)
 */
bool MetalMiner::initEpoch_internal()
{
    return true;
}


/*
   Miner should stop working on the current block
   This happens if a
     * new work arrived                       or
     * miner should stop (eg exit ethminer)   or
     * miner should pause
*/
void MetalMiner::kick_miner()
{
    m_new_work.store(true, std::memory_order_relaxed);
    m_new_work_signal.notify_one();
}

/*
 * The main work loop of a Worker thread
 */
void MetalMiner::workLoop()
{
    // DEV_BUILD_LOG_PROGRAMFLOW(mtllog, "mtl-" << m_index << " MetalMiner::workLoop() begin");

    WorkPackage current;
    current.header = h256();

    if (!initDevice())
        return;

    while (!shouldStop())
    {
        // Wait for work or 3 seconds (whichever the first)
        const WorkPackage w = work();
        if (!w)
        {
            boost::system_time const timeout =
                boost::get_system_time() + boost::posix_time::seconds(3);
            boost::mutex::scoped_lock l(x_work);
            m_new_work_signal.timed_wait(l, timeout);
            continue;
        }

        if (w.algo == "ethash")
        {
            // Epoch change ?
            if (current.epoch != w.epoch)
            {
                if (!initEpoch())
                    break;  // This will simply exit the thread

                // As DAG generation takes a while we need to
                // ensure we're on latest job, not on the one
                // which triggered the epoch change
                current = w;
                continue;
            }

            // Persist most recent job.
            // Job's differences should be handled at higher level
            current = w;

            // Start searching
            search(w);
        }
        else
        {
            throw std::runtime_error("Algo : " + w.algo + " not yet implemented");
        }
    }

    DEV_BUILD_LOG_PROGRAMFLOW(mtllog, "mtl-" << m_index << " MetalMiner::workLoop() end");
}

void MetalMiner::search(const dev::eth::WorkPackage& w) {
    DEV_BUILD_LOG_PROGRAMFLOW(mtllog, "mtl-" << m_index << "work block" << w.block  << " MetalMiner::search() begin") ;
    DEV_BUILD_LOG_PROGRAMFLOW(mtllog, "mtl-" << m_index << " MetalMiner::search() end") ;
}

void MetalMiner::enumDevices(std::map<string, DeviceDescriptor>& _DevicesCollection)
{
    DEV_BUILD_LOG_PROGRAMFLOW(mtllog, "MetalMiner::enumDevices begin");

    ns::Array<mtlpp::Device> devices = mtlpp::Device::CopyAllDevices();

    for (uint i = 0 ; i< devices.GetSize(); i++)
    {
        string uniqueId;
        std::ostringstream s;
        DeviceDescriptor deviceDescriptor;

        mtlpp::Device d = devices[i];
        s << "mtl-" << setfill('0') << setw(4) << hex << (unsigned int)d.GetRegistryID();
        uniqueId = s.str();

        if (_DevicesCollection.find(uniqueId) != _DevicesCollection.end())
            deviceDescriptor = _DevicesCollection[uniqueId];
        else
            deviceDescriptor = DeviceDescriptor();

        const ns::String name =  d.GetName();
        const int currentAllocatedSize = d.GetCurrentAllocatedSize();
        const uint maxWorkingSize = d.GetRecommendedMaxWorkingSetSize();

        DEV_BUILD_LOG_PROGRAMFLOW(mtllog, "device " << name.GetCStr() << " id=" << uniqueId
            << " currentAllocatedSize =" <<  currentAllocatedSize
            << " maxWorkingSize =" << maxWorkingSize);

        deviceDescriptor.name = name.GetCStr();
        deviceDescriptor.mtlDetected = true;
        deviceDescriptor.uniqueId = uniqueId;
        deviceDescriptor.type = DeviceTypeEnum::Gpu;
        deviceDescriptor.mtlDeviceIndex = i;
        deviceDescriptor.mtlDeviceOrdinal = i;
        deviceDescriptor.mtlName = name.GetCStr();
        if (maxWorkingSize > 0)  // metal's maxWorkingSize only works for integrated GPU
        {
            deviceDescriptor.totalMemory = maxWorkingSize;
        }else{ // For PCI GPU device
            io_iterator_t iterator;
            if (IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IOPCIDevice"), &iterator)==0) {
                while ( auto service = utils::get_io_service_by_property( iterator, CFSTR( "IOName" ), CFSTR("display") ) )
                {
                    // use 'model' property as the GPU device's name
                    utils::scoped_cf_ref< CFDataRef > model = utils::get_property< CFDataRef >( service.value(), CFSTR("model") );
                    auto _length = CFDataGetLength(model);
                    std::string _model_name;
                    _model_name.resize(_length);
                    CFDataGetBytes(model,CFRangeMake(0,_length), (UInt8*)_model_name.data());
                    // test if devices' model is match with mtlName.
                    // TODO, should use more precisely globle property like device id to do the test.
                    if ( deviceDescriptor.mtlName.find(_model_name) != std::string::npos )
                    {
                        utils::scoped_cf_ref<CFNumberRef> memsize =
                            utils::get_property<CFNumberRef>(service.value(), CFSTR("ATY,memsize"));
                        if (memsize)
                            CFNumberGetValue(
                                memsize, kCFNumberSInt64Type, &deviceDescriptor.totalMemory);
                        CFRelease(memsize);
                    }
                }
            }
        }
        _DevicesCollection[uniqueId] = deviceDescriptor;
    }

    DEV_BUILD_LOG_PROGRAMFLOW(mtllog, "MetalMiner::enumDevices end");
}
