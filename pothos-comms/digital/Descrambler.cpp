// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "lfsr.h"
#include <Pothos/Framework.hpp>
#include <iostream>
#include <cstring>
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Descrambler
 *
 * The descrambler block implements either an additive or a multiplicative
 * descrambler as defined in: http://en.wikipedia.org/wiki/Scrambler
 *
 * |category /Digital
 * |keywords descrambler
 * |alias /blocks/descrambler
 *
 * |param mode[Descrambler Mode]
 * |option [Additive] "additive"
 * |option [Multiplicative] "multiplicative"
 * |default "multiplicative"
 *
 * |param poly[Polynomial]
 * |default 0x19
 *
 * |param seed[Seed]
 * |default 0x1
 *
 * |factory /comms/descrambler()
 * |setter setPoly(poly)
 * |setter setMode(mode)
 * |setter setSeed(seed)
 **********************************************************************/
struct Descrambler : public Pothos::Block
{
    static Block *make(void)
    {
        return new Descrambler();
    }

    Descrambler(void):
        _polynom(1), _seed_value(1)
    {
        std::memset(&_lfsr, 0, sizeof(_lfsr));
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char));
        this->registerCall(this, POTHOS_FCN_TUPLE(Descrambler, setPoly));
        this->registerCall(this, POTHOS_FCN_TUPLE(Descrambler, poly));
        this->registerCall(this, POTHOS_FCN_TUPLE(Descrambler, setSeed));
        this->registerCall(this, POTHOS_FCN_TUPLE(Descrambler, seed));
        this->registerCall(this, POTHOS_FCN_TUPLE(Descrambler, setMode));
        this->registerCall(this, POTHOS_FCN_TUPLE(Descrambler, mode));
        this->registerCall(this, POTHOS_FCN_TUPLE(Descrambler, setSync));
        this->registerCall(this, POTHOS_FCN_TUPLE(Descrambler, sync));

        //some defaults
        this->setMode("multiplicative");
        this->setSync("");
        this->setPoly(0x19);
    }

    void setPoly(const int64_t &polynomial)
    {
        _polynom = polynomial;
        GLFSR_init(&_lfsr, _polynom, _seed_value);
    }

    int64_t poly(void) const
    {
        return _polynom;
    }

    void setSeed(const int64_t &seed)
    {
        _seed_value = seed;
        GLFSR_init(&_lfsr, _polynom, _seed_value);
    }

    int64_t seed(void) const
    {
        return _seed_value;
    }

    void setMode(const std::string &mode)
    {
        if (mode == "additive") _mode = MODE_ADD;
        else if (mode == "multiplicative") _mode = MODE_MULT;
        else throw Pothos::InvalidArgumentException("Descrambler::set_mode()", "unknown mode: " + mode);
    }

    std::string mode(void) const
    {
        if(_mode == MODE_ADD) return "additive";
        else return "multiplicative";
    }

    void setSync(const std::string &sync_word)
    {
        _sync_word = sync_word;
        if (_sync_word.size() > 64) throw Pothos::RangeException("Descrambler::set_sync()", "sync word max len 64 bits");

        _sync_bits.clear();
        for (size_t i = 0; i < _sync_word.size(); i++)
        {
            if (_sync_word[i] == '0')
            {
                _sync_bits.push_back(0);
            }
            else if (_sync_word[i] == '1')
            {
                _sync_bits.push_back(1);
            }
            else throw Pothos::RangeException("Descrambler::set_sync()", "sync word must be 0s and 1s: " + _sync_word);
        }
    }

    std::string sync(void) const
    {
        return _sync_word;
    }

    void work(void);
    unsigned char additive_bit_work(const unsigned char in);
    unsigned char multiplicative_bit_work(const unsigned char in);

    lfsr_t _lfsr;
    lfsr_data_t _polynom;
    lfsr_data_t _seed_value;
    enum {MODE_ADD, MODE_MULT} _mode;
    std::string _sync_word;
    std::vector<unsigned char> _sync_bits;
    long _count_down_to_sync_word;
};

unsigned char Descrambler::additive_bit_work(const unsigned char in)
{
    const unsigned char ret = GLFSR_next(&_lfsr);
    const unsigned char out = in ^ ret;
    return out;
}

unsigned char Descrambler::multiplicative_bit_work(const unsigned char in)
{
    const unsigned char ret = GLFSR_next(&_lfsr);
    const unsigned char out = in ^ ret;
    //input bit becomes the next bit0
    _lfsr.data &= ~lfsr_data_t(0x1);
    _lfsr.data |= in;
    return out;
}

void Descrambler::work(void)
{
    auto inPort = this->input(0);
    auto outPort = this->output(0);

    size_t n = std::min(inPort->elements(), outPort->elements());
    auto in = inPort->buffer().as<const unsigned char *>();
    auto out = outPort->buffer().as<unsigned char *>();

    //The main work loop deals with input bit by bit.
    if (_mode == MODE_ADD)
    {
        for (size_t i = 0; i < n; i++)
        {
            out[i] = this->additive_bit_work(in[i] & 0x1);
        }
    }
    if (_mode == MODE_MULT)
    {
        for (size_t i = 0; i < n; i++)
        {
            out[i] = this->multiplicative_bit_work(in[i] & 0x1);
        }
    }

    inPort->consume(n);
    outPort->produce(n);
}

static Pothos::BlockRegistry registerDescrambler(
    "/comms/descrambler", &Descrambler::make);

static Pothos::BlockRegistry registerDescramblerOldPath(
    "/blocks/descrambler", &Descrambler::make);
