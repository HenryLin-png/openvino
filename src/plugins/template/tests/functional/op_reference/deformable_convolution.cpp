// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>

#include "openvino/op/deformable_convolution.hpp"
#include "base_reference_test.hpp"

using namespace reference_tests;
using namespace ov;

namespace {
struct DeformableConvolutionParams {
    template <class IT>
    DeformableConvolutionParams(const PartialShape& inputShape, const PartialShape& filterShape,
                      const PartialShape& offsetShape, const PartialShape& outputShape,
                      const element::Type& iType,
                      const std::vector<IT>& iValues, const std::vector<IT>& filterValues,
                      const std::vector<IT>& offsetValues, const std::vector<IT>& oValues,
                      const Strides& strides, const CoordinateDiff& padBegin, const CoordinateDiff& padEnd, const Strides& dialations,
                      const int64_t group = 1, const int64_t deformableGroup = 1,
                      const std::string& test_name = "",
                      const bool use_bilinear_interpolation_padding = false)
        : inputShape(inputShape),
          filterShape(filterShape),
          offsetShape(offsetShape),
          outputShape(outputShape),
          inType(iType),
          filterType(iType),
          offsetType(iType),
          outType(iType),
          inputData(CreateTensor(iType, iValues)),
          filterData(CreateTensor(iType, filterValues)),
          offsetData(CreateTensor(iType, offsetValues)),
          refData(CreateTensor(iType, oValues)),
          strides(strides),
          padBegin(padBegin),
          padEnd(padEnd),
          dialations(dialations),
          group(group),
          deformableGroup(deformableGroup),
          testcaseName(test_name),
          use_bilinear_interpolation_padding(use_bilinear_interpolation_padding) {}

    template <class IT>
    DeformableConvolutionParams(const PartialShape& inputShape, const PartialShape& filterShape,
                      const PartialShape& offsetShape, const PartialShape& outputShape, const PartialShape& maskShape,
                      const element::Type& iType,
                      const std::vector<IT>& iValues, const std::vector<IT>& filterValues,
                      const std::vector<IT>& offsetValues, const std::vector<IT>& oValues, const std::vector<IT>& maskValues,
                      const Strides& strides, const CoordinateDiff& padBegin, const CoordinateDiff& padEnd, const Strides& dialations,
                      const int64_t group = 1, const int64_t deformableGroup = 1,
                      const std::string& test_name = "",
                      const bool use_bilinear_interpolation_padding = false)
        : inputShape(inputShape),
          filterShape(filterShape),
          offsetShape(offsetShape),
          outputShape(outputShape),
          maskShape(maskShape),
          inType(iType),
          filterType(iType),
          offsetType(iType),
          outType(iType),
          maskType(iType),
          inputData(CreateTensor(iType, iValues)),
          filterData(CreateTensor(iType, filterValues)),
          offsetData(CreateTensor(iType, offsetValues)),
          refData(CreateTensor(iType, oValues)),
          maskData(CreateTensor(iType, maskValues)),
          strides(strides),
          padBegin(padBegin),
          padEnd(padEnd),
          dialations(dialations),
          group(group),
          deformableGroup(deformableGroup),
          testcaseName(test_name),
          use_bilinear_interpolation_padding(use_bilinear_interpolation_padding) {}

