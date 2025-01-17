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
 NEON math library for the iPhone / iPod touch
 
 Copyright (c) 2009 Justin Saunders
 
 This software is provided 'as-is', without any express or implied warranty.
 In no event will the authors be held liable for any damages arising
 from the use of this software.
 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it freely,
 subject to the following restrictions:
 
 1. The origin of this software must not be misrepresented; you must
 not claim that you wrote the original software. If you use this
 software in a product, an acknowledgment in the product documentation
 would be appreciated but is not required.
 
 2. Altered source versions must be plainly marked as such, and must
 not be misrepresented as being the original software.
 
 3. This notice may not be removed or altered from any source distribution.
*/

#include "NeonMath.h"

#ifdef __DAVAENGINE_ARM_7__

namespace DAVA
{
// Note asm is taken from reader "Jeff" post Wolfgang's blog:
// https://www.blogger.com/comment.g?blogID=398682525365778708&postID=7527893703750196380&page=1

void NEON_Matrix4Mul(const float* a, const float* b, float* output)
{
    asm volatile
    (
    // Store A & B leaving room at top of registers for result (q0-q3)
    "vldmia %1, { q4-q7 }  \n\t"
    "vldmia %2, { q8-q11 } \n\t"

    // result = first column of B x first row of A
    "vmul.f32 q0, q8, d8[0]\n\t"
    "vmul.f32 q1, q8, d10[0]\n\t"
    "vmul.f32 q2, q8, d12[0]\n\t"
    "vmul.f32 q3, q8, d14[0]\n\t"

    // result += second column of B x second row of A
    "vmla.f32 q0, q9, d8[1]\n\t"
    "vmla.f32 q1, q9, d10[1]\n\t"
    "vmla.f32 q2, q9, d12[1]\n\t"
    "vmla.f32 q3, q9, d14[1]\n\t"

    // result += third column of B x third row of A
    "vmla.f32 q0, q10, d9[0]\n\t"
    "vmla.f32 q1, q10, d11[0]\n\t"
    "vmla.f32 q2, q10, d13[0]\n\t"
    "vmla.f32 q3, q10, d15[0]\n\t"

    // result += last column of B x last row of A
    "vmla.f32 q0, q11, d9[1]\n\t"
    "vmla.f32 q1, q11, d11[1]\n\t"
    "vmla.f32 q2, q11, d13[1]\n\t"
    "vmla.f32 q3, q11, d15[1]\n\t"

    // output = result registers
    "vstmia %0, { q0-q3 }"

    : // no output
    : "r"(output), "r"(a), "r"(b) // input - note *value* of pointer doesn't change
    : "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q11" //clobber
    );
}

void NEON_Matrix4Vector4Mul(const float* m, const float* v, float* output)
{
    asm volatile
    (
    // Store m & v leaving room at top of registers for result (q0)
    "vldmia %1, {q1-q4 }   \n\t" // q2-q5 = m
    "vldmia %2, {q5}               \n\t" // q1    = v

    // result = first column of A x V.x
    "vmul.f32 q0, q1, d10[0]\n\t"

    // result += second column of A x V.y
    "vmla.f32 q0, q2, d10[1]\n\t"

    // result += third column of A x V.z
    "vmla.f32 q0, q3, d11[0]\n\t"

    // result += last column of A x V.w
    "vmla.f32 q0, q4, d11[1]\n\t"

    // output = result registers
    "vstmia %0, {q0}"

    : // no output
    : "r"(output), "r"(m), "r"(v) // input - note *value* of pointer doesn't change
    : "memory", "q0", "q1", "q2", "q3", "q4", "q5" //clobber
    );
}
};

#endif //#ifdef __DAVAENGINE_ARM_7__
