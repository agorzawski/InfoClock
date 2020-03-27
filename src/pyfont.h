#ifndef PYFONT_H
#define PYFONT_H

#include <stdint.h>
#include <stddef.h>

struct PyFont
{
    PyFont(uint8_t chars, uint8_t baseChar, const uint8_t* data, const uint16_t* offsets, const uint8_t* sizes):
        chars(chars), baseChar(baseChar), data(data), offsets(offsets), sizes(sizes) {}

    uint8_t chars;
    uint8_t baseChar;
    const uint8_t* data;
    const uint16_t* offsets;
    const uint8_t* sizes;

    uint8_t getCharSize(char ch) const
    {
        if ((ch < baseChar) || (ch > (chars + baseChar)))
            ch = baseChar;
    	uint16_t o = ((uint8_t)ch)-baseChar;
        return sizes[o];
    }

    const uint8_t* getCharData(char ch) const
    {
        if ((ch < baseChar) || (ch > (chars + baseChar)))
            ch = baseChar;
        uint16_t o = ((uint8_t)ch)-baseChar;
        return data + offsets[o];
    }
};


int renderText(const PyFont& f, const char* text, uint8_t* output, int maxSize);
size_t calculateRenderedLength(const PyFont& f, const char* text);

#endif //PYFONT_H
