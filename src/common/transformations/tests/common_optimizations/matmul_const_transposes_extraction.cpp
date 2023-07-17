// Copyright (C) 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>

#include <ngraph/function.hpp>
#include <ngraph/opsets/opset8.hpp>
#include <ngraph/pass/manager.hpp>
#include <openvino/pass/constant_folding.hpp>
#include <transformations/common_optimizations/matmul_const_transposes_extraction.hpp>

#include "common_test_utils/ngraph_test_utils.hpp"

using namespace ngraph;

TEST_F(TransformationTestsF, MatMulConstTransposesExtractionConstantWeights) {
    {
        auto data = std::make_shared<opset8::Parameter>(element::f32, Shape{1, 3, 4});
        auto weights = opset8::Constant::create(element::f32, Shape{1, 3, 2}, {1, 2, 3, 4, 5, 6});
        auto matmul = std::make_shared<opset8::MatMul>(data, weights, true);
        function = std::make_shared<Function>(matmul, ParameterVector{data});

        manager.register_pass<ov::pass::MatMulConstTransposesExtraction>();
        manager.register_pass<ov::pass::ConstantFolding>();
    }

    {
        auto data = std::make_shared<opset8::Parameter>(element::f32, Shape{1, 3, 4});
        auto weights = opset8::Constant::create(element::f32, Shape{1, 2, 3}, {1, 3, 5, 2, 4, 6});
        auto matmul = std::make_shared<opset8::MatMul>(data, weights, true, true);
        function_ref = std::make_shared<Function>(matmul, ParameterVector{data});
    }
    comparator.enable(FunctionsComparator::CmpValues::CONST_VALUES);
    comparator.enable(FunctionsComparator::CmpValues::ACCURACY);
}

TEST_F(TransformationTestsF, MatMulConstTransposesExtractionFQOnWeights) {
    {
        auto data = std::make_shared<opset8::Parameter>(element::f32, Shape{1, 4, 3});
        auto weights = opset8::Constant::create(element::f32, Shape{1, 3, 2}, {1, 2, 3, 4, 5, 6});
        auto low = opset8::Constant::create(element::f32, Shape{1}, {0});
        auto high = opset8::Constant::create(element::f32, Shape{1}, {10});
        auto fq = std::make_shared<opset8::FakeQuantize>(weights, low, high, low, high, 255);
        auto matmul = std::make_shared<opset8::MatMul>(data, fq);
        function = std::make_shared<Function>(matmul, ParameterVector{data});

        manager.register_pass<ov::pass::MatMulConstTransposesExtraction>();
        manager.register_pass<ov::pass::ConstantFolding>();
    }

    {
        auto data = std::make_shared<opset8::Parameter>(element::f32, Shape{1, 4, 3});
        auto weights = opset8::Constant::create(element::f32, Shape{1, 3, 2}, {1, 2, 3, 4, 5, 6});
        auto low = opset8::Constant::create(element::f32, Shape{1}, {0});
        auto high = opset8::Constant::create(element::f32, Shape{1}, {10});
        auto fq = std::make_shared<opset8::FakeQuantize>(weights, low, high, low, high, 255);
        auto transpose =
            std::make_shared<opset8::Transpose>(fq, op::Constant::create(element::i32, Shape{3}, {0, 2, 1}));
        auto matmul = std::make_shared<opset8::MatMul>(data, transpose, false, true);
        function_ref = std::make_shared<Function>(matmul, ParameterVector{data});
    }
    comparator.enable(FunctionsComparator::CmpValues::CONST_VALUES);
    comparator.enable(FunctionsComparator::CmpValues::ACCURACY);
}

TEST_F(TransformationTestsF, NegativeMatMulConstTransposesExtractionInvalidRank) {
    auto data = std::make_shared<opset8::Parameter>(element::f32, Shape{1, 3, 4});
    auto weights = opset8::Constant::create(element::f32, Shape{3}, {1, 2, 3});
    auto matmul = std::make_shared<opset8::MatMul>(data, weights, true);
    function = std::make_shared<Function>(matmul, ParameterVector{data});
    manager.register_pass<ov::pass::MatMulConstTransposesExtraction>();
    comparator.enable(FunctionsComparator::CmpValues::CONST_VALUES);
}

TEST_F(TransformationTestsF, NegativeMatMulConstTransposesExtractionTransposeBSet) {
    auto data = std::make_shared<opset8::Parameter>(element::f32, Shape{1, 3, 4});
    auto weights = opset8::Constant::create(element::f32, Shape{1, 2, 3}, {1, 2, 3, 4, 5, 6});
    auto matmul = std::make_shared<opset8::MatMul>(data, weights, true, true);
    function = std::make_shared<Function>(matmul, ParameterVector{data});
    manager.register_pass<ov::pass::MatMulConstTransposesExtraction>();
    comparator.enable(FunctionsComparator::CmpValues::CONST_VALUES);
}

TEST_F(TransformationTestsF, NegativeMatMulConstTransposesExtractionNonUnitDims) {
    auto data = std::make_shared<opset8::Parameter>(element::f32, Shape{1, 3, 4});
    auto weights = opset8::Constant::create(element::f32, Shape{2, 3, 2}, {1, 2, 3, 4, 5, 6, 2, 3, 4, 5, 6, 7});
    auto matmul = std::make_shared<opset8::MatMul>(data, weights, true);
    function = std::make_shared<Function>(matmul, ParameterVector{data});
    manager.register_pass<ov::pass::MatMulConstTransposesExtraction>();
    comparator.enable(FunctionsComparator::CmpValues::CONST_VALUES);
}
