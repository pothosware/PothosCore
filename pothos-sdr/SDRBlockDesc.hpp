// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

/***********************************************************************
 * |PothosDoc @TITLE@
 *
 * The SDR source block configures the receiver end of an SDR
 * and forwards baseband samples into 1 or more output streams.
 *
 * |category @CATEGORY@
 * |category /SDR
 *
 * |param deviceArgs[Device Args] Device construction arguments.
 * A map of key/value string pairs that identifies a SDR device.
 * |default {"driver":"null", "type":"null"}
 * |preview disable
 * |tab Streaming
 *
 * |param dtype[Data Type] The data type produced by the SDR source.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float32"
 * |preview disable
 * |tab Streaming
 *
 * |param channels[Channels] A list of available streaming channels.
 * Each element maps a port index on the block to a device channel.
 * |default [0]
 * |preview disable
 * |tab Streaming
 *
 * |param streamArgs[Stream Args] Additional stream arguments.
 * A map of key/value string pairs with implementation-specific meaning.
 * |default {}
 * |preview valid
 * |tab Streaming
 *
 * |param sampleRate[Sample Rate] The rate of sample stream on each channel.
 * |units Sps
 * |default 1e6
 * |tab Streaming
 *
 * |param frontendMap[Frontend map] Specify the mapping of stream channels to RF frontends.
 * The format of the mapping is implementation-specific.
 * |default ""
 * |preview valid
 * |widget StringEntry()
 * |tab Streaming
 *
 * |param frequency[Frequency] The center frequency of the chain.
 * Provide a single value for all channels or a list of values for each channel.
 * |units Hz
 * |default 0.0
 * |preview valid
 * |tab Channels
 *
 * |param tuneArgs[Tune Args] Advanced key/value tuning parameters.
 * |default {}
 * |preview valid
 * |tab Channels
 *
 * |param gainMode[Gain Mode] Gain selection mode of the chain.
 * The possible options are automatic mode or manual mode.
 * In manual mode the user-provided gain value will be used.
 * Provide a single value for all channels or a list of values for each channel.
 * |default false
 * |option [Automatic] true
 * |option [Manual] false
 * |preview valid
 * |tab Channels
 *
 * |param gain[Gain Value] The overall amplification of the chain.
 * The user may also provide a dictionary of name to gain values
 * where each name represents an amplification element in the chain.
 * Provide a single value for all channels or a list of values for each channel.
 * <ul>
 * <li>Example: {"BB" : 10.0, "RF" : "5.5"}</li>
 * <li>Example: [{"BB" : 10.0}, {"RF" : "5.5"}]</li>
 * </ul>
 * |units dB
 * |default 0.0
 * |preview valid
 * |tab Channels
 *
 * |param antenna[Antenna] The selected antenna for the RF frontend.
 * Provide a single value for all channels or a list of values for each channel.
 * |default ""
 * |tab Channels
 * |preview valid
 * |widget StringEntry()
 *
 * |param bandwidth[Bandwidth] The filter bandwidth setting.
 * Provide a single value for all channels or a list of values for each channel.
 * |units Hz
 * |default 0.0
 * |preview valid
 * |tab Channels
 * 
 * |param clockRate[Clock rate] The master reference clock rate.
 * |default 0.0
 * |units Hz
 * |preview valid
 * |tab Clocking
 *
 * |param clockSource[Clock Source] The source of the reference clock.
 * |default ""
 * |preview valid
 * |tab Clocking
 * |widget StringEntry()
 *
 * |param timeSource[Time Source] The source of the hardware time.
 * |default ""
 * |preview valid
 * |tab Clocking
 * |widget StringEntry()
 *
 * |factory @PATH@(dtype, channels)
 * |initializer setupDevice(deviceArgs)
 * |initializer setupStream(streamArgs)
 * |setter setSampleRate(sampleRate)
 * |setter setFrontendMap(frontendMap)
 * |setter setFrequency(frequency, tuneArgs)
 * |setter setGainMode(gainMode)
 * |setter setGain(gain)
 * |setter setAntenna(antenna)
 * |setter setBandwidth(bandwidth)
 * |setter setClockRate(clockRate)
 * |setter setClockSource(clockSource)
 * |setter setTimeSource(timeSource)
 **********************************************************************/
