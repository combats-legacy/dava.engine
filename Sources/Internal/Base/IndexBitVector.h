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


#ifndef __DAVAENGINE_INDEX_BIT_VECTOR_H__
#define __DAVAENGINE_INDEX_BIT_VECTOR_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
class IndexBitVector
{
public:
    IndexBitVector(uint32 initBits = 2, uint32 elementCount = 4);
    ~IndexBitVector();

    inline void PushBack(uint32 value);
    inline void PopBack();
    inline uint32 At(uint32 index);
    inline void Set(uint32 index, uint32 value);

    inline uint32 DataCount(uint32 elementCount);
    inline uint32 GetCount();

private:
    inline void Set(uint32* newData, uint32 index, uint32 value, uint32 bits);
    inline uint32 DataCount(uint32 elementCount, uint32 bits);

    void ReallocateSize(uint32 _newMaxElementCount);
    void ReallocateBits(uint32 _newBits);

    uint32 maxIndexBits;
    uint32 maxElementCount;
    uint32 currentElementCount;
    uint32* dataArray;
};

IndexBitVector::IndexBitVector(uint32 initBits, uint32 elementCount)
{
    maxIndexBits = initBits;
    maxElementCount = 0;
    dataArray = 0;
    ReallocateSize(elementCount);
}

IndexBitVector::~IndexBitVector()
{
    SafeDeleteArray(dataArray);
}

inline void IndexBitVector::PushBack(uint32 value)
{
    if (value > (1u << maxIndexBits) - 1u)
    {
        ReallocateBits(maxIndexBits << 1);
    }
    Set(currentElementCount, value);
    ++currentElementCount;
}

inline void IndexBitVector::PopBack()
{
    --currentElementCount;
}

inline uint32 IndexBitVector::GetCount()
{
    return currentElementCount;
}

inline void IndexBitVector::Set(uint32 index, uint32 value)
{
    uint32 arrayIndex = index >> maxIndexBits;
    uint32 bitElement = index - (arrayIndex << maxIndexBits);
    dataArray[arrayIndex] = (value << (bitElement * maxIndexBits)) & ((1 << maxIndexBits) - 1);
}

inline void IndexBitVector::Set(uint32* newData, uint32 index, uint32 value, uint32 bits)
{
    uint32 arrayIndex = index >> bits;
    uint32 bitElement = index - (arrayIndex << bits);
    newData[arrayIndex] = (value << (bitElement * bits)) & ((1 << bits) - 1);
}

inline uint32 IndexBitVector::At(uint32 index)
{
    uint32 arrayIndex = index >> maxIndexBits;
    uint32 bitElement = index - (arrayIndex << maxIndexBits);
    return dataArray[arrayIndex] >> (bitElement * maxIndexBits) & ((1 << maxIndexBits) - 1);
}

uint32 IndexBitVector::DataCount(uint32 elementCount)
{
    return (elementCount >> maxIndexBits) + 1;
}

uint32 IndexBitVector::DataCount(uint32 elementCount, uint32 bits)
{
    return (elementCount >> bits) + 1;
}

void IndexBitVector::ReallocateSize(uint32 _newMaxElementCount)
{
    uint32* newDataArray = new uint32[DataCount(_newMaxElementCount)];
    if (dataArray)
        Memcpy(newDataArray, dataArray, DataCount(currentElementCount));
    maxElementCount = _newMaxElementCount;
    SafeDeleteArray(dataArray);
    dataArray = newDataArray;
}

void IndexBitVector::ReallocateBits(uint32 _newBits)
{
    uint32* newDataArray = new uint32[DataCount(maxElementCount, _newBits)];
    for (uint32 k = 0; k < currentElementCount; ++k)
    {
        Set(newDataArray, k, At(k), _newBits);
    }
    maxIndexBits = _newBits;
    SafeDeleteArray(dataArray);
    dataArray = newDataArray;
}
};
#endif // __DAVAENGINE_INDEX_BIT_VECTOR_H__
