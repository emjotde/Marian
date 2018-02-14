/* All or part of this file was contributed by Intel under license:
 *   Copyright (C) 2017-2018 Intel Corporation
 *   SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <algorithm>
#include <iterator>
#include <random>
#include <thrust/functional.h>

#include "kernels/tensor_operators.h"
#include "layers/word2vec_reader.h"
#include "layers/param_initializers.h"

namespace marian {

namespace inits {

using namespace thrust::placeholders;

float xor128() {
  static uint64_t x = 123456789;
  static uint64_t y = 362436069;
  static uint64_t z = 521288629;
  static uint64_t w = 88675123;
  uint64_t t;

  t = (x ^ (x << 11)) % 1000;
  x = y;
  y = z;
  z = w;
  w = (w ^ (w >> 19) ^ t ^ (t >> 8)) % 1000;
  return 0.1 * ((w % 1000) / 1000.f) - 0.05;
}

void zeros(Tensor t) {
  t->set(0.f);
}

void ones(Tensor t) {
  t->set(1.0f);
}

std::function<void(Tensor)> from_value(float v) {
  return [v](Tensor t) { t->set(v); };
}

std::function<void(Tensor)> diag(float val) {
  return [val](Tensor t) {
    if(t->shape()[0] == t->shape()[1] && t->shape()[2] == 1
       && t->shape()[3] == 1) {
      std::vector<float> vec(t->size(), 0);
      for(int i = 0; i < t->shape()[0]; ++i)
        vec[i * t->shape()[1] + i] = val;
      t->set(vec);
    }
  };
}

std::function<void(Tensor)> normal(float scale) {
  return [scale](Tensor t) {
    distribution<std::normal_distribution<float>>(t, 0, scale);
  };
}

std::function<void(Tensor)> uniform(float scale) {
  return [scale](Tensor t) {
    distribution<std::uniform_real_distribution<float>>(t, -scale, scale);
  };
}

void glorot_uniform(Tensor t) {
  float scale = sqrtf(6.0f / (t->shape()[0] + t->shape()[1]));
  distribution<std::uniform_real_distribution<float>>(t, -scale, scale);
}

void xorshift(Tensor t) {
  std::vector<float> vals(t->size());
  for(auto&& v : vals)
    v = xor128();
  t << vals;
}

void glorot_normal(Tensor t) {
  float scale = sqrtf(2.0f / (t->shape()[0] + t->shape()[1]));
  distribution<std::normal_distribution<float>>(t, 0, scale);
}

std::function<void(Tensor)> from_vector(const std::vector<float>& v) {
  return [v](Tensor t) { t->set(v); };
}

std::function<void(Tensor)> from_vector(const std::vector<size_t>& v) {
  std::vector<float> vf(v.begin(), v.end());
  return from_vector(vf);
}

std::function<void(Tensor)> from_sparse_vector(
    std::pair<std::vector<size_t>, std::vector<float>>& v) {
  return [v](Tensor t) {
    t->set(1e-6);
    t->setSparse(v.first, v.second);
  };
}

std::function<void(Tensor)> from_numpy(const cnpy::NpyArray& np) {
  size_t size = 1;
  for(size_t i = 0; i < np.shape.size(); ++i) {
    size *= np.shape[i];
  };

  std::vector<float> npv(size);
  std::copy((float*)np.data, (float*)np.data + size, npv.begin());

  return [npv](Tensor t) { t->set(npv); };
}

std::function<void(Tensor)> from_word2vec(const std::string& file,
                                          int dimVoc,
                                          int dimEmb,
                                          bool normalize /*= false*/) {
  return [file, dimVoc, dimEmb, normalize](Tensor t) {
    auto embs = Word2VecReader().read(file, dimVoc, dimEmb);
    t->set(embs);
    if(normalize){
      float l2Norm = L2Norm(t);
      if(l2Norm != 0)
        Element(_1 = _1 / l2Norm, t);
    }
  };
}
}

}  // namespace marian
