// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "gtest/gtest.h"
#include "test/providers/provider_test_utils.h"
#include "test/common/cuda_op_test_utils.h"

namespace onnxruntime {
namespace test {

namespace {
// Returns a vector of `count` values which start at `start` and change by increments of `step`.
template <typename T>
std::vector<T> ValueRange(
    size_t count, T start = static_cast<T>(0), T step = static_cast<T>(1)) {
  std::vector<T> result;
  result.reserve(count);
  T curr = start;
  for (size_t i = 0; i < count; ++i) {
    result.emplace_back(curr);
    curr += step;
  }
  return result;
}
}  // namespace


#ifdef USE_CUDA
TEST(GatherNDGradOpTest, GatherNDGrad_slice_float_int64_t_batch_dims_1) {
  OpTester test("GatherNDGrad", 1, kMSDomain);
  test.AddAttribute<int64_t>("batch_dims", 0);
  test.AddInput<int64_t>("shape", {3}, {2LL, 2LL, 3LL});
  test.AddInput<int64_t>("indices", {2, 2}, {0LL, 1LL, 1LL, 0LL});
  test.AddInput<float>("update", {2, 3}, ValueRange(6, 1.0f));
  test.AddOutput<float>("output", {2, 2, 3}, {0, 0, 0, 1, 2, 3, 4, 5, 6, 0, 0, 0});
  test.Run();
}

TEST(GatherNDGradOpTest, GatherNDGrad_slice_double_int32_t_batch_dims_3) {
  if (!HasCudaEnvironment(600 /*min_cuda_architecture*/)) return;

  OpTester test("GatherNDGrad", 1, kMSDomain);
  test.AddAttribute<int64_t>("batch_dims", 1);
  test.AddInput<int64_t>("shape", {3}, {2LL, 2LL, 3LL});
  test.AddInput<int64_t>("indices", {2, 1, 1}, {1LL, 0LL});
  test.AddInput<double>("update", {2, 3}, ValueRange(6, 1.0));
  test.AddOutput<double>("output", {2, 2, 3}, {0, 0, 0, 1, 2, 3, 4, 5, 6, 0, 0, 0});
  test.Run();
}


TEST(GatherNDGradOpTest, GatherNDGrad_slice_half_int32_t_batch_dims_3) {
  if (!HasCudaEnvironment(600 /*min_cuda_architecture*/)) return;

  OpTester test("GatherNDGrad", 1, kMSDomain);
  test.AddAttribute<int64_t>("batch_dims", 1);
  test.AddInput<int64_t>("shape", {3}, {2LL, 2LL, 3LL});
  test.AddInput<int64_t>("indices", {2, 1, 1}, {1LL, 0LL});
  std::vector<float> updates_f = ValueRange(6, 1.0f);
  std::vector<float> outputs_f({0, 0, 0, 1, 2, 3, 4, 5, 6, 0, 0, 0});
  std::vector<MLFloat16> updates(6);
  std::vector<MLFloat16> outputs(12);
  ConvertFloatToMLFloat16(updates_f.data(), updates.data(), 6);
  ConvertFloatToMLFloat16(outputs_f.data(), outputs.data(), 12);
  test.AddInput<MLFloat16>("update", {2, 3}, updates);
  test.AddOutput<MLFloat16>("output", {2, 2, 3}, outputs);
  test.Run();
}

TEST(GatherNDGradOpTest, GatherNDGrad_batch_dims_of_2) {
  OpTester test("GatherNDGrad", 1, kMSDomain);
  test.AddAttribute<int64_t>("batch_dims", 2);
  test.AddInput<int64_t>("shape", {4}, {2, 2, 2, 3});
  test.AddInput<int64_t>(
      "indices", {2, 2, 1},
      {
          1,  // batch 0
          1,  // batch 1
          0,  // batch 2
          1,  // batch 3
      });
  test.AddInput<float>("update", {2, 2, 3}, ValueRange<float>(12));
  test.AddOutput<float>(
      "output", {2, 2, 2, 3},
      {
          0, 0, 0, 0, 1, 2,    // batch 0
          0, 0, 0, 3, 4, 5,    // batch 1
          6, 7, 8, 0, 0, 0,    // batch 2
          0, 0, 0, 9, 10, 11,  // batch 3
      });
  test.Run();
}
#endif

}  // namespace test
}  // namespace onnxruntime
