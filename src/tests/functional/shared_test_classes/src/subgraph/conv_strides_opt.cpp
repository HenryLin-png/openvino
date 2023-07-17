// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "shared_test_classes/subgraph/conv_strides_opt.hpp"

namespace SubgraphTestsDefinitions {

using namespace ngraph;

std::string ConvStridesOpt::getTestCaseName(const testing::TestParamInfo<ConvStridesOptParams> &obj) {
    Shape input_shape;
    op::PadType pad;
    std::string targetName;
    std::tie(input_shape, pad, targetName) = obj.param;
    std::ostringstream results;

    results << "inputShape=" << input_shape << "_";
    results << "padType=" << pad << "_";
    results << "targetDevice=" << targetName;
    return results.str();
}

void ConvStridesOpt::SetUp() {
    Shape input_shape;
    op::PadType pad_type;
    std::tie(input_shape, pad_type, targetDevice) = this->GetParam();
    auto param = std::make_shared<opset4::Parameter>(element::f32, input_shape);
    auto C = input_shape[1];
    auto weights1 = ngraph::builder::makeConstant<float>(element::f32, {C, C, 3, 3}, {}, true);
    auto spatial_dims = input_shape.size() - 2;
    Strides strides1(spatial_dims, 1);
    Strides dilations(spatial_dims, 1);
    CoordinateDiff pad_begin1(spatial_dims, 1), pad_end1(spatial_dims, 1);
    auto conv1 = std::make_shared<opset4::Convolution>(param, weights1, strides1, pad_begin1, pad_end1,
            dilations, pad_type);
    auto weights2 = ngraph::builder::makeConstant<float>(element::f32, {C, C, 1, 1}, {}, true);
    CoordinateDiff pad_begin2(spatial_dims, 0), pad_end2(spatial_dims, 0);
    Strides strides2(spatial_dims, 2);
    auto conv2 = std::make_shared<opset4::Convolution>(conv1, weights2, strides2, pad_begin2, pad_end2,
            dilations);
    function = std::make_shared<Function>(OutputVector{conv2}, ParameterVector{param});
}
} // namespace SubgraphTestsDefinitions
