/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Ivan Petrochenko
=====================================================================================*/
#ifndef __DAVAENGINE_FTFONT_H__
#define __DAVAENGINE_FTFONT_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/2D/Font.h"
#include "Platform/Mutex.h"

struct FT_FaceRec_;
typedef struct FT_FaceRec_*  FT_Face;

struct FT_GlyphRec_;
typedef struct FT_GlyphRec_*  FT_Glyph;

namespace DAVA
{	
	
class FontManager;
class FTInternalFont;



class FTFont : public Font
{	
public:
	FTInternalFont	* internalFont;

	static		FTFont * Create(const String& path);
	virtual		~FTFont();

	FTFont *	Clone();

	virtual bool IsEqual(Font *font);

	
	virtual Size2i		GetStringSize(const WideString & str, Vector<int32> *charSizes = NULL);
	virtual uint32		GetFontHeight();
	virtual int32		GetAscender();
	virtual int32		GetDescender();
	
	virtual bool		IsCharAvaliable(char16 ch);

	virtual bool IsTextSupportsSoftwareRendering() { return true; };
	virtual Size2i DrawStringToBuffer(void * buffer, int32 bufWidth, int32 bufHeight, int32 offsetX, int32 offsetY, int32 justifyWidth, int32 spaceAddon, const WideString & str, bool contentScaleIncluded = false);

private:
	FTFont(FTInternalFont* internalFont);
};


	
};

#endif  //__DAVAENGINE_FTFONT_H__