    PartialShape inputShape;
    PartialShape filterShape;
    PartialShape offsetShape;
    PartialShape outputShape;
    PartialShape maskShape{};
    ov::element::Type inType;
    ov::element::Type filterType;
    ov::element::Type offsetType;
    ov::element::Type outType;
    ov::element::Type maskType;
    ov::Tensor inputData;
    ov::Tensor filterData;
    ov::Tensor offsetData;
    ov::Tensor refData;
    ov::Tensor maskData;
    ov::Strides strides;
    ov::CoordinateDiff padBegin;
    ov::CoordinateDiff padEnd;
    ov::Strides dialations;
    int64_t group;
    int64_t deformableGroup;
    std::string testcaseName;
    bool use_bilinear_interpolation_padding;
};

class ReferenceDeformableConvolutionLayerTest : public testing::TestWithParam<DeformableConvolutionParams>, public CommonReferenceTest {
public:
    void SetUp() override {
        auto params = GetParam();
        function = CreateFunction(params);
        inputData = {params.inputData, params.offsetData, params.filterData};
        refOutData = {params.refData};
    }
    static std::string getTestCaseName(const testing::TestParamInfo<DeformableConvolutionParams>& obj) {
        auto param = obj.param;
        std::ostringstream result;
        result << "inputShape=" << param.inputShape << "_";
        result << "filterShape=" << param.filterShape << "_";
        result << "offsetShape=" << param.offsetShape << "_";
        result << "outputShape=" << param.outputShape << "_";
        result << "iType=" << param.inType << "_";
        result << "strides=" << param.strides << "_";
        result << "padBegin=" << param.padBegin << "_";
        result << "padEnd=" << param.padEnd << "_";
        result << "dialations=" << param.dialations << "_";
        result << "group=" << param.group << "_";
        if (param.testcaseName != "") {
            result << "deformableGroup=" << param.deformableGroup << "_";
            result << param.testcaseName;
        } else {
            result << "deformableGroup=" << param.deformableGroup;
        }
        return result.str();
    }

private:
    static std::shared_ptr<Model> CreateFunction(const DeformableConvolutionParams& params) {
        const op::PadType auto_pad{op::PadType::EXPLICIT};

        const auto in = std::make_shared<op::v0::Parameter>(params.inType, params.inputShape);
        const auto offset = std::make_shared<op::v0::Parameter>(params.offsetType, params.offsetShape);
        const auto filter = std::make_shared<op::v0::Parameter>(params.filterType, params.filterShape);
        const auto DeformableConvolution = std::make_shared<op::v1::DeformableConvolution>(in,
                                                                       offset,
                                                                       filter,
                                                                       params.strides,
                                                                       params.padBegin,
                                                                       params.padEnd,
                                                                       params.dialations,
                                                                       auto_pad,
                                                                       params.group,
                                                                       params.deformableGroup);
        return std::make_shared<ov::Model>(NodeVector {DeformableConvolution}, ParameterVector {in, offset, filter});
    }
};

class ReferenceDeformableConvolutionV8LayerTest : public testing::TestWithParam<DeformableConvolutionParams>, public CommonReferenceTest {
public:
    void SetUp() override {
        auto params = GetParam();
        function = CreateFunction(params);
        if (params.maskShape.size() != 0) {
            inputData = {params.inputData, params.offsetData, params.filterData, params.maskData};
        } else {
            inputData = {params.inputData, params.offsetData, params.filterData};
        }
        refOutData = {params.refData};
    }
    static std::string getTestCaseName(const testing::TestParamInfo<DeformableConvolutionParams>& obj) {
        auto param = obj.param;
        std::ostringstream result;
        result << "inputShape=" << param.inputShape << "_";
        result << "filterShape=" << param.filterShape << "_";
        result << "offsetShape=" << param.offsetShape << "_";
        result << "outputShape=" << param.outputShape << "_";
        if (param.maskShape.size() != 0)
            result << "maskShape=" << param.maskShape << "_";
        result << "iType=" << param.inType << "_";
        result << "strides=" << param.strides << "_";
        result << "padBegin=" << param.padBegin << "_";
        result << "padEnd=" << param.padEnd << "_";
        result << "dialations=" << param.dialations << "_";
        result << "group=" << param.group << "_";
        result << "deformableGroup=" << param.deformableGroup << "_";
        if (param.testcaseName != "") {
            result << "use_bilinear_interpolation_padding=" << param.use_bilinear_interpolation_padding << "_";
            result << param.testcaseName;
        } else {
            result << "use_bilinear_interpolation_padding=" << param.use_bilinear_interpolation_padding;
        }
        return result.str();
    }

private:
    static std::shared_ptr<Model> CreateFunction(const DeformableConvolutionParams& params) {
        const op::PadType auto_pad{op::PadType::EXPLICIT};

        const auto in = std::make_shared<op::v0::Parameter>(params.inType, params.inputShape);
        const auto offset = std::make_shared<op::v0::Parameter>(params.offsetType, params.offsetShape);
        const auto filter = std::make_shared<op::v0::Parameter>(params.filterType, params.filterShape);
        if (params.maskShape.size() != 0) {
            const auto mask = std::make_shared<op::v0::Parameter>(params.maskType, params.maskShape);
            const auto DeformableConvolutionV8 = std::make_shared<op::v8::DeformableConvolution>(in,
                                                                        offset,
                                                                        filter,
                                                                        mask,
                                                                        params.strides,
                                                                        params.padBegin,
                                                                        params.padEnd,
                                                                        params.dialations,
                                                                        auto_pad,
                                                                        params.group,
                                                                        params.deformableGroup,
                                                                        params.use_bilinear_interpolation_padding);
            return std::make_shared<ov::Model>(NodeVector {DeformableConvolutionV8}, ParameterVector {in, offset, filter, mask});
        } else {
            const auto DeformableConvolutionV8 = std::make_shared<op::v8::DeformableConvolution>(in,
                                                                        offset,
                                                                        filter,
                                                                        params.strides,
                                                                        params.padBegin,
                                                                        params.padEnd,
                                                                        params.dialations,
                                                                        auto_pad,
                                                                        params.group,
                                                                        params.deformableGroup,
                                                                        params.use_bilinear_interpolation_padding);
            return std::make_shared<ov::Model>(NodeVector {DeformableConvolutionV8}, ParameterVector {in, offset, filter});
        }
    }
};

TEST_P(ReferenceDeformableConvolutionLayerTest, CompareWithRefs) {
    Exec();
}

TEST_P(ReferenceDeformableConvolutionV8LayerTest, CompareWithRefs) {
    Exec();
}

template <element::Type_t IN_ET>
std::vector<DeformableConvolutionParams> generateDeformableConvolutionFloatParams() {
    using T = typename element_type_traits<IN_ET>::value_type;

    std::vector<DeformableConvolutionParams> deformableConvolutionParams {
// --------------------- 2D DeformableConvolution ------------------------------------------
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f},
                          std::vector<T>{
                                    1.0f, 2.0f,
                                    -1.0f, -2.0f},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    -12.0f, -12.0f, -12.0f,
                                    -12.0f, -12.0f, -12.0f,
                                    -12.0f, -12.0f, -12.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 3, 3},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {1, 8, 4, 4},
                          PartialShape {1, 1, 4, 4},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 3.0f, 5.0f,
                                    7.0f, 5.0f, 3.0f,
                                    1.0f, 3.0f, 5.0f},
                          std::vector<T>{
                                    1.0f, 2.0f,
                                    0.0f, 1.0f},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    1.0f, 3.0f, 5.0f, 0.0f,
                                    9.0f, 12.0f, 16.0f, 5.0f,
                                    15.0f, 20.0f, 16.0f, 3.0f,
                                    2.0f, 7.0f, 13.0f, 5.0f},
                          {1, 1},
                          {1, 1},
                          {1, 1},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 5, 5},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 3.0f, 5.0f, 7.0f, 9.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f, 0.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f, 0.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f},
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    1.0f, 1.0f, 1.0f,
                                    3.0f, 2.0f, 1.0f},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    57.0f, 94.0f,
                                    66.0f, 102.0f},
                          {2, 2},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 7, 7},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 3.0f, 5.0f, 7.0f, 9.0f, 11.0f, 13.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f, -1.0f, -3.0f, -5.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f, 0.0f, -2.0f, -4.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f, -1.0f, -3.0f, -5.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f, 0.0f, -2.0f, -4.0f},
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    1.0f, 1.0f, 0.0f,
                                    3.0f, 1.0f, 2.0f},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    78.0f, 106.0f, 134.0f,
                                    44.0f, 16.0f, -12.0f,
                                    80.0f, 84.0f, 88.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {2, 2}),
        DeformableConvolutionParams(PartialShape {1, 1, 7, 7},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 4, 4},
                          PartialShape {1, 1, 4, 4},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 3.0f, 5.0f, 7.0f, 9.0f, 11.0f, 13.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f, -1.0f, -3.0f, -5.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f, 0.0f, -2.0f, -4.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f, -1.0f, -3.0f, -5.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f, 0.0f, -2.0f, -4.0f},
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    1.0f, 1.0f, 0.0f,
                                    3.0f, 1.0f, 2.0f},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    15.0f, 38.0f, 70.0f, 66.0f,
                                    33.0f, 78.0f, 134.0f, 103.0f,
                                    40.0f, 80.0f, 88.0f, 58.0f,
                                    30.0f, 56.0f, 72.0f, 34.0f},
                          {2, 2},
                          {2, 2},
                          {2, 2},
                          {2, 2}),
        DeformableConvolutionParams(PartialShape {1, 2, 4, 4},
                          PartialShape {1, 2, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 3.0f, 5.0f, 7.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f,
                                    // channel 2
                                    -1.0f, 3.0f, -5.0f, 7.0f,
                                    7.0f, -5.0f, 3.0f, -1.0f,
                                    -2.0f, 4.0f, -6.0f, 8.0f,
                                    8.0f, -6.0f, 4.0f, -2.0f},
                          std::vector<T>{
                                    // channel 1
                                    5.0f, 3.0f, 5.0f,
                                    1.0f, 3.0f, 1.0f,
                                    4.0f, 2.0f, 4.0f,
                                    // channel 2
                                    -5.0f, 3.0f, 5.0f,
                                    1.0f, -3.0f, 1.0f,
                                    4.0f, 2.0f, -4.0f},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    142.0f, 102.0f,
                                    94.0f, 160.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {2, 1, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 3.0f, 5.0f, 7.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f},
                          std::vector<T>{
                                    // channel 1
                                    5.0f, 3.0f, 5.0f,
                                    1.0f, 3.0f, 1.0f,
                                    4.0f, 2.0f, 4.0f,
                                    // channel 2
                                   -5.0f, 3.0f, 5.0f,
                                    1.0f, -3.0f, 1.0f,
                                    4.0f, 2.0f, -4.0f},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    // channel 1
                                    104.0f, 140.0f,
                                    145.0f, 109.0f,
                                    // channel 2
                                    16.0f, 28.0f,
                                    19.0f, 7.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {2, 1, 4, 4},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {2, 18, 2, 2},
                          PartialShape {2, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // batch 1
                                    1.0f, 3.0f, 2.0f, 1.0f,
                                    1.0f, 3.0f, 3.0f, 1.0f,
                                    2.0f, 1.0f, 1.0f, 3.0f,
                                    3.0f, 2.0f, 3.0f, 3.0f,
                                    // batch 2
                                    -1.0f, 3.0f, 2.0f, -1.0f,
                                    1.0f, 3.0f, -3.0f, 1.0f,
                                    -2.0f, -1.0f, 1.0f, 3.0f,
                                    3.0f, 2.0f, 3.0f, -3.0f},
                          std::vector<T>{
                                   -5.0f, 3.0f, 5.0f,
                                    1.0f, -3.0f, 1.0f,
                                    4.0f, 2.0f, -4.0f},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    // batch 1
                                    15.0f, -15.0f,
                                    23.0f, 2.0f,
                                    // batch 2
                                    -1.0f, -15.0f,
                                    -5.0f, 6.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 4, 3, 3},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 8, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 2.0f, 3.0f,
                                    4.0f, 5.0f, 6.0f,
                                    7.0f, 8.0f, 9.0f,
                                     // channel 2
                                    10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f,
                                    16.0f, 17.0f, 18.0f,
                                    // channel 3
                                    19.0f, 20.0f, 21.0f,
                                    22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f,
                                    // channel 4
                                    28.0f, 29.0f, 30.0f,
                                    31.0f, 32.0f, 33.0f,
                                    34.0f, 35.0f, 36.0f},
                          std::vector<T>{
                                    // filter 1 channel 1
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // filter 1 channel 2
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // filter 2 channel 1
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // filter 2 channel 2
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    // channel 1
                                    356.0f, 392.0f,
                                    464.0f, 500.0f,
                                    // channel 2
                                    -1004.0f, -1040.0f,
                                    -1112.0f, -1148.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          2),
        DeformableConvolutionParams(PartialShape {1, 8, 3, 3},
                          PartialShape {4, 2, 2, 2},
                          PartialShape {1, 8, 2, 2},
                          PartialShape {1, 4, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 2.0f, 3.0f,
                                    4.0f, 5.0f, 6.0f,
                                    7.0f, 8.0f, 9.0f,
                                    // channel 2
                                    10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f,
                                    16.0f, 17.0f, 18.0f,
                                    // channel 3
                                    19.0f, 20.0f, 21.0f,
                                    22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f,
                                    // channel 4
                                    28.0f, 29.0f, 30.0f,
                                    31.0f, 32.0f, 33.0f,
                                    34.0f, 35.0f, 36.0f,
                                    // channel 5
                                    37.0f, 38.0f, 39.0f,
                                    40.0f, 41.0f, 42.0f,
                                    43.0f, 44.0f, 45.0f,
                                    // channel 6
                                    46.0f, 47.0f, 48.0f,
                                    49.0f, 50.0f, 51.0f,
                                    52.0f, 53.0f, 54.0f,
                                    // channel 7
                                    55.0f, 56.0f, 57.0f,
                                    58.0f, 59.0f, 60.0f,
                                    61.0f, 62.0f, 63.0f,
                                    // channel 8
                                    64.0f, 65.0f, 66.0f,
                                    67.0f, 68.0f, 69.0f,
                                    70.0f, 71.0f, 72.0f},
                          std::vector<T>{
                                    // filter 1 channel 1
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // filter 1 channel 2
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // filter 2 channel 1
                                    9.0f, 10.0f,
                                    11.0f, 12.0f,
                                    // filter 2 channel 2
                                    13.0f, 14.0f,
                                    15.0f, 16.0f,
                                    // filter 3 channel 1
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // filter 3 channel 2
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f,
                                    // filter 4 channel 1
                                    -9.0f, -10.0f,
                                    -11.0f, -12.0f,
                                    // filter 4 channel 2
                                    -13.0f, -14.0f,
                                    -15.0f, -16.0f},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    // channel 1
                                    356.0f, 392.0f,
                                    464.0f, 500.0f,
                                    // channel 2
                                    2636.0f, 2736.0f,
                                    2936.0f, 3036.0f,
                                    // channel 3
                                    -1652.0f, -1688.0f,
                                    -1760.0f, -1796.0f,
                                    // channel 4
                                    -6236.0f, -6336.0f,
                                    -6536.0f, -6636.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          4),
        DeformableConvolutionParams(PartialShape {1, 2, 4, 4},
                          PartialShape {1, 2, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f,
                                    // channel 2
                                    17.0f, 18.0f, 19.0f, 20.0f,
                                    21.0f, 22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f, 28.0f,
                                    29.0f, 30.0f, 31.0f, 32.0f},
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 2.0f,
                                    -1.0f, -2.0f,
                                    // channel 2
                                    3.0f, 4.0f,
                                    -3.0f, -4.0f},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    -40.0f, -40.0f, -40.0f,
                                    -40.0f, -40.0f, -40.0f,
                                    -40.0f, -40.0f, -40.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 4, 3, 3},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 16, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    4.0f, 5.0f, 6.0f,
                                    7.0f, 8.0f, 9.0f,
                                    10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f,
                                    16.0f, 17.0f, 18.0f,
                                    19.0f, 20.0f, 21.0f,
                                    22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f,
                                    28.0f, 29.0f, 30.0f,
                                    31.0f, 32.0f, 33.0f,
                                    34.0f, 35.0f, 36.0f},
                          std::vector<T>{
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    356.0f, 392.0f,
                                    464.0f, 500.0f,
                                    -1004.0f, -1040.0f,
                                    -1112.0f, -1148.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          2,
                          2),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f},
                          std::vector<T>{
                                    1.0f, 2.0f,
                                    -1.0f, -2.0f},
                          std::vector<T>{
                                    // window 1 (Y=0, X=0) -> Y coordinate
                                    1.0f, 1.0f, 1.0f, // out1 .. out 3
                                    1.0f, 1.0f, 1.0f, // out4 .. out 6
                                    1.0f, 1.0f, 1.0f, // out7 .. out 9
                                    // window 1 (Y=0, X=0) -> X coordinate
                                    1.0f, 1.0f, 1.0f, // out1 .. out 3
                                    1.0f, 1.0f, 1.0f, // out4 .. out 6
                                    1.0f, 1.0f, 1.0f, // out7 .. out 9
                                    // window 2
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    // window 2
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    // window 3
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    // window 3
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    // window 4
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    // window 4
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f},
                          std::vector<T>{
                                    -12.0f, -12.0f, -4.0f,
                                    -12.0f, -12.0f, -4.0f,
                                    44.0f, 47.0f, 16.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offsets_1"),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 4, 4},
                          PartialShape {1, 1, 4, 4},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 3.0f, 7.0f, 7.0f,
                                    7.0f, 6.0f, 3.0f, 1.0f,
                                    4.0f, 4.0f, 2.0f, 8.0f,
                                    1.0f, 1.0f, 1.0f, 2.0f},
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    0.0f, 1.0f, 0.0f,
                                    3.0f, 2.0f, 1.0f},
                          std::vector<T>{
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f},
                          std::vector<T>{
                                    56.0f, 39.0f, 44.0f, 18.0f,
                                    38.0f, 56.0f, 65.0f, 0.0f,
                                    19.0f, 38.0f, 20.0f, 20.0f,
                                    6.0f, 19.0f, 33.0f, 0.0f},
                          {1, 1},
                          {1, 1},
                          {1, 1},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 5, 5},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 3.0f, 5.0f, 7.0f, 9.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f, 0.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f, 0.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f},
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    1.0f, 1.0f, 1.0f,
                                    3.0f, 2.0f, 1.0f},
                          std::vector<T>{
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f,
                                    0.0f, 2.0f,
                                    1.0f, 0.0f},
                          std::vector<T>{
                                    57.0f, 40.0f,
                                    38.0f, 102.0f},
                          {2, 2},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offsets_stride"),
        DeformableConvolutionParams(PartialShape {1, 1, 7, 7},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 3.0f, 5.0f, 7.0f, 9.0f, 11.0f, 13.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f, -1.0f, -3.0f, -5.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f, 0.0f, -2.0f, -4.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f, -1.0f, -3.0f, -5.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f, 0.0f, -2.0f, -4.0f},
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    1.0f, 1.0f, 0.0f,
                                    3.0f, 1.0f, 2.0f},
                          std::vector<T>{
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 1.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f,
                                    1.0f, 2.0f, 0.0f,
                                    0.0f, 2.0f, 0.0f,
                                    1.0f, 0.0f, 1.0f},
                          std::vector<T>{
                                    16.0f, -2.0f, 134.0f,
                                    44.0f, -4.0f, -12.0f,
                                    10.0f, 84.0f, -4.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {2, 2},
                          1,
                          1,
                          "integral_offset_dialation"),
        DeformableConvolutionParams(PartialShape {1, 1, 7, 7},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 4, 4},
                          PartialShape {1, 1, 4, 4},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 3.0f, 5.0f, 7.0f, 9.0f, 11.0f, 13.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f, -1.0f, -3.0f, -5.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f, 0.0f, -2.0f, -4.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f, -1.0f, -3.0f, -5.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f, 0.0f, -2.0f, -4.0f},
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    1.0f, 1.0f, 0.0f,
                                    3.0f, 1.0f, 2.0f},
                          std::vector<T>{
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,

                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f,
                                    1.0f, 0.0f, 1.0f, 0.0f,
                                    1.0f, 0.0f, 0.0f, 2.0f},
                          std::vector<T>{
                                    15.0f, 38.0f, 2.0f, 66.0f,
                                    26.0f, 78.0f, 134.0f, 16.0f,
                                    23.0f, 80.0f, -4.0f, 58.0f,
                                    13.0f, 56.0f, 72.0f, -4.0f},
                          {2, 2},
                          {2, 2},
                          {2, 2},
                          {2, 2},
                          1,
                          1,
                          "integral_offset_padding_stride_dialation"),
        DeformableConvolutionParams(PartialShape {1, 2, 4, 4},
                          PartialShape {1, 2, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 3.0f, 5.0f, 7.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f,
                                    // channel 2
                                    -1.0f, 3.0f, -5.0f, 7.0f,
                                    7.0f, -5.0f, 3.0f, -1.0f,
                                    -2.0f, 4.0f, -6.0f, 8.0f,
                                    8.0f, -6.0f, 4.0f, -2.0f},
                          std::vector<T>{
                                    // channel 1
                                    5.0f, 3.0f, 5.0f,
                                    1.0f, 3.0f, 1.0f,
                                    4.0f, 2.0f, 4.0f,
                                    // channel 2
                                    -5.0f, 3.0f, 5.0f,
                                    1.0f, -3.0f, 1.0f,
                                    4.0f, 2.0f, -4.0f},
                          std::vector<T>{
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f,
                                    1.0f, 1.0f,
                                    0.0f, 2.0f},
                          std::vector<T>{
                                    160.0f, 32.0f,
                                    94.0f, 20.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offset_input_channels"),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {2, 1, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 2, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f},
                          std::vector<T>{
                                    // filter 1
                                    1.0f, 2.0f,
                                    -1.0f, -2.0f,
                                    // filter 2
                                    3.0f, 4.0f,
                                    -3.0f, -4.0f},
                          std::vector<T>{
                                    //channel 1: Y offsets
                                    1.0f, 1.0f, 1.0f, // out1 .. out 3
                                    1.0f, 1.0f, 1.0f, // out4 .. out 6
                                    1.0f, 1.0f, 1.0f, // out7 .. out 9
                                    //channel 1: X offsets
                                    1.0f, 1.0f, 1.0f, // out1 .. out 3
                                    1.0f, 1.0f, 1.0f, // out4 .. out 6
                                    1.0f, 1.0f, 1.0f, // out7 .. out 9
                                    //channel 2: Y offsets
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    //channel 2: X offsets
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    //channel 3: Y offsets
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    //channel 3: X offsets
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    //channel 4: Y offsets
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    //channel 4: X offsets
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f},
                          std::vector<T>{
                                    // output 1
                                    -12.0f, -12.0f, -4.0f,
                                    -12.0f, -12.0f, -4.0f,
                                    44.0f, 47.0f, 16.0f,
                                    // output 2
                                    -28.0f, -28.0f, -12.0f,
                                    -28.0f, -28.0f, -12.0f,
                                    102.0f, 109.0f, 48.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offset_output_channels"),
        DeformableConvolutionParams(PartialShape {2, 1, 4, 4},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {2, 8, 3, 3},
                          PartialShape {2, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    //batch 1
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f,
                                    //batch 2
                                    17.0f, 18.0f, 19.0f, 20.0f,
                                    21.0f, 22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f, 28.0f,
                                    29.0f, 30.0f, 31.0f, 32.0f},
                          std::vector<T>{
                                    1.0f, 2.0f,
                                    -1.0f, -2.0f},
                          std::vector<T>{
                                    // batch1
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    // batch2
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f},
                          std::vector<T>{
                                    // batch 1
                                    -12.0f, -12.0f, -4.0f,
                                    -12.0f, -12.0f, -4.0f,
                                    44.0f, 47.0f, 16.0f,
                                    // batch 2
                                    -12.0f, -12.0f, -12.0f,
                                    -12.0f, -12.0f, -12.0f,
                                    -12.0f, -12.0f, -12.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offset_batch"),
        DeformableConvolutionParams(PartialShape {1, 4, 3, 3},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 8, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 2.0f, 3.0f,
                                    4.0f, 5.0f, 6.0f,
                                    7.0f, 8.0f, 9.0f,
                                     // channel 2
                                    10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f,
                                    16.0f, 17.0f, 18.0f,
                                    // channel 3
                                    19.0f, 20.0f, 21.0f,
                                    22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f,
                                    // channel 4
                                    28.0f, 29.0f, 30.0f,
                                    31.0f, 32.0f, 33.0f,
                                    34.0f, 35.0f, 36.0f},
                          std::vector<T>{
                                    // filter 1 channel 1
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                     // filter 1 channel 2
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                     // filter 2 channel 1
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // filter 2 channel 2
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    // window 1 (F_Y=0, F_X=0) -> I_Y coordinate
                                    1.0f, 0.0f, 2.0f, 1.0f, // out1 .. out 4
                                    // window 1 (F_Y=0, F_X=0) -> I_X coordinate
                                    0.0f, 1.0f, 1.0f, 2.0f, // out1 .. out 4
                                    // window 2 (F_Y=0, F_X=1) -> I_Y coordinate
                                    1.0f, 1.0f, 1.0f, 1.0f, // out1 .. out 4
                                    // window 2 (F_Y=0, F_X=1) -> I_X coordinate
                                    1.0f, 1.0f, 1.0f, 1.0f, // out1 .. out 4
                                    // window 3 (F_Y=1, F_X=0) -> I_Y coordinate
                                    2.0f, 2.0f, 2.0f, 2.0f, // out1 .. out 4
                                    // window 3 (F_Y=1, F_X=0) -> I_X coordinate
                                    2.0f, 2.0f, 2.0f, 2.0f, // out1 .. out 4
                                    // window 4 (F_Y=1, F_X=1) -> I_Y coordinate
                                    2.0f, 2.0f, 2.0f, 2.0f, // out1 .. out 4
                                    // window 4 (F_Y=1, F_X=1) -> I_X coordinate
                                    2.0f, 2.0f, 2.0f, 2.0f}, // out1 .. out 4
                          std::vector<T>{
                                    // channel 1
                                    171.0f, 63.0f,
                                    126.0f, 0.0f,
                                    // channel 2
                                    -423.0f, -171.0f,
                                    -270.0f, 0.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          2,
                          1,
                          "integral_offset_groups_basic"),
        DeformableConvolutionParams(PartialShape {1, 8, 3, 3},
                          PartialShape {4, 2, 2, 2},
                          PartialShape {1, 8, 2, 2},
                          PartialShape {1, 4, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 2.0f, 3.0f,
                                    4.0f, 5.0f, 6.0f,
                                    7.0f, 8.0f, 9.0f,
                                     // channel 2
                                    10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f,
                                    16.0f, 17.0f, 18.0f,
                                    // channel 3
                                    19.0f, 20.0f, 21.0f,
                                    22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f,
                                    // channel 4
                                    28.0f, 29.0f, 30.0f,
                                    31.0f, 32.0f, 33.0f,
                                    34.0f, 35.0f, 36.0f,
                                     // channel 5
                                    37.0f, 38.0f, 39.0f,
                                    40.0f, 41.0f, 42.0f,
                                    43.0f, 44.0f, 45.0f,
                                     // channel 6
                                    46.0f, 47.0f, 48.0f,
                                    49.0f, 50.0f, 51.0f,
                                    52.0f, 53.0f, 54.0f,
                                     // channel 7
                                    55.0f, 56.0f, 57.0f,
                                    58.0f, 59.0f, 60.0f,
                                    61.0f, 62.0f, 63.0f,
                                     // channel 8
                                    64.0f, 65.0f, 66.0f,
                                    67.0f, 68.0f, 69.0f,
                                    70.0f, 71.0f, 72.0f},
                          std::vector<T>{
                                    // filter 1 channel 1
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // filter 1 channel 2
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // filter 2 channel 1
                                    9.0f, 10.0f,
                                    11.0f, 12.0f,
                                    // filter 2 channel 2
                                    13.0f, 14.0f,
                                    15.0f, 16.0f,
                                    // filter 3 channel 1
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // filter 3 channel 2
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f,
                                    // filter 4 channel 1
                                    -9.0f, -10.0f,
                                    -11.0f, -12.0f,
                                    // filter 4 channel 2
                                    -13.0f, -14.0f,
                                    -15.0f, -16.0f},
                          std::vector<T>{
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f, 1.0f},
                          std::vector<T>{
                                    // channel 1
                                    500.0f, 234.0f,
                                    219.0f, 99.0f,
                                    // channel 2
                                    3036.0f, 1482.0f,
                                    1463.0f, 711.0f,
                                    // channel 3
                                    -1796.0f, -810.0f,
                                    -723.0f, -315.0f,
                                    // channel 4
                                    -6636.0f, -3210.0f,
                                    -3119.0f, -1503.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          4,
                          1,
                          "integral_offset_groups_complex"),
        DeformableConvolutionParams(PartialShape {1, 2, 4, 4},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 16, 3, 3},
                          PartialShape {1, 2, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f,
                                    // channel 2
                                    17.0f, 18.0f, 19.0f, 20.0f,
                                    21.0f, 22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f, 28.0f,
                                    29.0f, 30.0f, 31.0f, 32.0f},
                          std::vector<T>{
                                    // f1: channel 1
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // f1: channel 2
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // f2: channel 1
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // f2: channel 2
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    // defgroup 1
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    //  defgroup 2
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f},
                          std::vector<T>{
                                    // output 1
                                    610.0f, 646.0f, 612.0f,
                                    754.0f, 790.0f, 732.0f,
                                    768.0f, 797.0f, 792.0f,
                                    // output 2
                                    -610.0f, -646.0f, -612.0f,
                                    -754.0f, -790.0f, -732.0f,
                                    -768.0f, -797.0f, -792.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          2,
                          "integral_offset_deforgroup_basic"),
        DeformableConvolutionParams(PartialShape {1, 4, 4, 4},
                          PartialShape {2, 4, 2, 2},
                          PartialShape {1, 32, 3, 3},
                          PartialShape {1, 2, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f,
                                    // channel 2
                                    17.0f, 18.0f, 19.0f, 20.0f,
                                    21.0f, 22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f, 28.0f,
                                    29.0f, 30.0f, 31.0f, 32.0f,
                                    // channel 3
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f,
                                    // channel 4
                                    17.0f, 18.0f, 19.0f, 20.0f,
                                    21.0f, 22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f, 28.0f,
                                    29.0f, 30.0f, 31.0f, 32.0f},
                          std::vector<T>{
                                    // f1: channel 1
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // f1: channel 2
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // f1: channel 3
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // f1: channel 4
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // f2: channel 1
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // f2: channel 2
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f,
                                    // f2: channel 3
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // f2: channel 4
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    // defgroup 1
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    //  defgroup 2
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    // defgroup 3
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    //  defgroup 4
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f},
                          std::vector<T>{
                                    // output 1
                                    1220.0f, 1292.0f, 1224.0f,
                                    1508.0f, 1580.0f, 1464.0f,
                                    1536.0f, 1594.0f, 1584.0f,
                                    // output 2
                                    -1220.0f, -1292.0f, -1224.0f,
                                    -1508.0f, -1580.0f, -1464.0f,
                                    -1536.0f, -1594.0f, -1584.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          4,
                          "integral_offset_deforgroup_complex1"),
        DeformableConvolutionParams(PartialShape {1, 4, 4, 4},
                          PartialShape {2, 4, 2, 2},
                          PartialShape {1, 16, 3, 3},
                          PartialShape {1, 2, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f,
                                    // channel 2
                                    17.0f, 18.0f, 19.0f, 20.0f,
                                    21.0f, 22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f, 28.0f,
                                    29.0f, 30.0f, 31.0f, 32.0f,
                                    // channel 3
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f,
                                    // channel 4
                                    17.0f, 18.0f, 19.0f, 20.0f,
                                    21.0f, 22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f, 28.0f,
                                    29.0f, 30.0f, 31.0f, 32.0f},
                          std::vector<T>{
                                    // f1: channel 1
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // f1: channel 2
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // f1: channel 3
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // f1: channel 4
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // f2: channel 1
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // f2: channel 2
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f,
                                    // f2: channel 3
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // f2: channel 4
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    // defgroup 1
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,

                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f,
                                    //  defgroup 2
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,

                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f},
                          std::vector<T>{
                                    // output 1
                                    1300.0f, 1372.0f, 992.0f,
                                    1588.0f, 1660.0f, 1200.0f,
                                    1228.0f, 1278.0f, 1096.0f,
                                    // output 2
                                    -1300.0f, -1372.0f, -992.0f,
                                    -1588.0f, -1660.0f, -1200.0f,
                                    -1228.0f, -1278.0f, -1096.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          2,
                          "integral_offset_deforgroup_complex2"),
        DeformableConvolutionParams(PartialShape {1, 4, 3, 3},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 16, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    4.0f, 5.0f, 6.0f,
                                    7.0f, 8.0f, 9.0f,
                                    10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f,
                                    16.0f, 17.0f, 18.0f,
                                    19.0f, 20.0f, 21.0f,
                                    22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f,
                                    28.0f, 29.0f, 30.0f,
                                    31.0f, 32.0f, 33.0f,
                                    34.0f, 35.0f, 36.0f},
                          std::vector<T>{
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    // defgroup 1
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    1.0f, 1.0f, 1.0f, 1.0f,
                                    // defgroup 2
                                    0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f},
                          std::vector<T>{
                                    500.0f, 234.0f,
                                    219.0f, 99.0f,
                                    -1004.0f, -1040.0f,
                                    -1112.0f, -1148.0f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          2,
                          2,
                          "integral_offset_groups_and_deforgroups"),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f},
                          std::vector<T>{
                                    1.0f, 2.0f,
                                    -1.0f, -2.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    -11.999998f, -11.999999f, -4.000000f,
                                    -10.799999f, -10.800001f, -3.600004f,
                                    44.300000f, 47.100000f, 16.000000f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "real_offset_default"),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 4, 4},
                          PartialShape {1, 1, 4, 4},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 3.0f, 7.0f, 7.0f,
                                    7.0f, 6.0f, 3.0f, 1.0f,
                                    4.0f, 4.0f, 2.0f, 8.0f,
                                    1.0f, 1.0f, 1.0f, 2.0f},
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    0.0f, 1.0f, 0.0f,
                                    3.0f, 2.0f, 1.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    54.870006f, 61.630001f, 43.230003f, 28.600002f,
                                    35.590000f, 25.819999f, 20.880001f, 7.700000f,
                                    19.089998f, 31.719999f, 19.250000f, 7.399999f,
                                    6.299999f, 9.199999f, 5.099999f, 2.000000f},
                          {1, 1},
                          {1, 1},
                          {1, 1},
                          {1, 1},
                          1,
                          1,
                          "real_offset_padding"),
        DeformableConvolutionParams(PartialShape {1, 1, 5, 5},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 3.0f, 5.0f, 7.0f, 9.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f, 0.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f, 0.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f},
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    1.0f, 1.0f, 1.0f,
                                    3.0f, 2.0f, 1.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    61.229999f, 29.509998f,
                                    39.640003f, 22.640003f},
                          {2, 2},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "real_offset_stride"),
        DeformableConvolutionParams(PartialShape {1, 1, 7, 7},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 4, 4},
                          PartialShape {1, 1, 4, 4},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 3.0f, 5.0f, 7.0f, 9.0f, 11.0f, 13.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f, -1.0f, -3.0f, -5.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f, 0.0f, -2.0f, -4.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f, -1.0f, -3.0f, -5.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f, 0.0f, -2.0f, -4.0f},
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    1.0f, 1.0f, 0.0f,
                                    3.0f, 1.0f, 2.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    15.260000f, 24.119997f, 6.439994f, -3.940005f,
                                    26.440002f, 20.319999f, -0.500001f, -11.720002f,
                                    23.500003f, 14.040000f, -1.279998f, -3.860000f,
                                    12.500000f, -2.599999f, -5.299999f, -3.099999f},
                          {2, 2},
                          {2, 2},
                          {2, 2},
                          {2, 2},
                          1,
                          1,
                          "real_offset_padding_stride_dialation"),
        DeformableConvolutionParams(PartialShape {1, 2, 4, 4},
                          PartialShape {1, 2, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 3.0f, 5.0f, 7.0f,
                                    7.0f, 5.0f, 3.0f, 1.0f,
                                    2.0f, 4.0f, 6.0f, 8.0f,
                                    8.0f, 6.0f, 4.0f, 2.0f,
                                    // channel 2
                                    -1.0f, 3.0f, -5.0f, 7.0f,
                                    7.0f, -5.0f, 3.0f, -1.0f,
                                    -2.0f, 4.0f, -6.0f, 8.0f,
                                    8.0f, -6.0f, 4.0f, -2.0f},
                          std::vector<T>{
                                    // channel 1
                                    5.0f, 3.0f, 5.0f,
                                    1.0f, 3.0f, 1.0f,
                                    4.0f, 2.0f, 4.0f,
                                    // channel 2
                                    -5.0f, 3.0f, 5.0f,
                                    1.0f, -3.0f, 1.0f,
                                    4.0f, 2.0f, -4.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    148.000000f, 43.259998f,
                                    91.279998f, 111.199996f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "real_offset_input_channels"),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {2, 1, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 2, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f},
                          std::vector<T>{
                                    // filter 1
                                     1.0f, 2.0f,
                                    -1.0f, -2.0f,
                                    // filter 2
                                     3.0f, 4.0f,
                                    -3.0f, -4.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    // output 1
                                    -12.000000f, -12.000000f, -4.000000f,
                                    -10.799999f, -10.799995f, -3.600000f,
                                    44.299999f, 47.099998f, 16.000000f,
                                    // output 2
                                    -28.000000f, -28.000000f, -12.000000f,
                                    -25.200000f, -25.199993f, -10.800003f,
                                    102.699996f, 109.300003f, 48.000000f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "real_offset_output_channels"),
        DeformableConvolutionParams(PartialShape {2, 1, 4, 4},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {2, 8, 3, 3},
                          PartialShape {2, 2, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    //batch 1
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f,
                                    //batch 2
                                    17.0f, 18.0f, 19.0f, 20.0f,
                                    21.0f, 22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f, 28.0f,
                                    29.0f, 30.0f, 31.0f, 32.0f},
                          std::vector<T>{
                                    1.0f, 2.0f,
                                    -1.0f, -2.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    // batch 1
                                    -12.000000f, -12.000000f, -4.000000f,
                                    -10.799999f, -10.799995f, -3.600000f,
                                    44.299999f, 47.099998f, 16.000000f,
                                    // batch 2
                                    -12.000000f, -12.000000f, -4.000000f,
                                    -10.799999f, -10.799995f, -3.600000f,
                                    92.300003f, 95.099998f, 32.000000f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "real_offset_batch"),
        DeformableConvolutionParams(PartialShape {1, 4, 3, 3},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 8, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 2.0f, 3.0f,
                                    4.0f, 5.0f, 6.0f,
                                    7.0f, 8.0f, 9.0f,
                                    // channel 2
                                    10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f,
                                    16.0f, 17.0f, 18.0f,
                                    // channel 3
                                    19.0f, 20.0f, 21.0f,
                                    22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f,
                                    // channel 4
                                    28.0f, 29.0f, 30.0f,
                                    31.0f, 32.0f, 33.0f,
                                    34.0f, 35.0f, 36.0f},
                          std::vector<T>{
                                    // filter 1 channel 1
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // filter 1 channel 2
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // filter 2 channel 1
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // filter 2 channel 2
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    // channel 1
                                    505.800020f, 235.800000f,
                                    219.600000f, 99.000000f,
                                    // channel 2
                                    -1153.800000f, -523.800000f,
                                    -471.600000f, -207.0000000f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          2,
                          1,
                          "real_offset_group_basic"),
        DeformableConvolutionParams(PartialShape {1, 8, 3, 3},
                          PartialShape {4, 2, 2, 2},
                          PartialShape {1, 8, 2, 2},
                          PartialShape {1, 4, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 2.0f, 3.0f,
                                    4.0f, 5.0f, 6.0f,
                                    7.0f, 8.0f, 9.0f,
                                    // channel 2
                                    10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f,
                                    16.0f, 17.0f, 18.0f,
                                    // channel 3
                                    19.0f, 20.0f, 21.0f,
                                    22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f,
                                    // channel 4
                                    28.0f, 29.0f, 30.0f,
                                    31.0f, 32.0f, 33.0f,
                                    34.0f, 35.0f, 36.0f,
                                    // channel 5
                                    37.0f, 38.0f, 39.0f,
                                    40.0f, 41.0f, 42.0f,
                                    43.0f, 44.0f, 45.0f,
                                    // channel 6
                                    46.0f, 47.0f, 48.0f,
                                    49.0f, 50.0f, 51.0f,
                                    52.0f, 53.0f, 54.0f,
                                    // channel 7
                                    55.0f, 56.0f, 57.0f,
                                    58.0f, 59.0f, 60.0f,
                                    61.0f, 62.0f, 63.0f,
                                    // channel 8
                                    64.0f, 65.0f, 66.0f,
                                    67.0f, 68.0f, 69.0f,
                                    70.0f, 71.0f, 72.0f},
                          std::vector<T>{
                                    // filter 1 channel 1
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // filter 1 channel 2
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // filter 2 channel 1
                                    9.0f, 10.0f,
                                    11.0f, 12.0f,
                                    // filter 2 channel 2
                                    13.0f, 14.0f,
                                    15.0f, 16.0f,
                                    // filter 3 channel 1
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // filter 3 channel 2
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f,
                                    // filter 4 channel 1
                                    -9.0f, -10.0f,
                                    -11.0f, -12.0f,
                                    // filter 4 channel 2
                                    -13.0f, -14.0f,
                                    -15.0f, -16.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    // channel 1
                                    505.800020f, 235.800000f,
                                    219.600000f, 99.000000f,
                                    // channel 2
                                    3054.600000f, 1488.600000f,
                                    1465.200100f, 711.000000f,
                                    // channel 3
                                    -1801.799900f, -811.80000f,
                                    -723.600000f, -315.000000f,
                                    // channel 4
                                    -6654.600000f, -3216.600000f,
                                    -3121.200000f, -1503.000000f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          4,
                          1,
                          "real_offset_groups_complex"),
        DeformableConvolutionParams(PartialShape {1, 2, 4, 4},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 16, 3, 3},
                          PartialShape {1, 2, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f,
                                    // channel 2
                                    17.0f, 18.0f, 19.0f, 20.0f,
                                    21.0f, 22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f, 28.0f,
                                    29.0f, 30.0f, 31.0f, 32.0f},
                          std::vector<T>{
                                    // f1: channel 1
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // f1: channel 2
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // f2: channel 1
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // f2: channel 2
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    // output 1
                                    758.000000f, 792.000000f, 366.399993f,
                                    893.200012f, 927.200012f, 426.399993f,
                                    381.399993f, 394.600006f, 176.000000f,
                                    // output 2
                                    -758.000000f, -792.000000f, -366.399993f,
                                    -893.200012f, -927.200012f, -426.399993f,
                                    -381.399993f, -394.600006f, -176.000000f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          2,
                          "real_offset_deforgroup_basic"),
        DeformableConvolutionParams(PartialShape {1, 4, 4, 4},
                          PartialShape {2, 4, 2, 2},
                          PartialShape {1, 32, 3, 3},
                          PartialShape {1, 2, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f,
                                    // channel 2
                                    17.0f, 18.0f, 19.0f, 20.0f,
                                    21.0f, 22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f, 28.0f,
                                    29.0f, 30.0f, 31.0f, 32.0f,
                                    // channel 3
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f,
                                    // channel 4
                                    17.0f, 18.0f, 19.0f, 20.0f,
                                    21.0f, 22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f, 28.0f,
                                    29.0f, 30.0f, 31.0f, 32.0f},
                          std::vector<T>{
                                    // f1: channel 1
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // f1: channel 2
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // f1: channel 3
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // f1: channel 4
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // f2: channel 1
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // f2: channel 2
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f,
                                    // f2: channel 3
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // f2: channel 4
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    // output 1
                                    1516.000000f, 1583.999877f, 732.799987f,
                                    1786.400146f, 1854.400024f, 852.799987f,
                                    762.799987f, 789.200012f, 352.000000f,
                                    // output 2
                                    -1516.000000f, -1583.999877f, -732.799987f,
                                    -1786.400146f, -1854.400024f, -852.799987f,
                                    -762.799987f, -789.200012f, -352.000000f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          4,
                          "real_offset_deforgroup_complex1"),
        DeformableConvolutionParams(PartialShape {1, 4, 4, 4},
                          PartialShape {2, 4, 2, 2},
                          PartialShape {1, 16, 3, 3},
                          PartialShape {1, 2, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f,
                                    // channel 2
                                    17.0f, 18.0f, 19.0f, 20.0f,
                                    21.0f, 22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f, 28.0f,
                                    29.0f, 30.0f, 31.0f, 32.0f,
                                    // channel 3
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f,
                                    // channel 4
                                    17.0f, 18.0f, 19.0f, 20.0f,
                                    21.0f, 22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f, 28.0f,
                                    29.0f, 30.0f, 31.0f, 32.0f},
                          std::vector<T>{
                                    // f1: channel 1
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // f1: channel 2
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // f1: channel 3
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    // f1: channel 4
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    // f2: channel 1
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // f2: channel 2
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f,
                                    // f2: channel 3
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    // f2: channel 4
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    // output 1
                                    1516.000000f, 1583.999877f, 732.799987f,
                                    1786.400146f, 1854.400024f, 852.799987f,
                                    762.799987f, 789.200012f, 352.000000f,
                                    // output 2
                                    -1516.000000f, -1583.999877f, -732.799987f,
                                    -1786.400146f, -1854.400024f, -852.799987f,
                                    -762.799987f, -789.200012f, -352.000000f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          2,
                          "real_offset_deforgroup_complex2"),
        DeformableConvolutionParams(PartialShape {1, 4, 3, 3},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 16, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    4.0f, 5.0f, 6.0f,
                                    7.0f, 8.0f, 9.0f,
                                    10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f,
                                    16.0f, 17.0f, 18.0f,
                                    19.0f, 20.0f, 21.0f,
                                    22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f,
                                    28.0f, 29.0f, 30.0f,
                                    31.0f, 32.0f, 33.0f,
                                    34.0f, 35.0f, 36.0f},
                          std::vector<T>{
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    505.800020f, 235.800000f,
                                    219.600000f, 99.000000f,
                                    -1153.800000f, -523.800000f,
                                    -471.600000f, -207.000000f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          2,
                          2,
                          "real_offset_groups_and_deforgroups"),
    };
    return deformableConvolutionParams;
}

template <element::Type_t IN_ET>
std::vector<DeformableConvolutionParams> generateDeformableConvolutionIntParams() {
    using T = typename element_type_traits<IN_ET>::value_type;

    std::vector<DeformableConvolutionParams> deformableConvolutionParams {
// --------------------- 2D DeformableConvolution ------------------------------------------
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16},
                          std::vector<T>{
                                    1, 2,
                                    -1, -2},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    -12, -12, -12,
                                    -12, -12, -12,
                                    -12, -12, -12},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 3, 3},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {1, 8, 4, 4},
                          PartialShape {1, 1, 4, 4},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 5,
                                    7, 5, 3,
                                    1, 3, 5},
                          std::vector<T>{
                                    1, 2,
                                    0, 1},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    1, 3, 5, 0,
                                    9, 12, 16, 5,
                                    15, 20, 16, 3,
                                    2, 7, 13, 5},
                          {1, 1},
                          {1, 1},
                          {1, 1},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 5, 5},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 5, 7, 9,
                                    7, 5, 3, 1, 0,
                                    2, 4, 6, 8, 10,
                                    8, 6, 4, 2, 0,
                                    2, 4, 6, 8, 10},
                          std::vector<T>{
                                    1, 2, 3,
                                    1, 1, 1,
                                    3, 2, 1},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    57, 94,
                                    66, 102},
                          {2, 2},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 7, 7},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 5, 7, 9, 11, 13,
                                    7, 5, 3, 1, -1, -3, -5,
                                    2, 4, 6, 8, 10, 12, 14,
                                    8, 6, 4, 2, 0, -2, -4,
                                    2, 4, 6, 8, 10, 12, 14,
                                    7, 5, 3, 1, -1, -3, -5,
                                    8, 6, 4, 2, 0, -2, -4},
                          std::vector<T>{
                                    1, 2, 3,
                                    1, 1, 0,
                                    3, 1, 2},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    78, 106, 134,
                                    44, 16, -12,
                                    80, 84, 88},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {2, 2}),
        DeformableConvolutionParams(PartialShape {1, 1, 7, 7},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 4, 4},
                          PartialShape {1, 1, 4, 4},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 5, 7, 9, 11, 13,
                                    7, 5, 3, 1, -1, -3, -5,
                                    2, 4, 6, 8, 10, 12, 14,
                                    8, 6, 4, 2, 0, -2, -4,
                                    2, 4, 6, 8, 10, 12, 14,
                                    7, 5, 3, 1, -1, -3, -5,
                                    8, 6, 4, 2, 0, -2, -4},
                          std::vector<T>{
                                    1, 2, 3,
                                    1, 1, 0,
                                    3, 1, 2},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    15, 38, 70, 66,
                                    33, 78, 134, 103,
                                    40, 80, 88, 58,
                                    30, 56, 72, 34},
                          {2, 2},
                          {2, 2},
                          {2, 2},
                          {2, 2}),
        DeformableConvolutionParams(PartialShape {1, 2, 4, 4},
                          PartialShape {1, 2, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1, 3, 5, 7,
                                    7, 5, 3, 1,
                                    2, 4, 6, 8,
                                    8, 6, 4, 2,
                                    // channel 2
                                    -1, 3, -5, 7,
                                    7, -5, 3, -1,
                                    -2, 4, -6, 8,
                                    8, -6, 4, -2},
                          std::vector<T>{
                                    // channel 1
                                    5, 3, 5,
                                    1, 3, 1,
                                    4, 2, 4,
                                    // channel 2
                                    -5, 3, 5,
                                    1, -3, 1,
                                    4, 2, -4},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    142, 102,
                                    94, 160},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {2, 1, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 5, 7,
                                    7, 5, 3, 1,
                                    2, 4, 6, 8,
                                    8, 6, 4, 2},
                          std::vector<T>{
                                    // channel 1
                                    5, 3, 5,
                                    1, 3, 1,
                                    4, 2, 4,
                                    // channel 2
                                   -5, 3, 5,
                                    1, -3, 1,
                                    4, 2, -4},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    // channel 1
                                    104, 140,
                                    145, 109,
                                    // channel 2
                                    16, 28,
                                    19, 7},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {2, 1, 4, 4},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {2, 18, 2, 2},
                          PartialShape {2, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // batch 1
                                    1, 3, 2, 1,
                                    1, 3, 3, 1,
                                    2, 1, 1, 3,
                                    3, 2, 3, 3,
                                    // batch 2
                                    -1, 3, 2, -1,
                                    1, 3, -3, 1,
                                    -2, -1, 1, 3,
                                    3, 2, 3, -3},
                          std::vector<T>{
                                   -5, 3, 5,
                                    1, -3, 1,
                                    4, 2, -4},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    // batch 1
                                    15, -15,
                                    23, 2,
                                    // batch 2
                                    -1, -15,
                                    -5, 6},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 4, 3, 3},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 8, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1, 2, 3,
                                    4, 5, 6,
                                    7, 8, 9,
                                     // channel 2
                                    10, 11, 12,
                                    13, 14, 15,
                                    16, 17, 18,
                                    // channel 3
                                    19, 20, 21,
                                    22, 23, 24,
                                    25, 26, 27,
                                    // channel 4
                                    28, 29, 30,
                                    31, 32, 33,
                                    34, 35, 36},
                          std::vector<T>{
                                    // filter 1 channel 1
                                    1, 2,
                                    3, 4,
                                    // filter 1 channel 2
                                    5, 6,
                                    7, 8,
                                    // filter 2 channel 1
                                    -1, -2,
                                    -3, -4,
                                    // filter 2 channel 2
                                    -5, -6,
                                    -7, -8},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    // channel 1
                                    356, 392,
                                    464, 500,
                                    // channel 2
                                    -1004, -1040,
                                    -1112, -1148},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          2),
        DeformableConvolutionParams(PartialShape {1, 8, 3, 3},
                          PartialShape {4, 2, 2, 2},
                          PartialShape {1, 8, 2, 2},
                          PartialShape {1, 4, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1, 2, 3,
                                    4, 5, 6,
                                    7, 8, 9,
                                    // channel 2
                                    10, 11, 12,
                                    13, 14, 15,
                                    16, 17, 18,
                                    // channel 3
                                    19, 20, 21,
                                    22, 23, 24,
                                    25, 26, 27,
                                    // channel 4
                                    28, 29, 30,
                                    31, 32, 33,
                                    34, 35, 36,
                                    // channel 5
                                    37, 38, 39,
                                    40, 41, 42,
                                    43, 44, 45,
                                    // channel 6
                                    46, 47, 48,
                                    49, 50, 51,
                                    52, 53, 54,
                                    // channel 7
                                    55, 56, 57,
                                    58, 59, 60,
                                    61, 62, 63,
                                    // channel 8
                                    64, 65, 66,
                                    67, 68, 69,
                                    70, 71, 72},
                          std::vector<T>{
                                    // filter 1 channel 1
                                    1, 2,
                                    3, 4,
                                    // filter 1 channel 2
                                    5, 6,
                                    7, 8,
                                    // filter 2 channel 1
                                    9, 10,
                                    11, 12,
                                    // filter 2 channel 2
                                    13, 14,
                                    15, 16,
                                    // filter 3 channel 1
                                    -1, -2,
                                    -3, -4,
                                    // filter 3 channel 2
                                    -5, -6,
                                    -7, -8,
                                    // filter 4 channel 1
                                    -9, -10,
                                    -11, -12,
                                    // filter 4 channel 2
                                    -13, -14,
                                    -15, -16},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    // channel 1
                                    356, 392,
                                    464, 500,
                                    // channel 2
                                    2636, 2736,
                                    2936, 3036,
                                    // channel 3
                                    -1652, -1688,
                                    -1760, -1796,
                                    // channel 4
                                    -6236, -6336,
                                    -6536, -6636},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          4),
        DeformableConvolutionParams(PartialShape {1, 2, 4, 4},
                          PartialShape {1, 2, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16,
                                    // channel 2
                                    17, 18, 19, 20,
                                    21, 22, 23, 24,
                                    25, 26, 27, 28,
                                    29, 30, 31, 32},
                          std::vector<T>{
                                    // channel 1
                                    1, 2,
                                    -1, -2,
                                    // channel 2
                                    3, 4,
                                    -3, -4},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    -40, -40, -40,
                                    -40, -40, -40,
                                    -40, -40, -40},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 4, 3, 3},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 16, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1, 2, 3,
                                    4, 5, 6,
                                    7, 8, 9,
                                    10, 11, 12,
                                    13, 14, 15,
                                    16, 17, 18,
                                    19, 20, 21,
                                    22, 23, 24,
                                    25, 26, 27,
                                    28, 29, 30,
                                    31, 32, 33,
                                    34, 35, 36},
                          std::vector<T>{
                                    1, 2,
                                    3, 4,
                                    5, 6,
                                    7, 8,
                                    -1, -2,
                                    -3, -4,
                                    -5, -6,
                                    -7, -8},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    356, 392,
                                    464, 500,
                                    -1004, -1040,
                                    -1112, -1148},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          2,
                          2),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16},
                          std::vector<T>{
                                    1, 2,
                                    -1, -2},
                          std::vector<T>{
                                    // window 1 (Y=0, X=0) -> Y coordinate
                                    1, 1, 1, // out1 .. out 3
                                    1, 1, 1, // out4 .. out 6
                                    1, 1, 1, // out7 .. out 9
                                    // window 1 (Y=0, X=0) -> X coordinate
                                    1, 1, 1, // out1 .. out 3
                                    1, 1, 1, // out4 .. out 6
                                    1, 1, 1, // out7 .. out 9
                                    // window 2
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    // window 2
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    // window 3
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    // window 3
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    // window 4
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    // window 4
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1},
                          std::vector<T>{
                                    -12, -12, -4,
                                    -12, -12, -4,
                                    44, 47, 16},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offsets_1"),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 4, 4},
                          PartialShape {1, 1, 4, 4},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 7, 7,
                                    7, 6, 3, 1,
                                    4, 4, 2, 8,
                                    1, 1, 1, 2},
                          std::vector<T>{
                                    1, 2, 3,
                                    0, 1, 0,
                                    3, 2, 1},
                          std::vector<T>{
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2},
                          std::vector<T>{
                                    56, 39, 44, 18,
                                    38, 56, 65, 0,
                                    19, 38, 20, 20,
                                    6, 19, 33, 0},
                          {1, 1},
                          {1, 1},
                          {1, 1},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 5, 5},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 5, 7, 9,
                                    7, 5, 3, 1, 0,
                                    2, 4, 6, 8, 10,
                                    8, 6, 4, 2, 0,
                                    2, 4, 6, 8, 10},
                          std::vector<T>{
                                    1, 2, 3,
                                    1, 1, 1,
                                    3, 2, 1},
                          std::vector<T>{
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0},
                          std::vector<T>{
                                    57, 40,
                                    38, 102},
                          {2, 2},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offsets_stride"),
        DeformableConvolutionParams(PartialShape {1, 1, 7, 7},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 5, 7, 9, 11, 13,
                                    7, 5, 3, 1, -1, -3, -5,
                                    2, 4, 6, 8, 10, 12, 14,
                                    8, 6, 4, 2, 0, -2, -4,
                                    2, 4, 6, 8, 10, 12, 14,
                                    7, 5, 3, 1, -1, -3, -5,
                                    8, 6, 4, 2, 0, -2, -4},
                          std::vector<T>{
                                    1, 2, 3,
                                    1, 1, 0,
                                    3, 1, 2},
                          std::vector<T>{
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 1, 0,
                                    0, 2, 0,
                                    1, 0, 1,
                                    1, 2, 0,
                                    0, 2, 0,
                                    1, 0, 1},
                          std::vector<T>{
                                    16, -2, 134,
                                    44, -4, -12,
                                    10, 84, -4},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {2, 2},
                          1,
                          1,
                          "integral_offset_dialation"),
        DeformableConvolutionParams(PartialShape {1, 1, 7, 7},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 4, 4},
                          PartialShape {1, 1, 4, 4},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 5, 7, 9, 11, 13,
                                    7, 5, 3, 1, -1, -3, -5,
                                    2, 4, 6, 8, 10, 12, 14,
                                    8, 6, 4, 2, 0, -2, -4,
                                    2, 4, 6, 8, 10, 12, 14,
                                    7, 5, 3, 1, -1, -3, -5,
                                    8, 6, 4, 2, 0, -2, -4},
                          std::vector<T>{
                                    1, 2, 3,
                                    1, 1, 0,
                                    3, 1, 2},
                          std::vector<T>{
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2},
                          std::vector<T>{
                                    15, 38, 2, 66,
                                    26, 78, 134, 16,
                                    23, 80, -4, 58,
                                    13, 56, 72, -4},
                          {2, 2},
                          {2, 2},
                          {2, 2},
                          {2, 2},
                          1,
                          1,
                          "integral_offset_padding_stride_dialation"),
        DeformableConvolutionParams(PartialShape {1, 2, 4, 4},
                          PartialShape {1, 2, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1, 3, 5, 7,
                                    7, 5, 3, 1,
                                    2, 4, 6, 8,
                                    8, 6, 4, 2,
                                    // channel 2
                                    -1, 3, -5, 7,
                                    7, -5, 3, -1,
                                    -2, 4, -6, 8,
                                    8, -6, 4, -2},
                          std::vector<T>{
                                    // channel 1
                                    5, 3, 5,
                                    1, 3, 1,
                                    4, 2, 4,
                                    // channel 2
                                    -5, 3, 5,
                                    1, -3, 1,
                                    4, 2, -4},
                          std::vector<T>{
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2,
                                    1, 1,
                                    0, 2},
                          std::vector<T>{
                                    160, 32,
                                    94, 20},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offset_input_channels"),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {2, 1, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 2, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16},
                          std::vector<T>{
                                    // filter 1
                                    1, 2,
                                    -1, -2,
                                    // filter 2
                                    3, 4,
                                    -3, -4},
                          std::vector<T>{
                                    //channel 1: Y offsets
                                    1, 1, 1, // out1 .. out 3
                                    1, 1, 1, // out4 .. out 6
                                    1, 1, 1, // out7 .. out 9
                                    //channel 1: X offsets
                                    1, 1, 1, // out1 .. out 3
                                    1, 1, 1, // out4 .. out 6
                                    1, 1, 1, // out7 .. out 9
                                    //channel 2: Y offsets
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    //channel 2: X offsets
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    //channel 3: Y offsets
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    //channel 3: X offsets
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    //channel 4: Y offsets
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    //channel 4: X offsets
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1},
                          std::vector<T>{
                                    // output 1
                                    -12, -12, -4,
                                    -12, -12, -4,
                                    44, 47, 16,
                                    // output 2
                                    -28, -28, -12,
                                    -28, -28, -12,
                                    102, 109, 48},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offset_output_channels"),
        DeformableConvolutionParams(PartialShape {2, 1, 4, 4},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {2, 8, 3, 3},
                          PartialShape {2, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    //batch 1
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16,
                                    //batch 2
                                    17, 18, 19, 20,
                                    21, 22, 23, 24,
                                    25, 26, 27, 28,
                                    29, 30, 31, 32},
                          std::vector<T>{
                                    1, 2,
                                    -1, -2},
                          std::vector<T>{
                                    // batch1
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    // batch2
                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0},
                          std::vector<T>{
                                    // batch 1
                                    -12, -12, -4,
                                    -12, -12, -4,
                                    44, 47, 16,
                                    // batch 2
                                    -12, -12, -12,
                                    -12, -12, -12,
                                    -12, -12, -12},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offset_batch"),
        DeformableConvolutionParams(PartialShape {1, 4, 3, 3},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 8, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1, 2, 3,
                                    4, 5, 6,
                                    7, 8, 9,
                                     // channel 2
                                    10, 11, 12,
                                    13, 14, 15,
                                    16, 17, 18,
                                    // channel 3
                                    19, 20, 21,
                                    22, 23, 24,
                                    25, 26, 27,
                                    // channel 4
                                    28, 29, 30,
                                    31, 32, 33,
                                    34, 35, 36},
                          std::vector<T>{
                                    // filter 1 channel 1
                                    1, 2,
                                    3, 4,
                                     // filter 1 channel 2
                                    5, 6,
                                    7, 8,
                                     // filter 2 channel 1
                                    -1, -2,
                                    -3, -4,
                                    // filter 2 channel 2
                                    -5, -6,
                                    -7, -8},
                          std::vector<T>{
                                    // window 1 (F_Y=0, F_X=0) -> I_Y coordinate
                                    1, 0, 2, 1, // out1 .. out 4
                                    // window 1 (F_Y=0, F_X=0) -> I_X coordinate
                                    0, 1, 1, 2, // out1 .. out 4
                                    // window 2 (F_Y=0, F_X=1) -> I_Y coordinate
                                    1, 1, 1, 1, // out1 .. out 4
                                    // window 2 (F_Y=0, F_X=1) -> I_X coordinate
                                    1, 1, 1, 1, // out1 .. out 4
                                    // window 3 (F_Y=1, F_X=0) -> I_Y coordinate
                                    2, 2, 2, 2, // out1 .. out 4
                                    // window 3 (F_Y=1, F_X=0) -> I_X coordinate
                                    2, 2, 2, 2, // out1 .. out 4
                                    // window 4 (F_Y=1, F_X=1) -> I_Y coordinate
                                    2, 2, 2, 2, // out1 .. out 4
                                    // window 4 (F_Y=1, F_X=1) -> I_X coordinate
                                    2, 2, 2, 2}, // out1 .. out 4
                          std::vector<T>{
                                    // channel 1
                                    171, 63,
                                    126, 0,
                                    // channel 2
                                    -423, -171,
                                    -270, 0},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          2,
                          1,
                          "integral_offset_groups_basic"),
        DeformableConvolutionParams(PartialShape {1, 8, 3, 3},
                          PartialShape {4, 2, 2, 2},
                          PartialShape {1, 8, 2, 2},
                          PartialShape {1, 4, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1, 2, 3,
                                    4, 5, 6,
                                    7, 8, 9,
                                     // channel 2
                                    10, 11, 12,
                                    13, 14, 15,
                                    16, 17, 18,
                                    // channel 3
                                    19, 20, 21,
                                    22, 23, 24,
                                    25, 26, 27,
                                    // channel 4
                                    28, 29, 30,
                                    31, 32, 33,
                                    34, 35, 36,
                                     // channel 5
                                    37, 38, 39,
                                    40, 41, 42,
                                    43, 44, 45,
                                     // channel 6
                                    46, 47, 48,
                                    49, 50, 51,
                                    52, 53, 54,
                                     // channel 7
                                    55, 56, 57,
                                    58, 59, 60,
                                    61, 62, 63,
                                     // channel 8
                                    64, 65, 66,
                                    67, 68, 69,
                                    70, 71, 72},
                          std::vector<T>{
                                    // filter 1 channel 1
                                    1, 2,
                                    3, 4,
                                    // filter 1 channel 2
                                    5, 6,
                                    7, 8,
                                    // filter 2 channel 1
                                    9, 10,
                                    11, 12,
                                    // filter 2 channel 2
                                    13, 14,
                                    15, 16,
                                    // filter 3 channel 1
                                    -1, -2,
                                    -3, -4,
                                    // filter 3 channel 2
                                    -5, -6,
                                    -7, -8,
                                    // filter 4 channel 1
                                    -9, -10,
                                    -11, -12,
                                    // filter 4 channel 2
                                    -13, -14,
                                    -15, -16},
                          std::vector<T>{
                                    1, 1, 1, 1,
                                    1, 1, 1, 1,
                                    1, 1, 1, 1,
                                    1, 1, 1, 1,
                                    1, 1, 1, 1,
                                    1, 1, 1, 1,
                                    1, 1, 1, 1,
                                    1, 1, 1, 1},
                          std::vector<T>{
                                    // channel 1
                                    500, 234,
                                    219, 99,
                                    // channel 2
                                    3036, 1482,
                                    1463, 711,
                                    // channel 3
                                    -1796, -810,
                                    -723, -315,
                                    // channel 4
                                    -6636, -3210,
                                    -3119, -1503},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          4,
                          1,
                          "integral_offset_groups_complex"),
        DeformableConvolutionParams(PartialShape {1, 2, 4, 4},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 16, 3, 3},
                          PartialShape {1, 2, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16,
                                    // channel 2
                                    17, 18, 19, 20,
                                    21, 22, 23, 24,
                                    25, 26, 27, 28,
                                    29, 30, 31, 32},
                          std::vector<T>{
                                    // f1: channel 1
                                    1, 2,
                                    3, 4,
                                    // f1: channel 2
                                    5, 6,
                                    7, 8,
                                    // f2: channel 1
                                    -1, -2,
                                    -3, -4,
                                    // f2: channel 2
                                    -5, -6,
                                    -7, -8},
                          std::vector<T>{
                                    // defgroup 1
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    //  defgroup 2
                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0},
                          std::vector<T>{
                                    // output 1
                                    610, 646, 612,
                                    754, 790, 732,
                                    768, 797, 792,
                                    // output 2
                                    -610, -646, -612,
                                    -754, -790, -732,
                                    -768, -797, -792},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          2,
                          "integral_offset_deforgroup_basic"),
        DeformableConvolutionParams(PartialShape {1, 4, 4, 4},
                          PartialShape {2, 4, 2, 2},
                          PartialShape {1, 32, 3, 3},
                          PartialShape {1, 2, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16,
                                    // channel 2
                                    17, 18, 19, 20,
                                    21, 22, 23, 24,
                                    25, 26, 27, 28,
                                    29, 30, 31, 32,
                                    // channel 3
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16,
                                    // channel 4
                                    17, 18, 19, 20,
                                    21, 22, 23, 24,
                                    25, 26, 27, 28,
                                    29, 30, 31, 32},
                          std::vector<T>{
                                    // f1: channel 1
                                    1, 2,
                                    3, 4,
                                    // f1: channel 2
                                    5, 6,
                                    7, 8,
                                    // f1: channel 3
                                    1, 2,
                                    3, 4,
                                    // f1: channel 4
                                    5, 6,
                                    7, 8,
                                    // f2: channel 1
                                    -1, -2,
                                    -3, -4,
                                    // f2: channel 2
                                    -5, -6,
                                    -7, -8,
                                    // f2: channel 3
                                    -1, -2,
                                    -3, -4,
                                    // f2: channel 4
                                    -5, -6,
                                    -7, -8},
                          std::vector<T>{
                                    // defgroup 1
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    //  defgroup 2
                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    // defgroup 3
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    //  defgroup 4
                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0},
                          std::vector<T>{
                                    // output 1
                                    1220, 1292, 1224,
                                    1508, 1580, 1464,
                                    1536, 1594, 1584,
                                    // output 2
                                    -1220, -1292, -1224,
                                    -1508, -1580, -1464,
                                    -1536, -1594, -1584},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          4,
                          "integral_offset_deforgroup_complex1"),
        DeformableConvolutionParams(PartialShape {1, 4, 4, 4},
                          PartialShape {2, 4, 2, 2},
                          PartialShape {1, 16, 3, 3},
                          PartialShape {1, 2, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16,
                                    // channel 2
                                    17, 18, 19, 20,
                                    21, 22, 23, 24,
                                    25, 26, 27, 28,
                                    29, 30, 31, 32,
                                    // channel 3
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16,
                                    // channel 4
                                    17, 18, 19, 20,
                                    21, 22, 23, 24,
                                    25, 26, 27, 28,
                                    29, 30, 31, 32},
                          std::vector<T>{
                                    // f1: channel 1
                                    1, 2,
                                    3, 4,
                                    // f1: channel 2
                                    5, 6,
                                    7, 8,
                                    // f1: channel 3
                                    1, 2,
                                    3, 4,
                                    // f1: channel 4
                                    5, 6,
                                    7, 8,
                                    // f2: channel 1
                                    -1, -2,
                                    -3, -4,
                                    // f2: channel 2
                                    -5, -6,
                                    -7, -8,
                                    // f2: channel 3
                                    -1, -2,
                                    -3, -4,
                                    // f2: channel 4
                                    -5, -6,
                                    -7, -8},
                          std::vector<T>{
                                    // defgroup 1
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    //  defgroup 2
                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0},
                          std::vector<T>{
                                    // output 1
                                    1300, 1372, 992,
                                    1588, 1660, 1200,
                                    1228, 1278, 1096,
                                    // output 2
                                    -1300, -1372, -992,
                                    -1588, -1660, -1200,
                                    -1228, -1278, -1096},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          2,
                          "integral_offset_deforgroup_complex2"),
        DeformableConvolutionParams(PartialShape {1, 4, 3, 3},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 16, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1, 2, 3,
                                    4, 5, 6,
                                    7, 8, 9,
                                    10, 11, 12,
                                    13, 14, 15,
                                    16, 17, 18,
                                    19, 20, 21,
                                    22, 23, 24,
                                    25, 26, 27,
                                    28, 29, 30,
                                    31, 32, 33,
                                    34, 35, 36},
                          std::vector<T>{
                                    1, 2,
                                    3, 4,
                                    5, 6,
                                    7, 8,
                                    -1, -2,
                                    -3, -4,
                                    -5, -6,
                                    -7, -8},
                          std::vector<T>{
                                    // defgroup 1
                                    1, 1, 1, 1,
                                    1, 1, 1, 1,
                                    1, 1, 1, 1,
                                    1, 1, 1, 1,
                                    1, 1, 1, 1,
                                    1, 1, 1, 1,
                                    1, 1, 1, 1,
                                    1, 1, 1, 1,
                                    // defgroup 2
                                    0, 0, 0, 0,
                                    0, 0, 0, 0,
                                    0, 0, 0, 0,
                                    0, 0, 0, 0,
                                    0, 0, 0, 0,
                                    0, 0, 0, 0,
                                    0, 0, 0, 0,
                                    0, 0, 0, 0},
                          std::vector<T>{
                                    500, 234,
                                    219, 99,
                                    -1004, -1040,
                                    -1112, -1148},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          2,
                          2,
                          "integral_offset_groups_and_deforgroups"),
    };
    return deformableConvolutionParams;
}

