/*
 * Copyright (c) 2017-2018 ARM Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "arm_compute/core/Types.h"
#include "arm_compute/runtime/NEON/functions/NEDepthConcatenateLayer.h"
#include "arm_compute/runtime/Tensor.h"
#include "arm_compute/runtime/TensorAllocator.h"
#include "tests/NEON/Accessor.h"
#include "tests/datasets/ShapeDatasets.h"
#include "tests/framework/Asserts.h"
#include "tests/framework/Macros.h"
#include "tests/framework/datasets/Datasets.h"
#include "tests/validation/Validation.h"
#include "tests/validation/fixtures/DepthConcatenateLayerFixture.h"

namespace arm_compute
{
namespace test
{
namespace validation
{
TEST_SUITE(NEON)
TEST_SUITE(DepthConcatenateLayer)

// *INDENT-OFF*
// clang-format off
DATA_TEST_CASE(Validate, framework::DatasetMode::ALL, zip(zip(zip(
        framework::dataset::make("InputInfo1", {  TensorInfo(TensorShape(23U, 27U, 5U), 1, DataType::F32), // Mismatching data type input/output
                                                  TensorInfo(TensorShape(24U, 27U, 4U), 1, DataType::F32), // Mismatching x dimension
                                                  TensorInfo(TensorShape(23U, 27U, 3U), 1, DataType::F32), // Mismatching total depth
                                                  TensorInfo(TensorShape(16U, 27U, 6U), 1, DataType::F32)
        }),
        framework::dataset::make("InputInfo2", {  TensorInfo(TensorShape(23U, 27U, 4U), 1, DataType::F32),
                                                  TensorInfo(TensorShape(23U, 27U, 5U), 1, DataType::F32),
                                                  TensorInfo(TensorShape(23U, 27U, 4U), 1, DataType::F32),
                                                  TensorInfo(TensorShape(16U, 27U, 6U), 1, DataType::F32)
        })),
                                                              framework::dataset::make("OutputInfo", {  TensorInfo(TensorShape(23U, 27U, 9U), 1, DataType::F16),
                                                                                                        TensorInfo(TensorShape(25U, 12U, 9U), 1, DataType::F32),
                                                                                                        TensorInfo(TensorShape(23U, 27U, 8U), 1, DataType::F32),
                                                                                                        TensorInfo(TensorShape(16U, 27U, 12U), 1, DataType::F32)
                                                              })),
                                                          framework::dataset::make("Expected", { false, false, false, true })),
               input_info1, input_info2, output_info,expected)
{
    std::vector<TensorInfo> inputs_vector_info;
    inputs_vector_info.emplace_back(std::move(input_info1));
    inputs_vector_info.emplace_back(std::move(input_info2));

    std::vector<ITensorInfo *> inputs_vector_info_raw;
    for(auto &input : inputs_vector_info)
    {
        inputs_vector_info_raw.emplace_back(&input);
    }

    bool is_valid = bool(NEDepthConcatenateLayer::validate(inputs_vector_info_raw,
                                                           &output_info.clone()->set_is_resizable(false)));
    ARM_COMPUTE_EXPECT(is_valid == expected, framework::LogLevel::ERRORS);
}
// clang-format on
// *INDENT-ON*

TEST_CASE(Configuration, framework::DatasetMode::ALL)
{
    // Create tensors
    Tensor src1 = create_tensor<Tensor>(TensorShape(32U, 32U, 128U), DataType::F32, 1);
    Tensor src2 = create_tensor<Tensor>(TensorShape(32U, 32U, 32U), DataType::F32, 1);
    Tensor dst;

    ARM_COMPUTE_EXPECT(src1.info()->is_resizable(), framework::LogLevel::ERRORS);
    ARM_COMPUTE_EXPECT(src2.info()->is_resizable(), framework::LogLevel::ERRORS);
    ARM_COMPUTE_EXPECT(dst.info()->is_resizable(), framework::LogLevel::ERRORS);

    // Create and configure function
    NEDepthConcatenateLayer concat_layer;

    concat_layer.configure({ &src1, &src2 }, &dst);

    // Validate valid region
    const ValidRegion valid_region = shape_to_valid_region(TensorShape(32U, 32U, 160U));
    validate(dst.info()->valid_region(), valid_region);
}

template <typename T>
using NEDepthConcatenateLayerFixture = DepthConcatenateLayerValidationFixture<Tensor, ITensor, Accessor, NEDepthConcatenateLayer, T>;

TEST_SUITE(Float)
#ifdef __ARM_FEATURE_FP16_VECTOR_ARITHMETIC
TEST_SUITE(FP16)
FIXTURE_DATA_TEST_CASE(RunSmall, NEDepthConcatenateLayerFixture<half>, framework::DatasetMode::PRECOMMIT, combine(datasets::Small2DShapes(), framework::dataset::make("DataType",
                                                                                                                  DataType::F16)))
{
    // Validate output
    validate(Accessor(_target), _reference);
}
FIXTURE_DATA_TEST_CASE(RunLarge, NEDepthConcatenateLayerFixture<half>, framework::DatasetMode::NIGHTLY, combine(datasets::DepthConcatenateLayerShapes(), framework::dataset::make("DataType",
                                                                                                                DataType::F16)))
{
    // Validate output
    validate(Accessor(_target), _reference);
}
TEST_SUITE_END()
#endif /* __ARM_FEATURE_FP16_VECTOR_ARITHMETIC */

TEST_SUITE(FP32)
FIXTURE_DATA_TEST_CASE(RunSmall, NEDepthConcatenateLayerFixture<float>, framework::DatasetMode::PRECOMMIT, combine(datasets::Small2DShapes(), framework::dataset::make("DataType",
                                                                                                                   DataType::F32)))
{
    // Validate output
    validate(Accessor(_target), _reference);
}
FIXTURE_DATA_TEST_CASE(RunLarge, NEDepthConcatenateLayerFixture<float>, framework::DatasetMode::NIGHTLY, combine(datasets::DepthConcatenateLayerShapes(), framework::dataset::make("DataType",
                                                                                                                 DataType::F32)))
{
    // Validate output
    validate(Accessor(_target), _reference);
}
TEST_SUITE_END()
TEST_SUITE_END()

TEST_SUITE(Quantized)
TEST_SUITE(QASYMM8)
FIXTURE_DATA_TEST_CASE(RunSmall, NEDepthConcatenateLayerFixture<uint8_t>, framework::DatasetMode::PRECOMMIT, combine(datasets::Small2DShapes(), framework::dataset::make("DataType",
                                                                                                                     DataType::QASYMM8)))
{
    // Validate output
    validate(Accessor(_target), _reference);
}
FIXTURE_DATA_TEST_CASE(RunLarge, NEDepthConcatenateLayerFixture<uint8_t>, framework::DatasetMode::NIGHTLY, combine(datasets::DepthConcatenateLayerShapes(), framework::dataset::make("DataType",
                                                                                                                   DataType::QASYMM8)))
{
    // Validate output
    validate(Accessor(_target), _reference);
}
TEST_SUITE_END()
TEST_SUITE_END()

TEST_SUITE_END()
TEST_SUITE_END()
} // namespace validation
} // namespace test
} // namespace arm_compute