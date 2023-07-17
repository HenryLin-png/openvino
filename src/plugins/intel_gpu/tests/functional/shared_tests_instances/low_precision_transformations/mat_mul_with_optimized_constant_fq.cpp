// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <vector>

#include "low_precision_transformations/mat_mul_with_optimized_constant_fq.hpp"
#include "common_test_utils/test_constants.hpp"

using namespace LayerTestsDefinitions;

namespace {
const std::vector<ngraph::element::Type> netPrecisions = {
    ngraph::element::f32,
    ngraph::element::f16
};

const std::vector<LayerTestsDefinitions::MatMulWithOptimizedConstantFakeQuantizeTransformationTestValues> params = {
    {
        { 256ul, ngraph::Shape { 1 }, { 0.f }, { 25.5f }, { 0.f }, { 25.5f } },
        { 255ul, ngraph::Shape { 1 }, { -12.7f }, { 12.7f }, { -12.7f }, { 12.7f } }
    },
};

const std::vector<std::pair<ngraph::PartialShape, ngraph::Shape>> inputShapes = {
    {{ 1, 16 }, { 10, 16 }},
    {{ 1, 16 }, { 16, 10 }}
};

INSTANTIATE_TEST_SUITE_P(smoke_LPT, MatMulWithOptimizedConstantFq,
    ::testing::Combine(
        ::testing::ValuesIn(netPrecisions),
        ::testing::ValuesIn(inputShapes),
        ::testing::Values(CommonTestUtils::DEVICE_GPU),
        ::testing::ValuesIn(params)),
    MatMulWithOptimizedConstantFq::getTestCaseName);
}  // namespace