template <element::Type_t IN_ET>
std::vector<DeformableConvolutionParams> generateDeformableConvolutionInt8Params() {
    using T = typename element_type_traits<IN_ET>::value_type;

    std::vector<DeformableConvolutionParams> deformableConvolutionParams {
// --------------------- 2D DeformableConvolution ------------------------------------------
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16},
                          std::vector<T>{
                                    1, 2,
                                    -1, -2},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    -12, -12, -12,
                                    -12, -12, -12,
                                    -12, -12, -12},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 3, 3},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {1, 8, 4, 4},
                          PartialShape {1, 1, 4, 4},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 5,
                                    7, 5, 3,
                                    1, 3, 5},
                          std::vector<T>{
                                    1, 2,
                                    0, 1},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    1, 3, 5, 0,
                                    9, 12, 16, 5,
                                    15, 20, 16, 3,
                                    2, 7, 13, 5},
                          {1, 1},
                          {1, 1},
                          {1, 1},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 5, 5},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 5, 7, 9,
                                    7, 5, 3, 1, 0,
                                    2, 4, 6, 8, 10,
                                    8, 6, 4, 2, 0,
                                    2, 4, 6, 8, 10},
                          std::vector<T>{
                                    1, 2, 3,
                                    1, 1, 1,
                                    3, 2, 1},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    57, 94,
                                    66, 102},
                          {2, 2},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {2, 1, 4, 4},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {2, 18, 2, 2},
                          PartialShape {2, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    // batch 1
                                    1, 3, 2, 1,
                                    1, 3, 3, 1,
                                    2, 1, 1, 3,
                                    3, 2, 3, 3,
                                    // batch 2
                                    -1, 3, 2, -1,
                                    1, 3, -3, 1,
                                    -2, -1, 1, 3,
                                    3, 2, 3, -3},
                          std::vector<T>{
                                   -5, 3, 5,
                                    1, -3, 1,
                                    4, 2, -4},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    // batch 1
                                    15, -15,
                                    23, 2,
                                    // batch 2
                                    -1, -15,
                                    -5, 6},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 2, 4, 4},
                          PartialShape {1, 2, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    // channel 1
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16,
                                    // channel 2
                                    17, 18, 19, 20,
                                    21, 22, 23, 24,
                                    25, 26, 27, 28,
                                    29, 30, 31, 32},
                          std::vector<T>{
                                    // channel 1
                                    1, 2,
                                    -1, -2,
                                    // channel 2
                                    3, 4,
                                    -3, -4},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    -40, -40, -40,
                                    -40, -40, -40,
                                    -40, -40, -40},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16},
                          std::vector<T>{
                                    1, 2,
                                    -1, -2},
                          std::vector<T>{
                                    // window 1 (Y=0, X=0) -> Y coordinate
                                    1, 1, 1, // out1 .. out 3
                                    1, 1, 1, // out4 .. out 6
                                    1, 1, 1, // out7 .. out 9
                                    // window 1 (Y=0, X=0) -> X coordinate
                                    1, 1, 1, // out1 .. out 3
                                    1, 1, 1, // out4 .. out 6
                                    1, 1, 1, // out7 .. out 9
                                    // window 2
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    // window 2
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    // window 3
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    // window 3
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    // window 4
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    // window 4
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1},
                          std::vector<T>{
                                    -12, -12, -4,
                                    -12, -12, -4,
                                    44, 47, 16},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offsets_1"),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 4, 4},
                          PartialShape {1, 1, 4, 4},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 7, 7,
                                    7, 6, 3, 1,
                                    4, 4, 2, 8,
                                    1, 1, 1, 2},
                          std::vector<T>{
                                    1, 2, 3,
                                    0, 1, 0,
                                    3, 2, 1},
                          std::vector<T>{
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2},
                          std::vector<T>{
                                    56, 39, 44, 18,
                                    38, 56, 65, 0,
                                    19, 38, 20, 20,
                                    6, 19, 33, 0},
                          {1, 1},
                          {1, 1},
                          {1, 1},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 5, 5},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 5, 7, 9,
                                    7, 5, 3, 1, 0,
                                    2, 4, 6, 8, 10,
                                    8, 6, 4, 2, 0,
                                    2, 4, 6, 8, 10},
                          std::vector<T>{
                                    1, 2, 3,
                                    1, 1, 1,
                                    3, 2, 1},
                          std::vector<T>{
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0},
                          std::vector<T>{
                                    57, 40,
                                    38, 102},
                          {2, 2},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offsets_stride"),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {2, 1, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 2, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16},
                          std::vector<T>{
                                    // filter 1
                                    1, 2,
                                    -1, -2,
                                    // filter 2
                                    3, 4,
                                    -3, -4},
                          std::vector<T>{
                                    //channel 1: Y offsets
                                    1, 1, 1, // out1 .. out 3
                                    1, 1, 1, // out4 .. out 6
                                    1, 1, 1, // out7 .. out 9
                                    //channel 1: X offsets
                                    1, 1, 1, // out1 .. out 3
                                    1, 1, 1, // out4 .. out 6
                                    1, 1, 1, // out7 .. out 9
                                    //channel 2: Y offsets
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    //channel 2: X offsets
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    //channel 3: Y offsets
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    //channel 3: X offsets
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    //channel 4: Y offsets
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    //channel 4: X offsets
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1},
                          std::vector<T>{
                                    // output 1
                                    -12, -12, -4,
                                    -12, -12, -4,
                                    44, 47, 16,
                                    // output 2
                                    -28, -28, -12,
                                    -28, -28, -12,
                                    102, 109, 48},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offset_output_channels"),
        DeformableConvolutionParams(PartialShape {2, 1, 4, 4},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {2, 8, 3, 3},
                          PartialShape {2, 1, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    //batch 1
                                    1, 2, 3, 4,
                                    5, 6, 7, 8,
                                    9, 10, 11, 12,
                                    13, 14, 15, 16,
                                    //batch 2
                                    17, 18, 19, 20,
                                    21, 22, 23, 24,
                                    25, 26, 27, 28,
                                    29, 30, 31, 32},
                          std::vector<T>{
                                    1, 2,
                                    -1, -2},
                          std::vector<T>{
                                    // batch1
                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,

                                    1, 1, 1,
                                    1, 1, 1,
                                    1, 1, 1,
                                    // batch2
                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0,

                                    0, 0, 0,
                                    0, 0, 0,
                                    0, 0, 0},
                          std::vector<T>{
                                    // batch 1
                                    -12, -12, -4,
                                    -12, -12, -4,
                                    44, 47, 16,
                                    // batch 2
                                    -12, -12, -12,
                                    -12, -12, -12,
                                    -12, -12, -12},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offset_batch"),
    };
    return deformableConvolutionParams;
}

