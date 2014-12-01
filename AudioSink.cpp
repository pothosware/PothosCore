// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/Logger.h>
#include <portaudio.h>

/***********************************************************************
 * |PothosDoc Audio Sink
 *
 * The audio sink forwards an input sample stream into an audio output device.
 *
 * |category /Audio
 * |category /Sinks
 * |keywords audio sound stereo mono
 *
 * |param deviceName[Device Name] The name of an audio device on the system,
 * or an empty string to use the default input device.
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
 * |param dtype[Data Type] The data type consumed by the audio sink.
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
 * |factory /audio/sink(deviceName, sampRate, dtype, numChan)
 **********************************************************************/
class AudioSink : public Pothos::Block
{
public:
    AudioSink(const std::string &deviceName, const double sampRate, const Pothos::DType &dtype, const size_t numChannels):
        _stream(nullptr)
    {
        PaError err = Pa_Initialize();
        if (err != paNoError)
        {
            throw Pothos::Exception("AudioSink()", "Pa_Initialize: " + std::string(Pa_GetErrorText(err)));
        }

        //determine which device
        PaDeviceIndex deviceIndex = Pa_GetDefaultOutputDevice();
        for (PaDeviceIndex i = 0; i < Pa_GetDeviceCount(); i++)
        {
            if (Pa_GetDeviceInfo(i)->name == deviceName) deviceIndex = i;
        }
        const auto deviceInfo = Pa_GetDeviceInfo(deviceIndex);
        poco_information_f1(Poco::Logger::get("AudioSink"), "Using %s", std::string(deviceInfo->name));

        //stream params
        PaStreamParameters streamParams;
        streamParams.device = deviceIndex;
        streamParams.channelCount = numChannels;
        if (dtype == Pothos::DType("float32")) streamParams.sampleFormat = paFloat32;
        if (dtype == Pothos::DType("int32")) streamParams.sampleFormat = paInt32;
        if (dtype == Pothos::DType("int16")) streamParams.sampleFormat = paInt16;
        if (dtype == Pothos::DType("int8")) streamParams.sampleFormat = paInt8;
        if (dtype == Pothos::DType("uint8")) streamParams.sampleFormat = paUInt8;
        streamParams.suggestedLatency = (deviceInfo->defaultLowOutputLatency + deviceInfo->defaultHighOutputLatency)/2;
        streamParams.hostApiSpecificStreamInfo = nullptr;

        //try stream
        err = Pa_IsFormatSupported(nullptr, &streamParams, sampRate);
        if (err != paNoError)
        {
            throw Pothos::Exception("AudioSink()", "Pa_IsFormatSupported: " + std::string(Pa_GetErrorText(err)));
        }

        //open stream
        err = Pa_OpenStream(
            &_stream, // stream
            nullptr, // inputParameters
            &streamParams, // outputParameters
            sampRate,  //sampleRate
            paFramesPerBufferUnspecified, // framesPerBuffer
            0, // streamFlags
            nullptr, //streamCallback
            nullptr); //userData
        if (err != paNoError)
        {
            throw Pothos::Exception("AudioSink()", "Pa_OpenStream: " + std::string(Pa_GetErrorText(err)));
        }

        this->setupInput(0, Pothos::DType(dtype.name(), numChannels));
    }

    ~AudioSink(void)
    {
        PaError err = Pa_CloseStream(_stream);
        if (err != paNoError)
        {
            poco_error_f1(Poco::Logger::get("AudioSink"), "Pa_CloseStream: %s", std::string(Pa_GetErrorText(err)));
        }

        err = Pa_Terminate();
        if (err != paNoError)
        {
            poco_error_f1(Poco::Logger::get("AudioSink"), "Pa_Terminate: %s", std::string(Pa_GetErrorText(err)));
        }
    }

    static Block *make(const std::string &deviceName, const double sampRate, const Pothos::DType &dtype, const size_t numChannels)
    {
        return new AudioSink(deviceName, sampRate, dtype, numChannels);
    }

    void activate(void)
    {
        PaError err = Pa_StartStream(_stream);
        if (err != paNoError)
        {
            throw Pothos::Exception("AudioSink.activate()", "Pa_StartStream: " + std::string(Pa_GetErrorText(err)));
        }
    }

    void deactivate(void)
    {
        PaError err = Pa_StopStream(_stream);
        if (err != paNoError)
        {
            throw Pothos::Exception("AudioSink.deactivate()", "Pa_StopStream: " + std::string(Pa_GetErrorText(err)));
        }
    }

    void work(void)
    {
        auto inPort = this->input(0);

        //calculate the number of frames
        int numFrames = Pa_GetStreamWriteAvailable(_stream);
        if (numFrames < 0)
        {
            throw Pothos::Exception("AudioSink.work()", "Pa_GetStreamWriteAvailable: " + std::string(Pa_GetErrorText(numFrames)));
        }
        numFrames = std::min<int>(numFrames, inPort->elements());

        //handle do-nothing case with minimal sleep
        if (numFrames == 0)
        {
            Pa_Sleep(this->workInfo().maxTimeoutNs/1000000);
            return this->yield();
        }

        //peform write to the device
        PaError err = Pa_WriteStream(_stream, inPort->buffer().as<const void *>(), numFrames);
        if (err != paNoError)
        {
            throw Pothos::Exception("AudioSink.work()", "Pa_WriteStream: " + std::string(Pa_GetErrorText(err)));
        }

        //consume buffer
        inPort->consume(numFrames);
    }

private:
    PaStream *_stream;
};

static Pothos::BlockRegistry registerAudioSink(
    "/audio/sink", &AudioSink::make);
