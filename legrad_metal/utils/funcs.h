#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "boost/functional/hash.hpp"
#include "macros/expr.h"

namespace utils
{
LEGRAD_INLINE std::string read_file(const std::string& path)
{
  std::ifstream file(path);

  if (!file) {
    throw std::runtime_error("cannot read file");
  }

  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  file.ignore(std::numeric_limits<std::streamsize>::max());

  try {
    auto size = file.gcount();

    if (size > 0x10000)  // 64kib sanity check for shaders:
      return std::string();

    file.clear();
    file.seekg(0, std::ios_base::beg);

    std::stringstream sstr;
    sstr << file.rdbuf();
    file.close();

    return sstr.str();
  } catch (const std::ifstream::failure& e) {
    throw std::runtime_error("cannot read file: " + path + " (" + e.what()
                             + ")");
  }
}

/*
 * We cannot use std::pair as std::unordered_map's key directly, we need to have
 * a hash function for the key
 * https://stackoverflow.com/questions/20590656/how-to-solve-error-for-hash-function-of-pair-of-ints-in-unordered-map
 */
struct HashPairKey
{
  template <typename T, typename U>
  size_t operator()(const std::pair<T, U>& x) const
  {
    size_t seed = 0;
    boost::hash_combine(seed, x.first);
    boost::hash_combine(seed, x.second);
    return seed;
  }
};
}  // namespace utils
