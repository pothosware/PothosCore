// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <cmath> //fabs
#include <complex>
#include <iostream>
#include <algorithm> //max_element

/***********************************************************************
 * Convert an FFT into a power spectrum
 **********************************************************************/
template <typename InType>
const std::vector<double> fftPowerSpectrum(const std::complex<InType> *fftBins, const size_t fftSize)
{
    //power calculation
    std::vector<double> powerBins(fftSize);
    for (size_t i = 0; i < fftSize; i++)
    {
        powerBins[i] = 10*std::log10(std::norm(fftBins[i])) - 20*std::log10(fftSize);
    }

    //bin reorder
    for (size_t i = 0; i < powerBins.size()/2; i++)
    {
        std::swap(powerBins[i], powerBins[i+fftSize/2]);
    }

    return powerBins;
}

/***********************************************************************
 * ASCII art print of the power spectrum
 **********************************************************************/
static void printFFTPowerSpectrum(
    const std::vector<double> &powerBins,
    const double refLevel = 10.0,
    const double dynRange = 200.0,
    const size_t numCols = 70,
    const size_t numRows = 20
)
{
    std::vector<double> powerBinsColMax(numCols);
    std::vector<double> powerBinsColMin(numCols);
    for (size_t col = 0; col < numCols; col++)
    {
        const size_t hi = size_t(double(powerBins.size()*(col+1))/(numCols));
        const size_t lo = size_t(double(powerBins.size()*col)/(numCols));
        powerBinsColMax[col] = *std::max_element(&powerBins[lo], &powerBins[hi]);
        powerBinsColMin[col] = *std::min_element(&powerBins[lo], &powerBins[hi]);
    }

    for (size_t row = 0; row < numRows; row++)
    {
        const double lo = refLevel - (dynRange*(row+1))/(numRows);
        const double hi = refLevel - (dynRange*row)/(numRows);
        //std::cout << "hi " << hi << " lo " << lo << std::endl;
        for (size_t col = 0; col < numCols; col++)
        {
            const auto lvlMax = powerBinsColMax[col];
            const auto lvlMin = powerBinsColMin[col];
            if (
                (hi > lvlMax and lo <= lvlMax) or
                (hi > lvlMin and lo <= lvlMin) or
                (lvlMax > hi and lvlMin <= hi) or
                (lvlMax > lo and lvlMin <= lo)
            )
            {
                //low level resides in middle
                if (lvlMin > lo and lvlMin < hi)
                {
                    std::cout << "'";
                }
                //high level resides in middle
                else if (lvlMax > lo and lvlMax < hi)
                {
                    std::cout << ".";
                }
                else std::cout << "|";
            }
            else std::cout << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

/***********************************************************************
 * Test helpers
 **********************************************************************/
static bool testPassPoint(
    const std::vector<double> &powerBins,
    const double sampRate,
    const double freq
)
{
    const double minPassLevel = -30.0; //arbitrary, could be calculated from powerBins
    const size_t index = size_t(powerBins.size()*((freq + sampRate/2)/sampRate));
    std::cout << "Check pass level at " << freq/1e3 << " kHz, bin " << index << "...\t";
    if (powerBins[index] > minPassLevel)
    {
        std::cout << "OK" << std::endl;
        return true;
    }
    else
    {
        std::cout << "FAIL!" << std::endl;
        std::cout << "  Actual level = " << powerBins[index] << " dB" << std::endl;
        return false;
    }
}

static bool testStopPoint(
    const std::vector<double> &powerBins,
    const double sampRate,
    const double freq
)
{
    const double maxRejectLevel = -50.0; //arbitrary, could be calculated from powerBins
    const size_t index = size_t(powerBins.size()*((freq + sampRate/2)/sampRate));
    std::cout << "Check stop level at " << freq/1e3 << " kHz, bin " << index << "...\t";
    if (powerBins[index] < maxRejectLevel)
    {
        std::cout << "OK" << std::endl;
        return true;
    }
    else
    {
        std::cout << "FAIL!" << std::endl;
        std::cout << "  Actual level = " << powerBins[index] << " dB" << std::endl;
        return false;
    }
}

/***********************************************************************
 * Single test harness for a FIR configuration
 **********************************************************************/
static void testFIRDesignerResponse(
    const std::string &filterType,
    const std::string &bandType,
    const double sampRate,
    const double lowerFreq,
    const double upperFreq,
    const double fftSize)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    const auto dtype = Pothos::DType(typeid(std::complex<double>));

    //generate impulse with unit power (1.0)
    std::vector<double> impulse(fftSize, 0.0);
    impulse[fftSize-1] = double(fftSize);

    auto source = registry.callProxy("/blocks/vector_source", dtype);
    source.callVoid("setMode", "ONCE");
    source.callVoid("setElements", impulse);
    source.callVoid("setStartId", "START");

    auto filter = registry.callProxy("/comms/fir_filter", dtype, "COMPLEX");
    filter.callVoid("setDecimation", 1);
    filter.callVoid("setInterpolation", 1);
    filter.callVoid("setWaitTaps", true);
    filter.callVoid("setFrameStartId", "START");

    auto designer = registry.callProxy("/comms/fir_designer");
    designer.callVoid("setSampleRate", sampRate);
    designer.callVoid("setFilterType", filterType);
    designer.callVoid("setBandType", bandType);
    designer.callVoid("setFrequencyLower", lowerFreq);
    designer.callVoid("setFrequencyUpper", upperFreq);
    designer.callVoid("setNumTaps", 100);

    auto fft = registry.callProxy("/comms/fft", dtype, fftSize, false);
    auto collector = registry.callProxy("/blocks/collector_sink", dtype);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(designer, "tapsChanged", filter, "setTaps");
        topology.connect(source, 0, filter, 0);
        topology.connect(filter, 0, fft, 0);
        topology.connect(fft, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check the buffer
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.elements(), fftSize);

    //calculate power bins
    auto fftBins = buff.as<const std::complex<double> *>();
    const auto powerBins = fftPowerSpectrum(fftBins, fftSize);
    printFFTPowerSpectrum(powerBins);
    std::cout << filterType << "::" << bandType
        << "(rate=" << sampRate << ", low=" << lowerFreq << ", high=" << upperFreq << ")" << std::endl;

    //check based on band type
    if (bandType == "LOW_PASS")
    {
        POTHOS_TEST_TRUE(testPassPoint(powerBins, sampRate, 0.0)); //middle of pass band
        POTHOS_TEST_TRUE(testStopPoint(powerBins, sampRate, +(lowerFreq + sampRate/2)/2)); //middle of upper stop
        POTHOS_TEST_TRUE(testStopPoint(powerBins, sampRate, -(lowerFreq + sampRate/2)/2)); //middle of lower stop
    }
    if (bandType == "HIGH_PASS")
    {
        POTHOS_TEST_TRUE(testStopPoint(powerBins, sampRate, 0.0)); //middle of stop band
        POTHOS_TEST_TRUE(testPassPoint(powerBins, sampRate, +(lowerFreq + sampRate/2)/2)); //middle of upper pass
        POTHOS_TEST_TRUE(testPassPoint(powerBins, sampRate, -(lowerFreq + sampRate/2)/2)); //middle of lower pass
    }
    //TODO other types here...
    std::cout << std::endl;
}

/***********************************************************************
 * Loop through available filter and band types
 **********************************************************************/
POTHOS_TEST_BLOCK("/comms/tests", test_fir_designer)
{
    const double sampRate = 1e6;
    const size_t fftSize = 1024;

    std::vector<std::string> filterTypes;
    filterTypes.push_back("SINC");
    //TODO other types here....

    std::vector<std::string> bandTypes;
    bandTypes.push_back("LOW_PASS");
    bandTypes.push_back("HIGH_PASS");
    /*
    bandTypes.push_back("BAND_PASS");
    bandTypes.push_back("BAND_STOP");
    bandTypes.push_back("COMPLEX_BAND_PASS");
    bandTypes.push_back("COMPLEX_BAND_STOP");
    */

    //run through the test matrix
    for (const auto &filterType : filterTypes)
    {
        for (const auto &bandType : bandTypes)
        {
            testFIRDesignerResponse(filterType, bandType, sampRate, 2e5, 3e5, fftSize);
        }
    }
}
