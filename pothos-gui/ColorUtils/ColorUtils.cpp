// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "ColorUtils/ColorUtils.hpp"
#include <Poco/SingletonHolder.h>
#include <Poco/MD5Engine.h>
#include <Poco/RWLock.h>
#include <Poco/Types.h>
#include <Pothos/Framework.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <QPixmap>
#include <type_traits>
#include <complex>

/***********************************************************************
 * color map helper utilities
 **********************************************************************/
static QColor pastelize(const QColor &c)
{
    //Pastels have high value and low to intermediate saturation:
    //http://en.wikipedia.org/wiki/Pastel_%28color%29
    return QColor::fromHsv(c.hue(), int(c.saturationF()*128), int(c.valueF()*64)+191);
}

static QColor __typeStrToColor(const std::string &typeStr)
{
    //This first part does nothing more than create 3 random 8bit numbers
    //by mapping a chunk of a repeatable hash function to a color hex code.
    Poco::MD5Engine md5; md5.update(typeStr);
    const auto hexHash = Poco::DigestEngine::digestToHex(md5.digest());
    QColor c(QString::fromStdString("#" + hexHash.substr(0, 6)));

    //Use the 3 random numbers to create a pastel color.
    return pastelize(c);
}

/***********************************************************************
 * color map cache structures
 **********************************************************************/
static Poco::RWLock &getLookupMutex(void)
{
    static Poco::SingletonHolder<Poco::RWLock> sh;
    return *sh.get();
}

struct ColorMap : std::map<std::string, QColor>
{
    ColorMap(void);

    template <typename Type>
    void registerDType(const QColor &color)
    {
        (*this)[Pothos::DType(typeid(Type)).toString()] = color;
    }

    template <typename Type>
    void registerIntType(const QColor &color)
    {
        this->registerDType<typename std::make_signed<Type>::type>(color);
        this->registerDType<std::complex<typename std::make_signed<Type>::type>>(color.darker());
        this->registerDType<typename std::make_unsigned<Type>::type>(color);
        this->registerDType<std::complex<typename std::make_unsigned<Type>::type>>(color.darker());
    }

    template <typename Type>
    void registerFloatType(const QColor &color)
    {
        this->registerDType<Type>(color);
        this->registerDType<std::complex<Type>>(color.darker());
    }

    template <typename Type>
    void registerRType(const QColor &color)
    {
        (*this)[Pothos::Util::typeInfoToString(typeid(Type))] = color;
    }
};

static ColorMap &getColorMap(void)
{
    static Poco::SingletonHolder<ColorMap> sh;
    return *sh.get();
}

/*!
 * Initialize color map with some predefined colors.
 * Useful colors: http://www.computerhope.com/htmcolor.htm
 */
ColorMap::ColorMap(void)
{
    //special types
    (*this)[Pothos::DType().name()] = Qt::gray;
    (*this)[Pothos::DType("custom").name()] = Qt::gray;

    //integer types
    registerIntType<char>(Qt::magenta);
    registerIntType<short>(Qt::yellow);
    registerIntType<int>(Qt::green);
    static const QColor orange("#FF7F00");
    #ifndef POCO_LONG_IS_64_BIT
    registerIntType<long>(Qt::green);
    #else
    registerIntType<long>(orange);
    #endif
    registerIntType<long long>(orange);

    //floating point
    registerFloatType<float>(Qt::red);
    static const QColor skyBlue("#6698FF");
    registerFloatType<double>(skyBlue);

    //strings
    static const QColor cornYellow("#FFF380");
    registerRType<std::string>(cornYellow);
    registerRType<std::wstring>(cornYellow);
    registerRType<QString>(cornYellow);

    //boolean
    static const QColor tiffBlue("#81D8D0");
    registerRType<bool>(tiffBlue);

    //finalize with the pastelize
    for (auto &pair : *this) pair.second = pastelize(pair.second);
}

/***********************************************************************
 * color utils functions
 **********************************************************************/
QColor typeStrToColor(const std::string &typeStr_)
{
    auto typeStr = typeStr_;
    if (typeStr.empty()) return "white"; //not specified

    //try to pass it through DType to get a "nice" name
    try { typeStr = Pothos::DType(typeStr).toString(); }
    catch (const Pothos::DTypeUnknownError &){}

    //check the cache
    {
        Poco::RWLock::ScopedReadLock lock(getLookupMutex());
        auto it = getColorMap().find(typeStr);
        if (it != getColorMap().end()) return it->second;
    }

    //create a new entry
    Poco::RWLock::ScopedWriteLock lock(getLookupMutex());
    return (getColorMap()[typeStr] = __typeStrToColor(typeStr));
}

std::map<std::string, QColor> getTypeStrToColorMap(void)
{
    Poco::RWLock::ScopedReadLock lock(getLookupMutex());
    return getColorMap();
}

QIcon colorToWidgetIcon(const QColor &color)
{
    QPixmap pixmap(10, 10);
    pixmap.fill(color);
    return QIcon(pixmap);
}
