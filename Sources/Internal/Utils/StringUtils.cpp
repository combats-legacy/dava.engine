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


#include "DAVAEngine.h"
#include "StringUtils.h"
#include "Utils/UTF8Utils.h"

#include "unibreak/linebreak.h"

namespace DAVA
{
void StringUtils::GetLineBreaks(const WideString& string, Vector<uint8>& breaks, const char8* locale)
{
    breaks.resize(string.length(), LB_NOBREAK); // By default all characters not breakable
#if defined(__DAVAENGINE_WINDOWS__) // sizeof(wchar_t) == 2
    set_linebreaks_utf16(reinterpret_cast<const utf16_t*>(string.c_str()), string.length(), locale, reinterpret_cast<char*>(&breaks.front()));
#else
    set_linebreaks_utf32(reinterpret_cast<const utf32_t*>(string.c_str()), string.length(), locale, reinterpret_cast<char*>(&breaks.front()));
#endif
}

WideString StringUtils::Trim(const WideString& string)
{
    WideString::const_iterator it = string.begin();
    WideString::const_iterator end = string.end();
    WideString::const_reverse_iterator rit = string.rbegin();
    while (it != end && IsWhitespace(*it)) ++it;
    while (rit.base() != it && IsWhitespace(*rit)) ++rit;
    return WideString(it, rit.base());
}

WideString StringUtils::TrimLeft(const WideString& string)
{
    WideString::const_iterator it = string.begin();
    WideString::const_iterator end = string.end();
    while (it != end && IsWhitespace(*it)) ++it;
    return WideString(it, end);
}

WideString StringUtils::TrimRight(const WideString& string)
{
    WideString::const_reverse_iterator rit = string.rbegin();
    WideString::const_reverse_iterator rend = string.rend();
    while (rit != rend && IsWhitespace(*rit)) ++rit;
    return WideString(rend.base(), rit.base());
}

WideString StringUtils::RemoveNonPrintable(const WideString& string, const int8 tabRule /*= -1*/)
{
    WideString out;
    WideString::const_iterator it = string.begin();
    WideString::const_iterator end = string.end();
    for (; it != end; ++it)
    {
        switch (*it)
        {
        case L'\n':
        case L'\r':
        case 0x200B: // Zero-width space
        case 0x200E: // Zero-width Left-to-right zero-width character
        case 0x200F: // Zero-width Right-to-left zero-width non-Arabic character
        case 0x061C: // Right-to-left zero-width Arabic character
            // Skip this characters (remove it)
            break;
        case L'\t':
            if (tabRule < 0)
            {
                out.push_back(*it); // Leave tab symbol
            }
            else
            {
                out.append(tabRule, L' '); // Replace tab with (tabRule x spaces)
            }
            break;
        case 0x00A0: // Non-break space
            out.push_back(L' ');
            break;
        default:
            out.push_back(*it);
            break;
        }
    }
    return out;
}

bool IsEmoji(int32 sym)
{
    // ranges of symbol codes with unicode emojies.
    static Vector<std::pair<DAVA::int32, DAVA::int32>> ranges = { { 0x2190, 0x21FF }, { 0x2600, 0x26FF }, { 0x2700, 0x27BF }, { 0x3000, 0x303F }, { 0xF300, 0x1F64F }, { 0x1F680, 0x1F6FF } };
    for (auto range : ranges)
    {
        if (sym >= range.first && sym <= range.second)
        {
            return true;
        }
    }
    return false;
}

bool StringUtils::RemoveEmoji(WideString& string)
{
    DAVA::WideString ret;
    bool isChanged = false;

    auto data = string.data();
    size_t length = string.length();
    for (size_t i = 0; i < length; ++i)
    {
        int32 sym;
        Memcpy(&sym, data + i, sizeof(int32));

        if (!IsEmoji(sym))
        {
            ret += sym;
        }
        else
        {
            isChanged = true;
        }
    }

    string = ret;

    // true means "we removed some emojies".
    return isChanged;
}
}