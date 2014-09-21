// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>
#include <unistd.h>

static void test_fir_filter_args(
    const double waveFreq,
    const size_t decim,
    const size_t interp
)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    auto waveSource = registry.callProxy("/blocks/waveform_source", "complex128");
    waveSource.callVoid("setFrequency", waveFreq/1.0);

    auto filter = registry.callProxy("/blocks/fir_filter", "complex128", "COMPLEX");
    filter.callVoid("setDecimation", decim);
    filter.callVoid("setInterpolation", interp);

    auto designer = registry.callProxy("/blocks/fir_designer");
    designer.callVoid("setSampleRate", 1.0);
    designer.callVoid("setFilterType", "COMPLEX_BAND_PASS");
    designer.callVoid("setFrequencyLower", waveFreq-0.1);
    designer.callVoid("setFrequencyUpper", waveFreq+0.1);
    designer.callVoid("setNumTaps", 100);

    auto multiply = registry.callProxy("/blocks/arithmetic", "complex128", "MUL");
    auto downShift = registry.callProxy("/blocks/waveform_source", "complex128");
    waveSource.callVoid("setFrequency", -waveFreq/((1.0*interp)/decim));

    auto rmsProbe = registry.callProxy("/blocks/rms_probe", "complex128");

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(designer, "tapsChanged", filter, "setTaps");
        topology.connect(waveSource, 0, filter, 0);
        topology.connect(filter, 0, multiply, 0);
        topology.connect(downShift, 0, multiply, 1);
        topology.connect(multiply, 0, rmsProbe, 0);
        topology.commit();
        sleep(1); //TODO needs finite cutoff
        //POTHOS_TEST_TRUE(topology.waitInactive());
    }

    std::cout << "RMS: " << rmsProbe.call<double>("rms") << std::endl;
}

POTHOS_TEST_BLOCK("/blocks/tests", test_fir_filter)
{
    test_fir_filter_args(0.3, 1, 1);
}
