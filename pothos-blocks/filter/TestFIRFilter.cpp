// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <cmath> //fabs
#include <iostream>

static double filterToneGetRMS(
    const double sampRate,
    const double waveFreq,
    const size_t decim,
    const size_t interp
)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    auto waveSource = registry.callProxy("/blocks/waveform_source", "complex128");
    waveSource.callVoid("setWaveform", "SINE");
    waveSource.callVoid("setFrequency", waveFreq/sampRate);

    auto finiteRelease = registry.callProxy("/blocks/finite_release", "complex128");
    finiteRelease.callVoid("setTotalElements", 4096);

    auto filter = registry.callProxy("/blocks/fir_filter", "complex128", "COMPLEX");
    filter.callVoid("setDecimation", decim);
    filter.callVoid("setInterpolation", interp);

    auto designer = registry.callProxy("/blocks/fir_designer");
    designer.callVoid("setSampleRate", (sampRate*interp)/decim);
    designer.callVoid("setFilterType", "COMPLEX_BAND_PASS");
    designer.callVoid("setFrequencyLower", waveFreq-0.1*sampRate);
    designer.callVoid("setFrequencyUpper", waveFreq+0.1*sampRate);
    designer.callVoid("setNumTaps", 100);

    auto probe = registry.callProxy("/blocks/stream_probe", "complex128");
    probe.callVoid("setMode", "RMS");

    //propagate the taps
    {
        Pothos::Topology topology;
        topology.connect(designer, "tapsChanged", filter, "setTaps");
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(waveSource, 0, finiteRelease, 0);
        topology.connect(finiteRelease, 0, filter, 0);
        topology.connect(filter, 0, probe, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    return probe.call<double>("value");
}

POTHOS_TEST_BLOCK("/blocks/tests", test_fir_filter)
{

    for (size_t decim = 1; decim <= 3; decim++)
    {
        for (size_t interp = 1; interp <= 3; interp++)
        {
            const double rate = 1e6;
            const double freq = 30e3;
            std::cout << "freq " << freq << " decim " << decim << " interp " << interp << std::flush;
            auto rms = filterToneGetRMS(rate, freq, decim, interp);
            std::cout << " RMS = " << rms << std::endl;
            POTHOS_TEST_TRUE(rms > 0.1);
        }
    }
}
