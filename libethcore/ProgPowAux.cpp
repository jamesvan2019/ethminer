

#include "ProgPowAux.h"
#include <libethash-legacy/internal.h>
#include <ethash/keccak.hpp>

using namespace std;
using namespace chrono;
using namespace dev;
using namespace eth;

ProgPowAux& ProgPowAux::get()
{
    static ProgPowAux instance;
    return instance;
}

template<unsigned N> inline h256 sha3(FixedHash<N> const& _input) {
    auto hash = ethash::keccak256(_input.data(),_input.size);
    return h256{hash.bytes, h256::ConstructFromPointer};
}


int ProgPowAux::toEpoch(h256 const& _seedHash)
{
    ProgPowAux& ethash = ProgPowAux::get();

    if (_seedHash != ethash.m_cached_seed)
    {
        // Find epoch number corresponding to given seed hash.
        int epoch = 0;

        for (h256 h; h != _seedHash && epoch < 2048; ++epoch, h = sha3(h))
        {

        }
        if (epoch == 2048)
        {
            std::ostringstream error;
            error << "apparent epoch number for " << _seedHash << " is too high; max is " << 2048;
            throw std::invalid_argument(error.str());
        }

        ethash.m_cached_seed = _seedHash;
        ethash.m_cached_epoch = epoch;
    }

    return ethash.m_cached_epoch;
}

ProgPowAux::LightType ProgPowAux::light(int epoch)
{
    // TODO: Use epoch number instead of seed hash?

    ProgPowAux& ethash = ProgPowAux::get();

    Guard l(ethash.x_lights);

    auto it = ethash.m_lights.find(epoch);
    if (it != ethash.m_lights.end())
        return it->second;

    return (ethash.m_lights[epoch] = make_shared<LightAllocation>(epoch));
}

ProgPowAux::LightAllocation::LightAllocation(int epoch)
{
    int blockNumber = epoch * LEGACY_ETHASH_EPOCH_LENGTH;
    light = ethash_light_new(blockNumber);
    size = ethash_get_cachesize(blockNumber);
}

ProgPowAux::LightAllocation::~LightAllocation()
{
    ethash_light_delete(light);
}

bytesConstRef ProgPowAux::LightAllocation::data() const
{
    return bytesConstRef((byte const*)light->cache, size);
}

Result ProgPowAux::LightAllocation::compute(h256 const& _headerHash, uint64_t _nonce) const
{
    ethash_return_value r = ethash_light_compute(light, *(ethash_h256_t*)_headerHash.data(), _nonce);
    if (!r.success)
        BOOST_THROW_EXCEPTION(DAGCreationFailure());
    return Result{h256((uint8_t*)&r.result, h256::ConstructFromPointer), h256((uint8_t*)&r.mix_hash, h256::ConstructFromPointer)};
}

Result ProgPowAux::eval(int epoch, h256 const& _headerHash, uint64_t _nonce) noexcept
{
    try
    {
        return get().light(epoch)->compute(_headerHash, _nonce);
    }
    catch(...)
    {
        return Result{~h256(), h256()};
    }
}
