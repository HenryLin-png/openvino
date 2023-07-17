// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "include/auto_unit_test.hpp"

using namespace ov::mock_auto_plugin;
using Config = std::map<std::string, std::string>;
using ConfigParams = std::tuple<std::vector<std::string>>;

// define a matcher to check if perf hint expects
MATCHER_P(ComparePerfHint, perfHint, "Check if perf hint expects.") {
    ov::Any arg_perfHint = "";
    auto itor = arg.find(ov::hint::performance_mode.name());
    if (itor != arg.end()) {
        arg_perfHint = itor->second;
    }

    return perfHint == arg_perfHint.as<ov::hint::PerformanceMode>();
}

class LoadNetworkWithCTPUTMockTest :    public tests::AutoTest,
                                        public ::testing::TestWithParam<ConfigParams> {
public:
    static std::string getTestCaseName(testing::TestParamInfo<ConfigParams> obj) {
        std::vector<std::string> targetDevices;
        std::tie(targetDevices) = obj.param;
        std::ostringstream result;
        result << "ctput_loadnetwork_to_device_";
        for (auto& device : targetDevices) {
            if (device == targetDevices.back()) {
                result << device;
            } else {
                result << device << "_";
            }
        }
        return result.str();
    }

    void SetUp() override {
        std::vector<std::string> availableDevs = {"CPU", "GPU"};
        ON_CALL(*core, get_available_devices()).WillByDefault(Return(availableDevs));
        ON_CALL(*core, compile_model(::testing::Matcher<const std::shared_ptr<const ov::Model>&>(_),
            ::testing::Matcher<const std::string&>(StrEq(CommonTestUtils::DEVICE_CPU)), _))
            .WillByDefault(Return(mockExeNetwork));
        ON_CALL(*core, compile_model(::testing::Matcher<const std::shared_ptr<const ov::Model>&>(_),
                    ::testing::Matcher<const std::string&>(StrEq(CommonTestUtils::DEVICE_GPU)), _))
                    .WillByDefault(Return(mockExeNetworkActual));
    }
};

TEST_P(LoadNetworkWithCTPUTMockTest, CTPUTSingleDevLogicTest) {
    std::vector<std::string> targetDevices;
    std::tie(targetDevices) = this->GetParam();

    plugin->set_device_name("AUTO");
    config.insert(ov::hint::performance_mode(ov::hint::PerformanceMode::CUMULATIVE_THROUGHPUT));

    if (targetDevices.size() == 1) {
        std::string targetDevice = targetDevices[0];
        config.insert({InferenceEngine::MultiDeviceConfigParams::KEY_MULTI_DEVICE_PRIORITIES, targetDevices[0]});
        // Call single device logic and performance hint is THROUGHPUT
        EXPECT_CALL(*core,
                    compile_model(::testing::Matcher<const std::shared_ptr<const ov::Model>&>(_),
                                ::testing::Matcher<const std::string&>(targetDevice),
                                ::testing::Matcher<const ov::AnyMap&>(
                                    ComparePerfHint(ov::hint::PerformanceMode::THROUGHPUT))))
            .Times(1);
        // if target device only has GPU, no CPU helper to be called
        if (targetDevice.find("GPU") != std::string::npos) {
            EXPECT_CALL(*core,
                        compile_model(::testing::Matcher<const std::shared_ptr<const ov::Model>&>(_),
                                    ::testing::Matcher<const std::string&>(CommonTestUtils::DEVICE_CPU),
                                    ::testing::Matcher<const ov::AnyMap&>(
                                        ComparePerfHint(ov::hint::PerformanceMode::LATENCY))))
                .Times(0);
        }
    } else {
        std::string targetDev;
        for (auto& deviceName : targetDevices) {
            targetDev += deviceName;
            targetDev += ((deviceName == targetDevices.back()) ? "" : ",");
            EXPECT_CALL(*core,
                        compile_model(::testing::Matcher<const std::shared_ptr<const ov::Model>&>(_),
                                    ::testing::Matcher<const std::string&>(deviceName),
                                    ::testing::Matcher<const ov::AnyMap&>(
                                        ComparePerfHint(ov::hint::PerformanceMode::THROUGHPUT))))
                .Times(1);
        }
        config.insert(ov::device::priorities(targetDev));
        // no CPU helper to be called
        EXPECT_CALL(*core,
                    compile_model(::testing::Matcher<const std::shared_ptr<const ov::Model>&>(_),
                                ::testing::Matcher<const std::string&>(CommonTestUtils::DEVICE_CPU),
                                ::testing::Matcher<const ov::AnyMap&>(
                                    ComparePerfHint(ov::hint::PerformanceMode::LATENCY))))
            .Times(0);
    }

    ASSERT_NO_THROW(plugin->compile_model(model, config));
}

using LoadNetworkWithCTPUTMockTestExeDevice = LoadNetworkWithCTPUTMockTest;
TEST_P(LoadNetworkWithCTPUTMockTestExeDevice, CTPUTSingleDevExecutionDevie) {
    std::vector<std::string> targetDevices;
    std::shared_ptr<ov::ICompiledModel> exeNetwork;
    std::tie(targetDevices) = this->GetParam();

    plugin->set_device_name("AUTO");
    config.insert({{CONFIG_KEY(PERFORMANCE_HINT), InferenceEngine::PluginConfigParams::CUMULATIVE_THROUGHPUT}});
    config.insert(ov::device::priorities(targetDevices[0]));
    // Call single device logic and performance hint is THROUGHPUT
    ASSERT_NO_THROW(exeNetwork = plugin->compile_model(model, config));
    EXPECT_EQ(exeNetwork->get_property(ov::execution_devices.name()).as<std::string>(), CommonTestUtils::DEVICE_CPU);
}

