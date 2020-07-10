
#pragma once

#include "BlockHeader.h"
#include "EthashAux.h"
#include "Exceptions.h"
#include <libdevcore/Log.h>
#include <libdevcore/Worker.h>
#include <libethash-legacy/legacy_ethash.h>
#include <libethash-legacy/internal.h>
#include <condition_variable>

namespace dev
{
namespace eth
{

class ProgPowAux
{
public:
    struct LightAllocation
    {
        explicit LightAllocation(int epoch);
        ~LightAllocation();
        bytesConstRef data() const;
        Result compute(h256 const& _headerHash, uint64_t _nonce) const;
        ethash_light_t light;
        uint64_t size;
    };

    using LightType = std::shared_ptr<LightAllocation>;

    static int toEpoch(h256 const& _seedHash);

    static LightType light(int epoch);

    static Result eval(int epoch, h256 const& _headerHash, uint64_t  _nonce) noexcept;

private:
    ProgPowAux() = default;
    static ProgPowAux& get();

    Mutex x_lights;
    std::map<int, LightType> m_lights;

    int m_cached_epoch = 0;
    h256 m_cached_seed;  // Seed for epoch 0 is the null hash.
};

struct WorkPackageProgPow
{
    WorkPackageProgPow() = default;
    explicit WorkPackageProgPow(BlockHeader const& _bh)
        : boundary(_bh.boundary()),
          header(_bh.hashWithout()),
          epoch(static_cast<int>(_bh.number()) / LEGACY_ETHASH_EPOCH_LENGTH),
          height(static_cast<uint64_t>(_bh.number()))
    {}
    explicit operator bool() const { return header != h256(); }

    h256 boundary;
    h256 header;  ///< When h256() means "pause until notified a new work package is available".
    h256 job;
    int epoch = -1;

    uint64_t startNonce = 0;
    uint64_t height = 0;
    int exSizeBits = -1;
    int job_len = 8;
};

struct SolutionProgPow
{
    uint64_t nonce;
    h256 mixHash;
    WorkPackageProgPow work;
    bool stale;
};

}  // namespace eth
}  // namespace dev
