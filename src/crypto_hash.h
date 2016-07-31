/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/ */
// Written in 2014 by Nils Maier

#ifndef CRYPTO_HASH_H
#define CRYPTO_HASH_H

#include <cstdint>
#include <cstring>
#include <memory>
#include <set>
#include <string>
#include <limits>

namespace crypto {
namespace hash {

enum Algorithms {
  algoNone = 0x0,
  algoMD5 = 0x1,
  algoSHA1 = 0x2,
  algoSHA224 = 0x3,
  algoSHA256 = 0x4,
  algoSHA384 = 0x5,
  algoSHA512 = 0x6,
};

class Algorithm {
public:
  Algorithm() = default;

  virtual ~Algorithm() = default;

  virtual void update(const void* data, uint64_t len) = 0;

  inline void update(const std::string& data)
  {
    return update(data.data(), data.length());
  }

  virtual std::string finalize() = 0;

  virtual void reset() = 0;

  virtual uint_fast16_t length() const = 0;

  virtual uint_fast16_t blocksize() const = 0;

private:
  Algorithm(const Algorithm&) = delete;

  Algorithm& operator=(const Algorithm&) = delete;
};

const std::set<std::string>& all();

Algorithms lookup(const std::string& name);

std::unique_ptr<Algorithm> create(Algorithms algo);

inline std::unique_ptr<Algorithm> create(const std::string& name)
{
  return create(lookup(name));
}

inline uint_fast16_t length(Algorithms algo) { return create(algo)->length(); }

inline uint_fast16_t length(const std::string& name)
{
  return create(name)->length();
}

inline std::string compute(Algorithms algo, const void* data, uint_fast64_t len)
{
  auto ctx = create(algo);
  ctx->update(data, len);
  return ctx->finalize();
}

inline std::string compute(Algorithms algo, const std::string& data)
{
  return compute(algo, data.data(), data.length());
}

inline std::string compute(const std::string& name, const void* data,
                           uint_fast64_t len)
{
  return compute(lookup(name), data, len);
}

inline std::string compute(const std::string& name, const std::string& data)
{
  return compute(lookup(name), data.data(), data.length());
}

} // namespace hash
} // namespace crypto

#endif // CRYPTO_HASH_H
