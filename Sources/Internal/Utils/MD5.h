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

/*
**********************************************************************
** md5.h -- Header file for implementation of MD5                   **
** RSA Data Security, Inc. MD5 Message Digest Algorithm             **
** Created: 2/17/90 RLR                                             **
** Revised: 12/27/90 SRD,AJ,BSK,JT Reference C version              **
** Revised (for MD5): RLR 4/27/91                                   **
**   -- G modified to have y&~z instead of y&z                      **
**   -- FF, GG, HH modified to add in last register done            **
**   -- Access pattern: round 2 works mod 5, round 3 works mod 3    **
**   -- distinct additive constant for each step                    **
**   -- round 4 added, working mod 7                                **
**********************************************************************
*/

/*
**********************************************************************
** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved. **
**                                                                  **
** License to copy and use this software is granted provided that   **
** it is identified as the "RSA Data Security, Inc. MD5 Message     **
** Digest Algorithm" in all material mentioning or referencing this **
** software or this function.                                       **
**                                                                  **
** License is also granted to make and use derivative works         **
** provided that such works are identified as "derived from the RSA **
** Data Security, Inc. MD5 Message Digest Algorithm" in all         **
** material mentioning or referencing the derived work.             **
**                                                                  **
** RSA Data Security, Inc. makes no representations concerning      **
** either the merchantability of this software or the suitability   **
** of this software for any particular purpose.  It is provided "as **
** is" without express or implied warranty of any kind.             **
**                                                                  **
** These notices must be retained in any copies of any part of this **
** documentation and/or software.                                   **
**********************************************************************
*/

#ifndef __DAVAENGINE_MD5__
#define __DAVAENGINE_MD5__



#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
/* Data structure for MD5 (Message Digest) computation */

class MD5
{
public:
    struct MD5Digest
    {
        MD5Digest()
        {
            digest.fill(0);
        }

        bool operator==(const MD5Digest& right) const
        {
            return digest == right.digest;
        }

        static const int32 DIGEST_SIZE = 16;
        Array<uint8, DIGEST_SIZE> digest;
    };

public:
    static void ForData(const uint8* data, uint32 dataSize, MD5Digest& digest);
    static void ForFile(const FilePath& pathName, MD5Digest& digest);
    static void ForDirectory(const FilePath& pathName, MD5Digest& digest, bool isRecursive, bool includeHidden);

    static void HashToChar(const MD5Digest& digest, char8* buffer, uint32 bufferSize);
    static void HashToChar(const uint8* hash, uint32 hashSize, char8* buffer, uint32 bufferSize);

    static void CharToHash(const char8* buffer, MD5Digest& digest);
    static void CharToHash(const char8* buffer, uint32 bufferSize, uint8* hash, uint32 hashSize);

private:
    void Init();
    void Update(const uint8* inBuf, uint32 inLen);
    void Final();
    const MD5Digest& GetDigest()
    {
        return digest;
    }

    static void RecursiveDirectoryMD5(const FilePath& pathName, MD5& md5, bool isRecursive, bool includeHidden);

    static uint8 GetNumberFromCharacter(char8 character);
    static char8 GetCharacterFromNumber(uint8 number);

    uint32 i[2]; /* number of _bits_ handled mod 2^64 */
    uint32 buf[4]; /* scratch buffer */
    uint8 in[64]; /* input buffer */
    MD5Digest digest; /* actual digest after MD5Final call */
};
}

#endif //