const std::vector<ConfigParams> testConfigs = {
    ConfigParams{{"CPU"}},
    ConfigParams{{"GPU"}},
    ConfigParams{{"CPU", "GPU"}},
    ConfigParams{{"GPU", "CPU"}},
};

INSTANTIATE_TEST_SUITE_P(smoke_AutoMock_CTPUTSingleDevLogicTest,
                         LoadNetworkWithCTPUTMockTest,
                         ::testing::ValuesIn(testConfigs),
                         LoadNetworkWithCTPUTMockTest::getTestCaseName);

const std::vector<ConfigParams> executionDevieTestConfigs = {
    ConfigParams{{"CPU"}},
};

INSTANTIATE_TEST_SUITE_P(smoke_AutoCTPUTExecutionDevice,
                         LoadNetworkWithCTPUTMockTestExeDevice,
                         ::testing::ValuesIn(executionDevieTestConfigs),
                         LoadNetworkWithCTPUTMockTestExeDevice::getTestCaseName);

using ConfigParams_1 = std::tuple<bool, std::vector<std::string>>;
class AutoCTPUTCallMulti : public tests::AutoTest, public ::testing::TestWithParam<ConfigParams_1> {
public:
    static std::string getTestCaseName(testing::TestParamInfo<ConfigParams_1> obj) {
        std::vector<std::string> targetDevices;
        bool AutoCallMulti;
        std::tie(AutoCallMulti, targetDevices) = obj.param;
        std::ostringstream result;
        if (AutoCallMulti) {
            result << "AutoCallMulti_";
        } else {
            result << "Multi_";
        }
        result << "ctput_loadnetwork_to_device_";
        for (auto& device : targetDevices) {
            if (device == targetDevices.back()) {
                result << device;
            } else {
                result << device << "_";
            }
        }
        return result.str();
    }
    void SetUp() override {
        std::vector<std::string> availableDevs = {"CPU", "GPU"};
        ON_CALL(*core, get_available_devices()).WillByDefault(Return(availableDevs));
        ON_CALL(*core, compile_model(::testing::Matcher<const std::shared_ptr<const ov::Model>&>(_),
            ::testing::Matcher<const std::string&>(StrEq(CommonTestUtils::DEVICE_CPU)), _))
            .WillByDefault(Return(mockExeNetwork));
        ON_CALL(*core, compile_model(::testing::Matcher<const std::shared_ptr<const ov::Model>&>(_),
                    ::testing::Matcher<const std::string&>(StrEq(CommonTestUtils::DEVICE_GPU)), _))
                    .WillByDefault(Return(mockExeNetworkActual));
    }
};

TEST_P(AutoCTPUTCallMulti, CTPUTDeviceLoadFailedNoExceptionThrowTest) {
    std::vector<std::string> targetDevices;
    std::string targetDev;
    bool AutoCallMulti;
    std::tie(AutoCallMulti, targetDevices) = this->GetParam();
    std::string loadFailedDevice = targetDevices.size() > 0 ? targetDevices[0] : "";
    std::string secondDevice = targetDevices.size() > 1 ? targetDevices[1] : "";
    plugin->set_device_name("MULTI");
    for (auto& deviceName : targetDevices) {
        targetDev += deviceName;
        targetDev += ((deviceName == targetDevices.back()) ? "" : ",");
    }
    std::shared_ptr<ov::ICompiledModel> exeNetwork;
    config.insert({{CONFIG_KEY(PERFORMANCE_HINT), InferenceEngine::PluginConfigParams::CUMULATIVE_THROUGHPUT}});
    config.insert(ov::device::priorities(targetDev));
    ON_CALL(*core,
            compile_model(::testing::Matcher<const std::shared_ptr<const ov::Model>&>(_),
                        ::testing::Matcher<const std::string&>(StrEq(loadFailedDevice)),
                        ::testing::Matcher<const ov::AnyMap&>(_)))
        .WillByDefault(Throw(InferenceEngine::GeneralError{""}));
    if (loadFailedDevice != CommonTestUtils::DEVICE_CPU) {
        EXPECT_CALL(*core,
                    compile_model(::testing::Matcher<const std::shared_ptr<const ov::Model>&>(_),
                                ::testing::Matcher<const std::string&>(CommonTestUtils::DEVICE_CPU),
                                ::testing::Matcher<const ov::AnyMap&>(_)))
            .Times(1);
    }
    if (loadFailedDevice != CommonTestUtils::DEVICE_GPU) {
        EXPECT_CALL(*core,
                    compile_model(::testing::Matcher<const std::shared_ptr<const ov::Model>&>(_),
                                ::testing::Matcher<const std::string&>(CommonTestUtils::DEVICE_GPU),
                                ::testing::Matcher<const ov::AnyMap&>(_)))
            .Times(1);
    }
    ASSERT_NO_THROW(exeNetwork = plugin->compile_model(model, config));
    EXPECT_EQ(exeNetwork->get_property(ov::execution_devices.name()).as<std::string>(), secondDevice);
}

const std::vector<ConfigParams_1> testConfigs_1 = {
    ConfigParams_1{true, {"CPU", "GPU"}},
    ConfigParams_1{true, {"GPU", "CPU"}},
};

INSTANTIATE_TEST_SUITE_P(smoke_AutoCTPUTCallMulti,
                         AutoCTPUTCallMulti,
                         ::testing::ValuesIn(testConfigs_1),
                         AutoCTPUTCallMulti::getTestCaseName);
