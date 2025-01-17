/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Render/2D/TextLayout.h"
#include "Utils/BiDiHelper.h"
#include "Systems/VirtualCoordinatesSystem.h"
#include "Utils/StringUtils.h"

namespace DAVA
{
TextLayout::TextLayout()
    : TextLayout(false)
{
}

TextLayout::TextLayout(const bool _useBiDi)
    : useBiDi(_useBiDi)
    , isRtl(false)
    , lineData()
{
}

TextLayout::~TextLayout()
{
}

void TextLayout::Reset(const WideString& _input)
{
    if (inputText != _input)
    {
        // Update prepared text and line breaks only if input text was changed
        inputText = _input;
        preparedText = _input;
        if (useBiDi)
        {
            bidiHelper.PrepareString(inputText, preparedText, &isRtl);
        }
        StringUtils::GetLineBreaks(preparedText, breaks);
        DVASSERT_MSG(breaks.size() == preparedText.length(), "Incorrect breaks information");
    }
    characterSizes.clear();
    Seek(0);
}

void TextLayout::Reset(const WideString& input, const Font& font)
{
    Reset(input);
    CalculateCharSizes(font);
}

void TextLayout::SetCharSizes(const Vector<float32>& charSizes)
{
    characterSizes = charSizes;
    PrepareCharSizes();
}

void TextLayout::CalculateCharSizes(const Font& font)
{
    // Update characters sizes from font
    characterSizes.clear();
    font.GetStringMetrics(preparedText, &characterSizes);
    PrepareCharSizes();
}

void TextLayout::PrepareCharSizes()
{
    DVASSERT(characterSizes.size() == preparedText.length());
    if (useBiDi)
    {
        size_t size = characterSizes.size();
        for (size_t i = 0; i < size; ++i)
        {
            if (bidiHelper.IsBiDiSpecialCharacter(preparedText[i]))
            {
                characterSizes[i] = 0.0f;
            }
        }
    }
}

void TextLayout::Seek(const uint32 _position)
{
    lineData.offset = _position;
    lineData.length = 0;
}

bool TextLayout::IsEndOfText()
{
    return lineData.offset + lineData.length >= preparedText.length();
}

bool TextLayout::NextByWords(const float32 lineWidth)
{
    DVASSERT(characterSizes.size() == preparedText.length());
    float32 targetWidth = std::floor(lineWidth);
    float32 currentWidth = 0;
    uint32 textLength = uint32(preparedText.length());
    uint32 lastPossibleBreak = 0;

    lineData.offset += lineData.length; // Move line cursor to next line
    lineData.length = 0;

    for (uint32 pos = lineData.offset; pos < textLength; ++pos)
    {
        char16 ch = preparedText[pos];
        uint8 canBreak = breaks[pos];

        currentWidth += characterSizes[pos];

        // Check that targetWidth defined and currentWidth less than targetWidth.
        // If symbol is whitespace skip it and go to next (add all whitespace to current line)
        if (currentWidth <= targetWidth || StringUtils::IsWhitespace(ch))
        {
            if (canBreak == StringUtils::LB_MUSTBREAK) // If symbol is line breaker then split string
            {
                lineData.length = pos - lineData.offset + 1;
                currentWidth = 0.f;
                lastPossibleBreak = 0;
                return true;
            }
            else if (canBreak == StringUtils::LB_ALLOWBREAK) // Store breakable symbol position
            {
                lastPossibleBreak = pos;
            }
            continue;
        }

        if (lastPossibleBreak > 0) // If we have any breakable symbol in current substring then split by it
        {
            pos = lastPossibleBreak;
            lineData.length = pos - lineData.offset + 1;
            currentWidth = 0.f;
            lastPossibleBreak = 0;
            return true;
        }

        return false;
    }

    DVASSERT_MSG(lineData.offset == textLength, "Incorrect line split");
    return false;
}

bool TextLayout::NextBySymbols(const float32 lineWidth)
{
    DVASSERT(characterSizes.size() == preparedText.length());
    float32 targetWidth = std::floor(lineWidth);
    float32 currentLineDx = 0;
    uint32 totalSize = uint32(preparedText.length());
    uint32 pos = 0;

    lineData.offset += lineData.length; // Move line cursor to next line
    lineData.length = 0;

    for (pos = lineData.offset; pos < totalSize; pos++)
    {
        char16 t = preparedText[pos];
        if (t == L'\n')
        {
            lineData.length = pos - lineData.offset + 1;
            return true;
        }

        float32 characterSize = characterSizes[pos];

        // Use additional condition to prevent endless loop, when target size is less than
        // size of one symbol (sizes[pos] > targetWidth)
        // To keep initial index logic we should always perform action currentLineDx += sizes[pos]
        // before entering this condition, so currentLineDx > 0.
        if ((currentLineDx > 0) && ((currentLineDx + characterSize) > targetWidth))
        {
            lineData.length = pos - lineData.offset;
            return true;
        }

        currentLineDx += characterSize;
    }

    lineData.length = pos - lineData.offset;
    return true;
}

const WideString TextLayout::GetVisualText(const bool trimEnd) const
{
    return BuildVisualString(preparedText, trimEnd);
}

const WideString TextLayout::GetVisualLine(const Line& line, const bool trimEnd) const
{
    WideString preparedLine = preparedText.substr(line.offset, line.length);
    return BuildVisualString(preparedLine, trimEnd);
}

void TextLayout::FillList(Vector<WideString>& outputList, float32 lineWidth, bool splitBySymbols, bool trimEnd)
{
    Vector<Line> lines;
    FillList(lines, lineWidth, splitBySymbols);
    for (auto& line : lines)
    {
        outputList.push_back(GetVisualLine(line, trimEnd));
    }
}

void TextLayout::FillList(Vector<TextLayout::Line>& outputList, float32 lineWidth, bool splitBySymbols)
{
    while (!IsEndOfText())
    {
        if (splitBySymbols || !NextByWords(lineWidth))
        {
            NextBySymbols(lineWidth);
        }
        outputList.push_back(GetLine());
    }
}

const WideString TextLayout::BuildVisualString(const WideString& _input, const bool trimEnd) const
{
    WideString output = _input;
    if (useBiDi)
    {
        bidiHelper.ReorderString(output, output, isRtl);
    }
    output = StringUtils::RemoveNonPrintable(output, 1);
    if (trimEnd)
    {
        output = !isRtl ? StringUtils::TrimRight(output) : StringUtils::TrimLeft(output);
    }
    return output;
}
}