template <element::Type_t IN_ET>
std::vector<DeformableConvolutionParams> generateDeformableConvolutionUintParams() {
    using T = typename element_type_traits<IN_ET>::value_type;

    std::vector<DeformableConvolutionParams> deformableConvolutionParams {
// --------------------- 2D DeformableConvolution ------------------------------------------
        DeformableConvolutionParams(PartialShape {1, 1, 3, 3},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {1, 8, 4, 4},
                          PartialShape {1, 1, 4, 4},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 5,
                                    7, 5, 3,
                                    1, 3, 5},
                          std::vector<T>{
                                    1, 2,
                                    0, 1},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    1, 3, 5, 0,
                                    9, 12, 16, 5,
                                    15, 20, 16, 3,
                                    2, 7, 13, 5},
                          {1, 1},
                          {1, 1},
                          {1, 1},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 5, 5},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 5, 7, 9,
                                    7, 5, 3, 1, 0,
                                    2, 4, 6, 8, 10,
                                    8, 6, 4, 2, 0,
                                    2, 4, 6, 8, 10},
                          std::vector<T>{
                                    1, 2, 3,
                                    1, 1, 1,
                                    3, 2, 1},
                          std::vector<T>{
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          std::vector<T>{
                                    57, 94,
                                    66, 102},
                          {2, 2},
                          {0, 0},
                          {0, 0},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 4, 4},
                          PartialShape {1, 1, 4, 4},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 7, 7,
                                    7, 6, 3, 1,
                                    4, 4, 2, 8,
                                    1, 1, 1, 2},
                          std::vector<T>{
                                    1, 2, 3,
                                    0, 1, 0,
                                    3, 2, 1},
                          std::vector<T>{
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2,

                                    1, 0, 1, 0,
                                    1, 0, 0, 2,
                                    1, 0, 1, 0,
                                    1, 0, 0, 2},
                          std::vector<T>{
                                    56, 39, 44, 18,
                                    38, 56, 65, 0,
                                    19, 38, 20, 20,
                                    6, 19, 33, 0},
                          {1, 1},
                          {1, 1},
                          {1, 1},
                          {1, 1}),
        DeformableConvolutionParams(PartialShape {1, 1, 5, 5},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 18, 2, 2},
                          PartialShape {1, 1, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1, 3, 5, 7, 9,
                                    7, 5, 3, 1, 0,
                                    2, 4, 6, 8, 10,
                                    8, 6, 4, 2, 0,
                                    2, 4, 6, 8, 10},
                          std::vector<T>{
                                    1, 2, 3,
                                    1, 1, 1,
                                    3, 2, 1},
                          std::vector<T>{
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0,
                                    0, 2,
                                    1, 0},
                          std::vector<T>{
                                    57, 40,
                                    38, 102},
                          {2, 2},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "integral_offsets_stride"),
    };
    return deformableConvolutionParams;
}

