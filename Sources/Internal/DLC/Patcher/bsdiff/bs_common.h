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


#ifndef __BS_COMMON_H__
#define __BS_COMMON_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" { // only need to export C interface if
// used by C++ source code
#endif

typedef enum
{
    BS_PLAIN,
    BS_ZLIB
}
BSType;

struct bsdiff_stream
{
    void* opaque;
    BSType type;

    void* (*malloc)(int64_t size);
    void (*free)(void* ptr);
    int (*write)(struct bsdiff_stream* stream, const void* buffer, int64_t size);
};

struct bspatch_stream
{
    void* opaque;
    BSType type;

    int (*read)(const struct bspatch_stream* stream, void* buffer, int64_t size);
};

int bspatch(const uint8_t* olddata, int64_t oldsize, uint8_t* newdata, int64_t newsize, struct bspatch_stream* stream);
int bsdiff(const uint8_t* olddata, int64_t oldsize, const uint8_t* newdata, int64_t newsize, struct bsdiff_stream* stream);
void offtout(int64_t x, uint8_t* buf);
int64_t offtin(uint8_t* buf);

#ifdef __cplusplus
}
#endif

#endif // __BS_COMMON_H__
