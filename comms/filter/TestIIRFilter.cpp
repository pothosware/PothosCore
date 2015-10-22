#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <cmath> //fabs
#include <iostream>

static double iirfilterToneGetRMS(
																	const double sampRate,
																	const double waveFreq
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

    auto filter = registry.callProxy("/comms/iir_filter", "complex128");
		filter.callVoid("setWaitTaps", true);

    auto designer = registry.callProxy("/comms/iir_designer");
    designer.callVoid("setSampleRate", sampRate);
    designer.callVoid("setIIRType", "butterworth");
    designer.callVoid("setFilterType", "LOW_PASS");
    designer.callVoid("setFrequencyLower", 0.1*sampRate);
    designer.callVoid("setOrder", 4);

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

POTHOS_TEST_BLOCK("/comms/tests", test_iir_filter)
{

	const double rate = 1e6;
	const double freq = 30e3;
	std::cout << "freq " << freq << std::flush;
	auto rms = iirfilterToneGetRMS(rate, freq);
	std::cout << " RMS = " << rms << std::endl;
	POTHOS_TEST_TRUE(rms > 0.1);
}
