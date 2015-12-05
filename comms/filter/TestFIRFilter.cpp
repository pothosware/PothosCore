// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <cmath> //fabs
#include <iostream>

static double filterToneGetRMS(
    const Pothos::DType &dtype,
    const double amplitude,
    const double sampRate,
    const double waveFreq,
    const size_t decim,
    const size_t interp
)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    auto waveSource = registry.callProxy("/comms/waveform_source", dtype);
    waveSource.callVoid("setAmplitude", amplitude);
    waveSource.callVoid("setWaveform", "SINE");
    waveSource.callVoid("setFrequency", waveFreq);
    waveSource.callVoid("setSampleRate", sampRate);

    auto finiteRelease = registry.callProxy("/blocks/finite_release");
    finiteRelease.callVoid("setTotalElements", 4096);

    auto filter = registry.callProxy("/comms/fir_filter", dtype, "COMPLEX");
    filter.callVoid("setDecimation", decim);
    filter.callVoid("setInterpolation", interp);
    filter.callVoid("setWaitTaps", true);

    auto designer = registry.callProxy("/comms/fir_designer");
    designer.callVoid("setSampleRate", (sampRate*interp)/decim);
    designer.callVoid("setFilterType", "SINC");
    designer.callVoid("setBandType", "COMPLEX_BAND_PASS");
    designer.callVoid("setFrequencyLower", waveFreq-0.1*sampRate);
    designer.callVoid("setFrequencyUpper", waveFreq+0.1*sampRate);
    designer.callVoid("setBandwidthTrans", waveFreq+0.1*sampRate);
    designer.callVoid("setNumTaps", 101);

    auto probe = registry.callProxy("/comms/signal_probe", dtype);
    probe.callVoid("setMode", "RMS");

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(designer, "tapsChanged", filter, "setTaps");
        topology.connect(waveSource, 0, finiteRelease, 0);
        topology.connect(finiteRelease, 0, filter, 0);
        topology.connect(filter, 0, probe, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    return probe.call<double>("value");
}

POTHOS_TEST_BLOCK("/comms/tests", test_fir_filter)
{
    std::vector<Pothos::DType> types;
    types.push_back(Pothos::DType("complex_float64"));
    types.push_back(Pothos::DType("complex_int16"));

    for (size_t i = 0; i < types.size(); i++)
    {
        std::cout << "Testing FIR filter on data type " << types[i].toString() << std::endl;
        for (size_t decim = 1; decim <= 3; decim++)
        {
            for (size_t interp = 1; interp <= 3; interp++)
            {
                const double amplitude = 1000;
                const double rate = 1e6;
                const double freq = 30e3;
                std::cout << "freq " << freq << " decim " << decim << " interp " << interp << std::flush;
                auto rms = filterToneGetRMS(types[i], amplitude, rate, freq, decim, interp);
                std::cout << " RMS = " << rms << std::endl;
                POTHOS_TEST_TRUE(rms > (0.1*amplitude));
            }
        }
    }
}
