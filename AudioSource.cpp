// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/Logger.h>
#include <portaudio.h>

/***********************************************************************
 * |PothosDoc Audio Source
 *
 * The audio source forwards an audio input device to an output sample stream.
 *
 * The audio source will post a sample rate stream label named "rxRate"
 * on the first call to work() after activate() has been called.
 * Downstream blocks like the plotter widgets can consume this label
 * and use it to set internal parameters like the axis scaling.
 *
 * |category /Audio
 * |category /Sinks
 * |keywords audio sound stereo mono
 *
 * |param deviceName[Device Name] The name of an audio device on the system,
 * or an empty string to use the default output device.
 * |widget StringEntry()
 * |default ""
 * |preview valid
 *
 * |param sampRate[Sample Rate] The rate of audio samples.
 * |option 32e3
 * |option 44.1e3
 * |option 48e3
 * |default 44.1e3
 * |units Sps
 * |widget ComboBox(editable=true)
 *
 * |param dtype[Data Type] The data type produced by the audio source.
 * |option [Float32] "float32"
 * |option [Int32] "int32"
 * |option [Int16] "int16"
 * |option [Int8] "int8"
 * |option [UInt8] "uint8"
 * |default "float32"
 * |preview disable
 *
 * |param numChan [Num Channels] The number of audio channels.
 * This parameter controls the number of samples per stream element.
 * |widget SpinBox(minimum=1)
 * |default 1
 *
 * |factory /audio/source(deviceName, sampRate, dtype, numChan)
 **********************************************************************/
class AudioSource : public Pothos::Block
{
public:
    AudioSource(const std::string &deviceName, const double sampRate, const Pothos::DType &dtype, const size_t numChannels):
        _stream(nullptr),
        _sendLabel(false)
    {
        PaError err = Pa_Initialize();
        if (err != paNoError)
        {
            throw Pothos::Exception("AudioSource()", "Pa_Initialize: " + std::string(Pa_GetErrorText(err)));
        }

        //determine which device
        PaDeviceIndex deviceIndex = Pa_GetDefaultInputDevice();
        for (PaDeviceIndex i = 0; i < Pa_GetDeviceCount(); i++)
        {
            if (Pa_GetDeviceInfo(i)->name == deviceName) deviceIndex = i;
        }
        const auto deviceInfo = Pa_GetDeviceInfo(deviceIndex);
        poco_information_f1(Poco::Logger::get("AudioSource"), "Using %s", std::string(deviceInfo->name));

        //stream params
        PaStreamParameters streamParams;
        streamParams.device = deviceIndex;
        streamParams.channelCount = numChannels;
        if (dtype == Pothos::DType("float32")) streamParams.sampleFormat = paFloat32;
        if (dtype == Pothos::DType("int32")) streamParams.sampleFormat = paInt32;
        if (dtype == Pothos::DType("int16")) streamParams.sampleFormat = paInt16;
        if (dtype == Pothos::DType("int8")) streamParams.sampleFormat = paInt8;
        if (dtype == Pothos::DType("uint8")) streamParams.sampleFormat = paUInt8;
        streamParams.suggestedLatency = (deviceInfo->defaultLowInputLatency + deviceInfo->defaultHighInputLatency)/2;
        streamParams.hostApiSpecificStreamInfo = nullptr;

        //try stream
        err = Pa_IsFormatSupported(&streamParams, nullptr, sampRate);
        if (err != paNoError)
        {
            throw Pothos::Exception("AudioSource()", "Pa_IsFormatSupported: " + std::string(Pa_GetErrorText(err)));
        }

        //open stream
        err = Pa_OpenStream(
            &_stream, // stream
            &streamParams, // inputParameters
            nullptr, // outputParameters
            sampRate,  //sampleRate
            paFramesPerBufferUnspecified, // framesPerBuffer
            0, // streamFlags
            nullptr, //streamCallback
            nullptr); //userData
        if (err != paNoError)
        {
            throw Pothos::Exception("AudioSource()", "Pa_OpenStream: " + std::string(Pa_GetErrorText(err)));
        }

        this->setupOutput(0, Pothos::DType(dtype.name(), numChannels));
    }

    ~AudioSource(void)
    {
        PaError err = Pa_CloseStream(_stream);
        if (err != paNoError)
        {
            poco_error_f1(Poco::Logger::get("AudioSource"), "Pa_CloseStream: %s", std::string(Pa_GetErrorText(err)));
        }

        err = Pa_Terminate();
        if (err != paNoError)
        {
            poco_error_f1(Poco::Logger::get("AudioSource"), "Pa_Terminate: %s", std::string(Pa_GetErrorText(err)));
        }
    }

    static Block *make(const std::string &deviceName, const double sampRate, const Pothos::DType &dtype, const size_t numChannels)
    {
        return new AudioSource(deviceName, sampRate, dtype, numChannels);
    }

    void activate(void)
    {
        PaError err = Pa_StartStream(_stream);
        if (err != paNoError)
        {
            throw Pothos::Exception("AudioSource.activate()", "Pa_StartStream: " + std::string(Pa_GetErrorText(err)));
        }
        _sendLabel = true;
    }

    void deactivate(void)
    {
        PaError err = Pa_StopStream(_stream);
        if (err != paNoError)
        {
            throw Pothos::Exception("AudioSource.deactivate()", "Pa_StopStream: " + std::string(Pa_GetErrorText(err)));
        }
    }

    void work(void)
    {
        auto outPort = this->output(0);

        //calculate the number of frames
        int numFrames = Pa_GetStreamReadAvailable(_stream);
        if (numFrames < 0)
        {
            throw Pothos::Exception("AudioSource.work()", "Pa_GetStreamReadAvailable: " + std::string(Pa_GetErrorText(numFrames)));
        }
        numFrames = std::min<int>(numFrames, outPort->elements());

        //handle do-nothing case with minimal sleep
        if (numFrames == 0)
        {
            Pa_Sleep(this->workInfo().maxTimeoutNs/1000000);
            return this->yield();
        }

        //peform read from the device
        if (outPort->elements() == 0) return;
        PaError err = Pa_ReadStream(_stream, outPort->buffer().as<void *>(), numFrames);
        if (err != paNoError)
        {
            throw Pothos::Exception("AudioSource.work()", "Pa_ReadStream: " + std::string(Pa_GetErrorText(err)));
        }

        if (_sendLabel)
        {
            _sendLabel = false;
            const auto rate = Pa_GetStreamInfo(_stream)->sampleRate;
            outPort->postLabel(Pothos::Label("rxRate", rate, 0));
        }

        //produce buffer
        outPort->produce(numFrames);
    }

private:
    PaStream *_stream;
    bool _sendLabel;
};

static Pothos::BlockRegistry registerAudioSource(
    "/audio/source", &AudioSource::make);
