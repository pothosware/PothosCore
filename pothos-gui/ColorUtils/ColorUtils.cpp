// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "ColorUtils/ColorUtils.hpp"
#include <Poco/SingletonHolder.h>
#include <Poco/MD5Engine.h>
#include <Poco/RWLock.h>

static Poco::RWLock &getLookupMutex(void)
{
    static Poco::SingletonHolder<Poco::RWLock> sh;
    return *sh.get();
}

static std::map<std::string, QColor> &getColorMap(void)
{
    static Poco::SingletonHolder<std::map<std::string, QColor>> sh;
    return *sh.get();
}

static QColor __typeStrToColor(const std::string &typeStr)
{
    //This first part does nothing more than create 3 random 8bit numbers
    //by mapping a chunk of a repeatable hash function to a color hex code.
    Poco::MD5Engine md5; md5.update(typeStr);
    const auto hexHash = Poco::DigestEngine::digestToHex(md5.digest());
    QColor c(QString::fromStdString("#" + hexHash.substr(0, 6)));

    //Use the 3 random numbers to create a pastel color.
    //Pastels have high value and low to intermediate saturation:
    //http://en.wikipedia.org/wiki/Pastel_%28color%29
    return QColor::fromHsv(c.hue(), int(c.saturationF()*128), int(c.valueF()*64)+191);
}

QColor typeStrToColor(const std::string &typeStr)
{
    {
        Poco::RWLock::ScopedReadLock lock(getLookupMutex());
        auto it = getColorMap().find(typeStr);
        if (it != getColorMap().end()) return it->second;
    }
    Poco::RWLock::ScopedWriteLock lock(getLookupMutex());
    return (getColorMap()[typeStr] = __typeStrToColor(typeStr));
}

std::map<std::string, QColor> getTypeStrToColorMap(void)
{
    Poco::RWLock::ScopedReadLock lock(getLookupMutex());
    return getColorMap();
}
