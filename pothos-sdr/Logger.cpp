// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <SoapySDR/Logger.hpp>
#include <Poco/Logger.h>

/***********************************************************************
 * Register Poco logging handler for SoapySDR
 **********************************************************************/
static void SoapyPocoLogHandler(const SoapySDR::LogLevel logLevel, const char *message)
{
    static auto &logger = Poco::Logger::get("SoapySDR");
    static_assert(Poco::Message::Priority(SOAPY_SDR_FATAL) == Poco::Message::PRIO_FATAL, "SoapySDR log levels match Poco");
    static_assert(Poco::Message::Priority(SOAPY_SDR_INFO) == Poco::Message::PRIO_INFORMATION, "SoapySDR log levels match Poco");
    static_assert(Poco::Message::Priority(SOAPY_SDR_TRACE) == Poco::Message::PRIO_TRACE, "SoapySDR log levels match Poco");
    logger.log(Poco::Message("SoapySDR", message, Poco::Message::Priority(logLevel)));
}

pothos_static_block(registerSoapySDRLogHandler)
{
    SoapySDR::registerLogHandler(&SoapyPocoLogHandler);
}
