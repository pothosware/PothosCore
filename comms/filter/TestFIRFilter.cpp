// Copyright (c) 2014-2015 Josh Blum
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

    auto waveSource = registry.callProxy("/comms/waveform_source", "complex128");
    waveSource.callVoid("setWaveform", "SINE");
    waveSource.callVoid("setFrequency", waveFreq);
    waveSource.callVoid("setSampleRate", sampRate);

    auto finiteRelease = registry.callProxy("/blocks/finite_release");
    finiteRelease.callVoid("setTotalElements", 4096);

    auto filter = registry.callProxy("/comms/fir_filter", "complex128", "COMPLEX");
    filter.callVoid("setDecimation", decim);
    filter.callVoid("setInterpolation", interp);
    filter.callVoid("setWaitTaps", true);

    auto designer = registry.callProxy("/comms/fir_designer");
    designer.callVoid("setSampleRate", (sampRate*interp)/decim);
    designer.callVoid("setFilterType", "SINC");
    designer.callVoid("setBandType", "COMPLEX_BAND_PASS");
    designer.callVoid("setFrequencyLower", waveFreq-0.1*sampRate);
    designer.callVoid("setFrequencyUpper", waveFreq+0.1*sampRate);
    designer.callVoid("setFrequencyTrans", waveFreq+0.1*sampRate);
    designer.callVoid("setNumTaps", 100);

    auto probe = registry.callProxy("/comms/signal_probe", "complex128");
    probe.callVoid("setMode", "RMS");

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(designer, "tapsChanged", filter, "setTaps");
        topology.connect(waveSource, 0, finiteRelease, 0);
        topology.connect(finiteRelease, 0, filter, 0);
        topology.connect(filter, 0, probe, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    return probe.call<double>("value");
}

POTHOS_TEST_BLOCK("/comms/tests", test_fir_filter)
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
