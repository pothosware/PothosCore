// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "AudioHelper.hpp"
#include <Pothos/Framework.hpp>
#include <Poco/Logger.h>
#include <portaudio.h>

/***********************************************************************
 * |PothosDoc Audio Sink
 *
 * The audio sink forwards an input sample stream into an audio output device.
 * In interleaved mode, the samples are interleaved from one input port,
 * In the port-per-channel mode, each audio channel uses a separate port.
 *
 * |category /Audio
 * |category /Sinks
 * |keywords audio sound stereo mono speaker
 *
 * |param deviceName[Device Name] The name of an audio device on the system,
 * the integer index of an audio device on the system,
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
 * |param numChans [Num Channels] The number of audio channels.
 * This parameter controls the number of samples per stream element.
 * |widget SpinBox(minimum=1)
 * |default 1
 *
 * |param chanMode [Channel Mode] The channel mode.
 * One port with interleaved channels or one port per channel?
 * |option [Interleaved channels] "INTERLEAVED"
 * |option [One port per channel] "PORTPERCHAN"
 * |default "INTERLEAVED"
 * |preview disable
 *
 * |factory /audio/sink(deviceName, sampRate, dtype, numChans, chanMode)
 **********************************************************************/
class AudioSink : public Pothos::Block
{
public:
    AudioSink(const std::string &deviceName, const double sampRate, const Pothos::DType &dtype, const size_t numChans, const std::string &chanMode):
        _stream(nullptr),
        _interleaved(chanMode == "INTERLEAVED")
    {
        PaError err = Pa_Initialize();
        if (err != paNoError)
        {
            throw Pothos::Exception("AudioSink()", "Pa_Initialize: " + std::string(Pa_GetErrorText(err)));
        }

        //determine which device
        const auto deviceIndex = getDeviceMatch("AudioSink", deviceName, Pa_GetDefaultOutputDevice());
        const auto deviceInfo = Pa_GetDeviceInfo(deviceIndex);
        poco_information_f2(Poco::Logger::get("AudioSink"), "Using %s through %s",
            std::string(deviceInfo->name), std::string(Pa_GetHostApiInfo(deviceInfo->hostApi)->name));

        //stream params
        PaStreamParameters streamParams;
        streamParams.device = deviceIndex;
        streamParams.channelCount = numChans;
        if (dtype == Pothos::DType("float32")) streamParams.sampleFormat = paFloat32;
        if (dtype == Pothos::DType("int32")) streamParams.sampleFormat = paInt32;
        if (dtype == Pothos::DType("int16")) streamParams.sampleFormat = paInt16;
        if (dtype == Pothos::DType("int8")) streamParams.sampleFormat = paInt8;
        if (dtype == Pothos::DType("uint8")) streamParams.sampleFormat = paUInt8;
        if (not _interleaved) streamParams.sampleFormat |= paNonInterleaved;
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
        if (Pa_GetSampleSize(streamParams.sampleFormat) != int(dtype.size()))
        {
            throw Pothos::Exception("AudioSink()", "Pa_GetSampleSize mismatch");
        }

        //setup ports
        if (_interleaved) this->setupInput(0, Pothos::DType(dtype.name(), numChans));
        else for (size_t i = 0; i < numChans; i++) this->setupInput(i, dtype);
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

    static Block *make(const std::string &deviceName, const double sampRate, const Pothos::DType &dtype, const size_t numChans, const std::string &chanMode)
    {
        return new AudioSink(deviceName, sampRate, dtype, numChans, chanMode);
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
        //calculate the number of frames
        int numFrames = Pa_GetStreamWriteAvailable(_stream);
        if (numFrames < 0)
        {
            throw Pothos::Exception("AudioSink.work()", "Pa_GetStreamWriteAvailable: " + std::string(Pa_GetErrorText(numFrames)));
        }
        numFrames = std::min<int>(numFrames, this->workInfo().minInElements);

        //handle do-nothing case with minimal sleep
        if (numFrames == 0)
        {
            Pa_Sleep(this->workInfo().maxTimeoutNs/1000000);
            return this->yield();
        }

        //get the buffer
        const void *buffer = nullptr;
        if (_interleaved) buffer = this->workInfo().inputPointers[0];
        else buffer = (const void *)this->workInfo().inputPointers.data();

        //peform write to the device
        PaError err = Pa_WriteStream(_stream, buffer, numFrames);
        if (err != paNoError)
        {
            poco_error(Poco::Logger::get("AudioSink"), "Pa_WriteStream: " + std::string(Pa_GetErrorText(err)));
        }

        //consume buffer (all modes)
        for (auto port : this->inputs()) port->consume(numFrames);
    }

private:
    PaStream *_stream;
    bool _interleaved;
};

static Pothos::BlockRegistry registerAudioSink(
    "/audio/sink", &AudioSink::make);
