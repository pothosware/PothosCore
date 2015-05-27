// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

/***********************************************************************
 * |PothosDoc SDR @TITLE@
 *
 * The SDR @TITLE@ block configures the @XCVR@ end of an SDR
 * and interfaces baseband samples to 1 or more data ports.
 *
 * <h3>Runtime configuration</h3>
 * The SDR @TITLE@ block has a number of setter methods or slots
 * which can be used to make configuration calls from an external block:
 * <ul>
 * <li>setSampleRate(rate)</li>
 * <li>setFrontendMap(mapping)</li>
 * <li>setFrequency(chan, freq)</li>
 * <li>setFrequency(chan, freq, tuneArgs)</li>
 * <li>setGainMode(chan, automatic)</li>
 * <li>setGain(chan, gain)</li>
 * <li>setGain(chan, name, gain)</li>
 * <li>setGain(chan, gainDict)</li>
 * <li>setAntenna(chan, antenna)</li>
 * <li>setBandwidth(chan, bw)</li>
 * <li>setDCOffsetMode(chan, automatic)</li>
 * <li>setClockRate(rate)</li>
 * <li>setClockSource(source)</li>
 * <li>setTimeSource(source)</li>
 * <li>setHardwareTime(timeNs)</li>
 * <li>setCommandTime(timeNs)</li>
 * </ul>
 *
 * All calls which have a channel parameter also have the following call variants.
 * Consider setFoo(3, val):
 * <ul>
 * <li>setFoo(3, val) sets val on channel 3</li>
 * <li>setFoo3(val) also sets on for channel 3</li>
 * <li>setFoo(val) sets val on all channels</li>
 * <li>setFoo(valArray) sets valArray[i] on channel[i]</li>
 * </ul>
 *
 * <h3>Advanced stream control</h3>
 * By default, the block begins streaming upon activation.
 * To disable this behavior, modify the auto activate property.
 *
 * The user can access the streaming control directly
 * for advanced usage through the streamControl() slot.
 * The streamControl() method has the following variants:
 * <ul>
 * <li>streamControl("ACTIVATE") - stream continuously immediately</li>
 * <li>streamControl("ACTIVATE_AT", timeNs) - stream continuously starting at timeNs</li>
 * <li>streamControl("ACTIVATE_BURST", 0, numElems) - stream a burst of size numElems immediately</li>
 * <li>streamControl("ACTIVATE_BURST_AT", timeNs, numElems) - stream a burst of size numElems at timeNs</li>
 * <li>streamControl("DEACTIVATE") - halt a continuous stream</li>
 * <li>streamControl("DEACTIVATE_AT", timeNs) - halt a continuous stream at timeNs</li>
 * </ul>
 *
 * <h3>Stream metadata</h3>
 * The SDR source block uses labels to associate metadata and events with the output streams.
 * The SDR source block produces the following labels:
 * <ul>
 * <li>rxTime - the hardware time in ns of the associated stream element</li>
 * <li>rxEnd - a burst has ended with the associated stream element</li>
 * <li>rxFreq - the center frequency of the chain from the last tune operation</li>
 * <li>rxRate - the sample rate of the channel the last call to setSampleRate()</li>
 * </ul>
 *
 * The SDR sink block uses labels to control transmission time and bursts with the input stream.
 * The SDR sink is responsive to the following labels:
 * <ul>
 * <li>txTime - the hardware transmit time for the associated stream element</li>
 * <li>txEnd - the associated stream element signifies the end of a burst</li>
 * </ul>
 *
 * |category @CATEGORY@
 * |category /SDR
 *
 * |param deviceArgs[Device Args] Device construction arguments.
 * A map of key/value string pairs that identifies a SDR device.
 * |default {"driver":"null"}
 * |preview valid
 * |tab Streaming
 *
 * |param dtype[Data Type] The data type used by the stream ports.
 * The type is homogenous across all streaming channels.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float32"
 * |preview disable
 * |tab Streaming
 *
 * |param channels[Channels] A list of available streaming channels.
 * Each element maps a port index on the block to a device channel.
 * When channels is empty, the block automatically selects channel 0.
 * |default []
 * |preview valid
 * |tab Streaming
 *
 * |param streamArgs[Stream Args] Additional stream arguments.
 * A map of key/value string pairs with implementation-specific meaning.
 * <ul>
 * <li>Example to set the wire format: {"WIRE" : "sc16"}</li>
 * </ul>
 * |default {}
 * |preview valid
 * |tab Streaming
 *
 * |param sampleRate[Sample Rate] The rate of sample stream on each channel.
 * |units Sps
 * |default 1e6
 * |tab Streaming
 *
 * |param autoActivate[Auto Activate] Automatic stream activation control.
 * The default action is to activate the stream when the block becomes active.
 * The user can disable automatic activation in favor of direct control
 * via the registered streamControl() slot.
 * |default true
 * |option [Automatic] true
 * |option [Manual] false
 * |tab Streaming
 * |preview disable
 *
 * |param frontendMap[Frontend map] Specify the mapping of stream channels to RF frontends.
 * The format of the mapping is implementation-specific.
 * |default ""
 * |preview valid
 * |widget StringEntry()
 * |tab Channels
 *
 * |param frequency[Frequency] The center frequency of the chain.
 * Provide a single value for all channels or a list of values for each channel.
 * |units Hz
 * |default 0.0
 * |preview valid
 * |tab Channels
 *
 * |param tuneArgs[Tune Args] Advanced key/value tuning parameters.
 * Well known arguments include "OFFSET":1e6 which specifies a RF frontend tuning offset,
 * which is automatically compensated for by adjusting the baseband tuning component;
 * "RF":"IGNORE" which avoids tuning the named tuning component;
 * "BB":0.0 which forces the named tuning component to a specified value.
 * <ul>
 * <li>Example: {"OFFSET" : 1e6}</li>
 * <li>Example: {"RF" : "IGNORE"}</li>
 * <li>Example: {"BB" : 0.0}</li>
 * </ul>
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
 * |widget ComboBox(editable=true)
 * |preview valid
 * |tab Channels
 *
 * |param gain[Gain Value] The overall amplification of the chain.
 * The user may also provide a dictionary of name to gain values
 * where each name represents an amplification element in the chain.
 * Provide a single value for all channels or a list of values for each channel.
 * <ul>
 * <li>Example: {"PGA" : 10.0, "LNA" : 5.5}</li>
 * <li>Example: [{"PGA" : 10.0}, {"PGA" : 20.0}]</li>
 * </ul>
 * |units dB
 * |default 0.0
 * |preview valid
 * |tab Channels
 *
 * |param antenna[Antenna] The selected antenna for the RF frontend.
 * Provide a single value for all channels or a list of values for each channel.
 * <ul>
 * <li>Example: "RX1"</li>
 * <li>Example: ["RX1", "RX3"]</li>
 * </ul>
 * |default ""
 * |tab Channels
 * |preview valid
 *
 * |param bandwidth[Bandwidth] The filter bandwidth setting.
 * Provide a single value for all channels or a list of values for each channel.
 * |units Hz
 * |default 0.0
 * |preview valid
 * |tab Channels
 *
 * |param dcOffsetMode[DC Offset Mode] Enable/disable automatic DC offset removal.
 * DC offset removal subtracts out the signal average. Not supported on all platforms.
 * |default true
 * |option [Enable] true
 * |option [Disable] false
 * |widget ComboBox(editable=true)
 * |preview disable
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
 * |initializer setFrontendMap(frontendMap)
 * |setter setSampleRate(sampleRate)
 * |setter setAutoActivate(autoActivate)
 * |setter setFrequency(frequency, tuneArgs)
 * |setter setGainMode(gainMode)
 * |setter setGain(gain)
 * |setter setAntenna(antenna)
 * |setter setBandwidth(bandwidth)
 * |setter setDCOffsetMode(dcOffsetMode)
 * |setter setClockRate(clockRate)
 * |setter setClockSource(clockSource)
 * |setter setTimeSource(timeSource)
 **********************************************************************/
