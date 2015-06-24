// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstring> //memcpy
#include <iostream>

/***********************************************************************
 * |PothosDoc Preamble Framer
 *
 * The Preamble Framer parses a symbol stream on input port 0
 * for a special label indicating the start of a new frame.
 * The preamble is inserted into the stream before the label,
 * and the rest of the stream is forwarded to output port 0.
 *
 * All labels with the same index as the frame start label
 * (and including the frame start label itself),
 * will be shifted to the start of the preamble buffer.
 * All other labels propagate with the same position.
 *
 * This block supports operations on arbitrary symbol widths,
 * and therefore it may be used operationally on a bit-stream,
 * because a bit-stream is identically a symbol stream of N=1.
 *
 * |category /Digital
 * |keywords bit symbol preamble frame
 *
 * |param preamble A vector of symbols representing the preamble.
 * The width of each preamble symbol must the intended input stream.
 * |default [1]
 *
 * |param frameStartId[Frame Start ID] The label ID that marks the first symbol of frame data.
 * |default "frameStart"
 * |widget StringEntry()
 *
 * |factory /blocks/preamble_framer()
 * |setter setPreamble(preamble)
 * |setter setFrameStartId(frameStartId)
 **********************************************************************/
class PreambleFramer : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new PreambleFramer();
    }

    PreambleFramer(void)
    {
        this->setupInput(0, typeid(unsigned char));
        this->setupOutput(0, typeid(unsigned char), this->uid()); //unique domain because of buffer forwarding
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleFramer, setPreamble));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleFramer, getPreamble));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleFramer, setFrameStartId));
        this->registerCall(this, POTHOS_FCN_TUPLE(PreambleFramer, getFrameStartId));
        this->setPreamble(std::vector<unsigned char>(1, 1)); //initial update
        this->setFrameStartId("frameStart"); //initial update
    }

    void setPreamble(const std::vector<unsigned char> preamble)
    {
        if (preamble.empty()) throw Pothos::InvalidArgumentException("PreambleFramer::setPreamble()", "preamble cannot be empty");
        _preamble = preamble;
        _preambleBuff = Pothos::BufferChunk(_preamble.size());
        std::memcpy(_preambleBuff.as<unsigned char *>(), _preamble.data(), _preamble.size());
    }

    std::vector<unsigned char> getPreamble(void) const
    {
        return _preamble;
    }

    void setFrameStartId(std::string id)
    {
        _frameStartId = id;
    }

    std::string getFrameStartId(void) const
    {
        return _frameStartId;
    }

    void work(void)
    {
        auto inputPort = this->input(0);
        auto outputPort = this->output(0);

        //get input buffer
        auto inBuff = inputPort->buffer();
        if (inBuff.length == 0) return;

        //label propagation offset incremented as preambles are posted
        size_t labelIndexOffset = 0;

        //track the index of the last found frame start label
        int lastFoundIndex = -1;

        for (auto &label : inputPort->labels())
        {
            // Skip any label that doesn't yet appear in the data buffer
            if (label.index >= inputPort->elements()) continue;

            if (label.id == _frameStartId)
            {
                lastFoundIndex = label.index;

                //post everything before this label and exit
                Pothos::BufferChunk headBuff = inBuff;
                headBuff.length = label.index;
                if (headBuff.length != 0) outputPort->postBuffer(headBuff);

                //post the preamble
                outputPort->postBuffer(_preambleBuff);

                //remove header from the remaining buffer
                inBuff.length -= headBuff.length;
                inBuff.address += headBuff.length;
            }

            //increment the offset as soon as we are past the last found index
            if (lastFoundIndex != -1 and size_t(lastFoundIndex) != label.index)
            {
                lastFoundIndex = -1;
                labelIndexOffset += _preamble.size();
            }

            //propagate labels here with the offset
            Pothos::Label newLabel(label);
            newLabel.index += labelIndexOffset;
            outputPort->postLabel(newLabel);
        }

        //post the remaining bytes
        if (inBuff.length != 0) outputPort->postBuffer(inBuff);

        //consume the entire buffer
        inputPort->consume(inputPort->elements());
    }

    void propagateLabels(const Pothos::InputPort *)
    {
        //don't propagate here, its done in work()
    }

private:
    std::string _frameStartId;
    std::vector<unsigned char> _preamble;
    Pothos::BufferChunk _preambleBuff;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerPreambleFramer(
    "/blocks/preamble_framer", &PreambleFramer::make);