template <element::Type_t IN_ET>
std::vector<DeformableConvolutionParams> generateDeformableConvolutionV8MaskParams() {
    using T = typename element_type_traits<IN_ET>::value_type;

    std::vector<DeformableConvolutionParams> deformableConvolutionParams {
        DeformableConvolutionParams(PartialShape {1, 1, 4, 4},
                          PartialShape {1, 1, 2, 2},
                          PartialShape {1, 8, 3, 3},
                          PartialShape {1, 1, 3, 3},
                          PartialShape {1, 4, 3, 3},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f, 4.0f,
                                    5.0f, 6.0f, 7.0f, 8.0f,
                                    9.0f, 10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f, 16.0f},
                          std::vector<T>{
                                    1.0f, 2.0f,
                                    -1.0f, -2.0f},
                          std::vector<T>{
                                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
                          std::vector<T>{
                                    -6.0f, -6.0f, -6.0f,
                                    -6.0f, -6.0f, -6.0f,
                                    -6.0f, -6.0f, -6.0f},
                          std::vector<T>{
                                    0.5f, 0.5f, 0.5f,
                                    0.5f, 0.5f, 0.5f,
                                    0.5f, 0.5f, 0.5f,
                                    0.5f, 0.5f, 0.5f,
                                    0.5f, 0.5f, 0.5f,
                                    0.5f, 0.5f, 0.5f,
                                    0.5f, 0.5f, 0.5f,
                                    0.5f, 0.5f, 0.5f,
                                    0.5f, 0.5f, 0.5f,
                                    0.5f, 0.5f, 0.5f,
                                    0.5f, 0.5f, 0.5f,
                                    0.5f, 0.5f, 0.5f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          1,
                          "v8_zeroed_offset_default_mask",
                          true),
        DeformableConvolutionParams(PartialShape {1, 4, 3, 3},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 16, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          PartialShape {1, 8, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    4.0f, 5.0f, 6.0f,
                                    7.0f, 8.0f, 9.0f,
                                    10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f,
                                    16.0f, 17.0f, 18.0f,
                                    19.0f, 20.0f, 21.0f,
                                    22.0f, 23.0f, 24.0f,
                                    25.0f, 26.0f, 27.0f,
                                    28.0f, 29.0f, 30.0f,
                                    31.0f, 32.0f, 33.0f,
                                    34.0f, 35.0f, 36.0f},
                          std::vector<T>{
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    220.15443f ,   38.199608f,
                                    32.643005f,   59.340614f,
                                    -419.0005f  , -252.08015f,
                                    -182.44444f , -165.99335f},
                          std::vector<T>{
                                    0.64f,
                                    0.18f,
                                    0.23f,
                                    0.74f,
                                    0.89f,
                                    0.70f,
                                    0.13f,
                                    0.99f,
                                    0.48f,
                                    0.20f,
                                    0.67f,
                                    0.88f,
                                    0.17f,
                                    0.19f,
                                    0.53f,
                                    0.22f,
                                    0.50f,
                                    0.07f,
                                    0.21f,
                                    0.99f,
                                    0.09f,
                                    0.28f,
                                    0.66f,
                                    0.91f,
                                    0.28f,
                                    0.89f,
                                    0.91f,
                                    0.39f,
                                    0.70f,
                                    0.67f,
                                    0.26f,
                                    0.09f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          2,
                          2,
                          "v8_real_offset_groups_deforgroups_mask",
                          true),
        DeformableConvolutionParams(PartialShape {1, 2, 3, 3},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 16, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          PartialShape {1, 8, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    4.0f, 5.0f, 6.0f,
                                    7.0f, 8.0f, 9.0f,
                                    10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f,
                                    16.0f, 17.0f, 18.0f},
                          std::vector<T>{
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f,
                                    1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f},
                          std::vector<T>{
                                    184.25163,  101.04752,
                                    77.46842,   77.56562,
                                    -184.25163, -101.04752,
                                    -77.46842,  -77.56562},
                          std::vector<T>{
                                    0.64f,
                                    0.18f,
                                    0.23f,
                                    0.74f,
                                    0.89f,
                                    0.70f,
                                    0.13f,
                                    0.99f,
                                    0.48f,
                                    0.20f,
                                    0.67f,
                                    0.88f,
                                    0.17f,
                                    0.19f,
                                    0.53f,
                                    0.22f,
                                    0.50f,
                                    0.07f,
                                    0.21f,
                                    0.99f,
                                    0.09f,
                                    0.28f,
                                    0.66f,
                                    0.91f,
                                    0.28f,
                                    0.89f,
                                    0.91f,
                                    0.39f,
                                    0.70f,
                                    0.67f,
                                    0.26f,
                                    0.09f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          2,
                          "v8_real_offset_groups_deforgroups_mask_2",
                          true),
        DeformableConvolutionParams(PartialShape {1, 2, 3, 3},
                          PartialShape {2, 2, 2, 2},
                          PartialShape {1, 16, 2, 2},
                          PartialShape {1, 2, 2, 2},
                          PartialShape {1, 8, 2, 2},
                          IN_ET,
                          std::vector<T>{
                                    1.0f, 2.0f, 3.0f,
                                    4.0f, 5.0f, 6.0f,
                                    7.0f, 8.0f, 9.0f,
                                    10.0f, 11.0f, 12.0f,
                                    13.0f, 14.0f, 15.0f,
                                    16.0f, 17.0f, 18.0f},
                          std::vector<T>{
                                    1.0f, 2.0f,
                                    3.0f, 4.0f,
                                    5.0f, 6.0f,
                                    7.0f, 8.0f,
                                    -1.0f, -2.0f,
                                    -3.0f, -4.0f,
                                    -5.0f, -6.0f,
                                    -7.0f, -8.0f},
                          std::vector<T>{
                                    -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f,
                                    -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f,
                                    -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f,
                                    -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f, -1.1f},
                          std::vector<T>{
                                    45.910797,  104.8302,
                                    63.12059 ,  151.47789,
                                    -45.910797, -104.8302,
                                    -63.12059 , -151.47789},
                          std::vector<T>{
                                    0.64f,
                                    0.18f,
                                    0.23f,
                                    0.74f,
                                    0.89f,
                                    0.70f,
                                    0.13f,
                                    0.99f,
                                    0.48f,
                                    0.20f,
                                    0.67f,
                                    0.88f,
                                    0.17f,
                                    0.19f,
                                    0.53f,
                                    0.22f,
                                    0.50f,
                                    0.07f,
                                    0.21f,
                                    0.99f,
                                    0.09f,
                                    0.28f,
                                    0.66f,
                                    0.91f,
                                    0.28f,
                                    0.89f,
                                    0.91f,
                                    0.39f,
                                    0.70f,
                                    0.67f,
                                    0.26f,
                                    0.09f},
                          {1, 1},
                          {0, 0},
                          {0, 0},
                          {1, 1},
                          1,
                          2,
                          "v8_neg_offset_groups_deforgroups_mask",
                          true),
    };
    return deformableConvolutionParams;
}

std::vector<DeformableConvolutionParams> generateDeformableConvolutionCombinedParams() {
    const std::vector<std::vector<DeformableConvolutionParams>> deformableConvolutionTypeParams {
        generateDeformableConvolutionFloatParams<element::Type_t::f64>(),
        generateDeformableConvolutionFloatParams<element::Type_t::f32>(),
        generateDeformableConvolutionFloatParams<element::Type_t::f16>(),
        generateDeformableConvolutionFloatParams<element::Type_t::bf16>(),
        generateDeformableConvolutionIntParams<element::Type_t::i64>(),
        generateDeformableConvolutionIntParams<element::Type_t::i32>(),
        generateDeformableConvolutionIntParams<element::Type_t::i16>(),
        generateDeformableConvolutionInt8Params<element::Type_t::i8>(),
        generateDeformableConvolutionUintParams<element::Type_t::u64>(),
        generateDeformableConvolutionUintParams<element::Type_t::u32>(),
        generateDeformableConvolutionUintParams<element::Type_t::u16>(),
        generateDeformableConvolutionUintParams<element::Type_t::u8>()
        };
    std::vector<DeformableConvolutionParams> combinedParams;

    for (const auto& params : deformableConvolutionTypeParams) {
        combinedParams.insert(combinedParams.end(), params.begin(), params.end());
    }
    return combinedParams;
}

std::vector<DeformableConvolutionParams> generateDeformableConvolutionV8CombinedParams() {
    const std::vector<std::vector<DeformableConvolutionParams>> deformableConvolutionTypeParams {
        generateDeformableConvolutionFloatParams<element::Type_t::f64>(),
        generateDeformableConvolutionFloatParams<element::Type_t::f32>(),
        generateDeformableConvolutionFloatParams<element::Type_t::f16>(),
        generateDeformableConvolutionFloatParams<element::Type_t::bf16>(),
        generateDeformableConvolutionV8MaskParams<element::Type_t::f64>(),
        generateDeformableConvolutionV8MaskParams<element::Type_t::f32>(),
        generateDeformableConvolutionV8MaskParams<element::Type_t::f16>(),
        generateDeformableConvolutionV8MaskParams<element::Type_t::bf16>(),
        generateDeformableConvolutionIntParams<element::Type_t::i64>(),
        generateDeformableConvolutionIntParams<element::Type_t::i32>(),
        generateDeformableConvolutionIntParams<element::Type_t::i16>(),
        generateDeformableConvolutionInt8Params<element::Type_t::i8>(),
        generateDeformableConvolutionUintParams<element::Type_t::u64>(),
        generateDeformableConvolutionUintParams<element::Type_t::u32>(),
        generateDeformableConvolutionUintParams<element::Type_t::u16>(),
        generateDeformableConvolutionUintParams<element::Type_t::u8>()
        };
    std::vector<DeformableConvolutionParams> combinedParams;

    for (const auto& params : deformableConvolutionTypeParams) {
        combinedParams.insert(combinedParams.end(), params.begin(), params.end());
    }
    return combinedParams;
}

INSTANTIATE_TEST_SUITE_P(smoke_DeformableConvolution_With_Hardcoded_Refs, ReferenceDeformableConvolutionLayerTest,
    testing::ValuesIn(generateDeformableConvolutionCombinedParams()), ReferenceDeformableConvolutionLayerTest::getTestCaseName);

INSTANTIATE_TEST_SUITE_P(smoke_DeformableConvolutionV8_With_Hardcoded_Refs, ReferenceDeformableConvolutionV8LayerTest,
    testing::ValuesIn(generateDeformableConvolutionV8CombinedParams()), ReferenceDeformableConvolutionV8LayerTest::getTestCaseName);

} // namespace
