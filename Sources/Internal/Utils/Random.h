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

// MersenneTwister.h
// Mersenne Twister random number generator -- a C++ class MTRand
// Based on code by Makoto Matsumoto, Takuji Nishimura, and Shawn Cokus
// Richard J. Wagner  v1.1  28 September 2009  wagnerr@umich.edu

// The Mersenne Twister is an algorithm for generating random numbers.  It
// was designed with consideration of the flaws in various other generators.
// The period, 2^19937-1, and the order of equidistribution, 623 dimensions,
// are far greater.  The generator is also fast; it avoids multiplication and
// division, and it benefits from caches and pipelines.  For more information
// see the inventors' web page at
// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html

// Reference
// M. Matsumoto and T. Nishimura, "Mersenne Twister: A 623-Dimensionally
// Equidistributed Uniform Pseudo-Random Number Generator", ACM Transactions on
// Modeling and Computer Simulation, Vol. 8, No. 1, January 1998, pp 3-30.

// Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
// Copyright (C) 2000 - 2009, Richard J. Wagner
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//   3. The names of its contributors may not be used to endorse or promote
//      products derived from this software without specific prior written
//      permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// The original code included the following notice:
//
//     When you use this, send an email to: m-mat@math.sci.hiroshima-u.ac.jp
//     with an appropriate reference to your work.
//
// It would be nice to CC: wagnerr@umich.edu and Cokus@math.washington.edu
// when you write.

#ifndef MERSENNETWISTER_H
#define MERSENNETWISTER_H

// Not thread safe (unless auto-initialization is avoided and each thread has
// its own MTRand object)

#include <iostream>
#include <climits>
#include <cstdio>
#include <ctime>
#include <cmath>
#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Debug/Replay.h"

namespace DAVA
{
inline uint32 Rand();
class Random : public Singleton<Random>
{
    // Data
public:
    enum
    {
        N = 624
    }; // length of state vector
    enum
    {
        SAVE = N + 1
    }; // length of array for save()

protected:
    enum
    {
        M = 397
    }; // period parameter

    uint32 state[N]; // internal state
    uint32* pNext; // next value to get from state
    int32 left; // number of values left before reload needed

    // Methods
public:
    Random(const uint32 oneSeed); // initialize with a simple uint32
    //Random(uint32 *const bigSeed, uint32 const seedLength = N);  // or array
    Random(); // auto-initialize with /dev/urandom or time() and clock()
    Random(const Random& o); // copy

    // Do NOT use for CRYPTOGRAPHY without securely hashing several returned
    // values together, otherwise the generator state can be learned after
    // reading 624 consecutive values.

    // Access to 32-bit random numbers
    uint32 Rand(); // integer in [0,2^32-1]
    uint32 Rand(const uint32 n); // integer in [0,n] for n < 2^32
    float64 RandFloat(); // real number in [0,1]
    float64 RandFloat(const float64 n); // real number in [0,n]
    float64 RandExc(); // real number in [0,1)
    float64 RandExc(const float64 n); // real number in [0,n)
    float64 RandDblExc(); // real number in (0,1)
    float64 RandDblExc(const float64 n); // real number in (0,n)
    float64 operator()(); // same as rand()

    float32 RandFloat32InBounds(const float32 lower, const float32 upper);

    // Access to 53-bit random numbers (capacity of IEEE float64 precision)
    float64 Rand53(); // real number in [0,1)

    // Access to nonuniform random number distributions
    float64 RandNorm(const float64 mean = 0.0, const float64 stddev = 1.0);

    // Re-seeding functions with same behavior as initializers
    void Seed(const uint32 oneSeed);
    //void Seed(uint32 *const bigSeed, const uint32 seedLength = N);
    void Seed();

    // Saving and loading generator state
    void Save(uint32* saveArray) const; // to array of size SAVE
    void Load(uint32* const loadArray); // from such array
    friend std::ostream& operator<<(std::ostream& os, const Random& mtrand);
    friend std::istream& operator>>(std::istream& is, Random& mtrand);
    Random& operator=(const Random& o);

protected:
    void initialize(const uint32 oneSeed);
    void reload();
    uint32 hiBit(const uint32 u) const
    {
        return u & 0x80000000UL;
    }
    uint32 loBit(const uint32 u) const
    {
        return u & 0x00000001UL;
    }
    uint32 loBits(const uint32 u) const
    {
        return u & 0x7fffffffUL;
    }
    uint32 mixBits(const uint32 u, const uint32 v) const
    {
        return hiBit(u) | loBits(v);
    }
    uint32 magic(const uint32 u) const
    {
        return loBit(u) ? 0x9908b0dfUL : 0x0UL;
    }
    uint32 twist(const uint32 m, const uint32 s0, const uint32 s1) const
    {
        return m ^ (mixBits(s0, s1) >> 1) ^ magic(s1);
    }
    static uint32 hash(time_t t, clock_t c);
};

inline uint32 Rand()
{
    return Random::Instance()->Rand();
}

// Functions are defined in order of usage to assist inlining

inline uint32 Random::hash(time_t t, clock_t c)
{
    // Get a uint32 from t and c
    // Better than uint32(x) in case x is floating point in [0,1]
    // Based on code by Lawrence Kirby (fred@genesis.demon.co.uk)

    static uint32 differ = 0; // guarantee time-based seeds will change

    uint32 h1 = 0;
    unsigned char* p = reinterpret_cast<unsigned char*>(&t);
    for (size_t i = 0; i < sizeof(t); ++i)
    {
        h1 *= UCHAR_MAX + 2U;
        h1 += p[i];
    }
    uint32 h2 = 0;
    p = reinterpret_cast<unsigned char*>(&c);
    for (size_t j = 0; j < sizeof(c); ++j)
    {
        h2 *= UCHAR_MAX + 2U;
        h2 += p[j];
    }
    return (h1 + differ++) ^ h2;
}

inline void Random::initialize(const uint32 seed)
{
    // Initialize generator state with seed
    // See Knuth TAOCP Vol 2, 3rd Ed, p.106 for multiplier.
    // In previous versions, most significant bits (MSBs) of the seed affect
    // only MSBs of the state array.  Modified 9 Jan 2002 by Makoto Matsumoto.
    uint32* s = state;
    uint32* r = state;
    int32 i = 1;
    *s++ = seed & 0xffffffffUL;
    for (; i < N; ++i)
    {
        *s++ = (1812433253UL * (*r ^ (*r >> 30)) + i) & 0xffffffffUL;
        r++;
    }
}

inline void Random::reload()
{
    // Generate N new values in state
    // Made clearer and faster by Matthew Bellew (matthew.bellew@home.com)
    static const int32 MmN = int32(M) - int32(N); // in case enums are unsigned
    uint32* p = state;
    int32 i;
    for (i = N - M; i--; ++p)
        *p = twist(p[M], p[0], p[1]);
    for (i = M; --i; ++p)
        *p = twist(p[MmN], p[0], p[1]);
    *p = twist(p[MmN], p[0], state[0]);

    left = N, pNext = state;
}

inline void Random::Seed(const uint32 oneSeed)
{
    if (Replay::IsRecord())
    {
        Replay::Instance()->RecordSeed(oneSeed);
    }
    // Seed the generator with a simple uint32
    initialize(oneSeed);
    reload();
}

//inline void Random::Seed( uint32 *const bigSeed, const uint32 seedLength )
//{
//	// Seed the generator with an array of uint32's
//	// There are 2^19937-1 possible initial states.  This function allows
//	// all of those to be accessed by providing at least 19937 bits (with a
//	// default seed length of N = 624 uint32's).  Any bits above the lower 32
//	// in each element are discarded.
//	// Just call seed() if you want to get array from /dev/urandom
//	initialize(19650218UL);
//	register int32 i = 1;
//	register uint32 j = 0;
//	register int32 k = ( N > seedLength ? N : seedLength );
//	for( ; k; --k )
//	{
//		state[i] =
//		state[i] ^ ( (state[i-1] ^ (state[i-1] >> 30)) * 1664525UL );
//		state[i] += ( bigSeed[j] & 0xffffffffUL ) + j;
//		state[i] &= 0xffffffffUL;
//		++i;  ++j;
//		if( i >= N ) { state[0] = state[N-1];  i = 1; }
//		if( j >= seedLength ) j = 0;
//	}
//	for( k = N - 1; k; --k )
//	{
//		state[i] =
//		state[i] ^ ( (state[i-1] ^ (state[i-1] >> 30)) * 1566083941UL );
//		state[i] -= i;
//		state[i] &= 0xffffffffUL;
//		++i;
//		if( i >= N ) { state[0] = state[N-1];  i = 1; }
//	}
//	state[0] = 0x80000000UL;  // MSB is 1, assuring non-zero initial array
//	reload();
//}

inline void Random::Seed()
{
    // Seed the generator with an array from /dev/urandom if available
    // Otherwise use a hash of time() and clock() values

    // First try getting an array from /dev/urandom
    //FILE* urandom = fopen( "/dev/urandom", "rb" );
    //if( urandom )
    //{
    //	uint32 bigSeed[N];
    //	register uint32 *s = bigSeed;
    //	register int32 i = N;
    //	register bool success = true;
    //	while( success && i-- )
    //		success = fread( s++, sizeof(uint32), 1, urandom );
    //	fclose(urandom);
    //	if( success ) { Seed( bigSeed, N );  return; }
    //}

    // Was not successful, so use time() and clock() instead
    Seed(hash(time(NULL), clock()));
}

inline Random::Random(const uint32 oneSeed)
{
    Seed(oneSeed);
}

//inline Random::Random( uint32 *const bigSeed, const uint32 seedLength )
//	{ Seed(bigSeed,seedLength); }

inline Random::Random()
{
    Seed();
}

inline Random::Random(const Random& o)
{
    const uint32* t = o.state;
    uint32* s = state;
    int32 i = N;
    for (; i--; *s++ = *t++)
    {
    }
    left = o.left;
    pNext = &state[N - left];
}

inline uint32 Random::Rand()
{
    // Pull a 32-bit integer from the generator state
    // Every other access function simply transforms the numbers extracted here

    if (left == 0)
        reload();
    --left;

    uint32 s1;
    s1 = *pNext++;
    s1 ^= (s1 >> 11);
    s1 ^= (s1 << 7) & 0x9d2c5680UL;
    s1 ^= (s1 << 15) & 0xefc60000UL;
    return (s1 ^ (s1 >> 18));
}

inline uint32 Random::Rand(const uint32 n)
{
    // Find which bits are used in n
    // Optimized by Magnus Jonsson (magnus@smartelectronix.com)
    uint32 used = n;
    used |= used >> 1;
    used |= used >> 2;
    used |= used >> 4;
    used |= used >> 8;
    used |= used >> 16;

    // Draw numbers until one is found in [0,n]
    uint32 i;
    do
        i = Rand() & used; // toss unused bits to shorten search
    while (i > n);
    return i;
}

inline float64 Random::RandFloat()
{
    return float64(Rand()) * (1.0 / 4294967295.0);
}

inline float64 Random::RandFloat(const float64 n)
{
    return RandFloat() * n;
}

inline float64 Random::RandExc()
{
    return float64(Rand()) * (1.0 / 4294967296.0);
}

inline float64 Random::RandExc(const float64 n)
{
    return RandExc() * n;
}

inline float64 Random::RandDblExc()
{
    return (float64(Rand()) + 0.5) * (1.0 / 4294967296.0);
}

inline float64 Random::RandDblExc(const float64 n)
{
    return RandDblExc() * n;
}

inline float64 Random::Rand53()
{
    uint32 a = Rand() >> 5, b = Rand() >> 6;
    return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0); // by Isaku Wada
}

inline float64 Random::RandNorm(const float64 mean, const float64 stddev)
{
    // Return a real number from a normal (Gaussian) distribution with given
    // mean and standard deviation by polar form of Box-Muller transformation
    float64 x, y, r;
    do
    {
        x = 2.0 * RandFloat() - 1.0;
        y = 2.0 * RandFloat() - 1.0;
        r = x * x + y * y;
    }
    while (r >= 1.0 || r == 0.0);
    float64 s = sqrt(-2.0 * log(r) / r);
    return mean + x * s * stddev;
}

inline float64 Random::operator()()
{
    return RandFloat();
}

inline float32 Random::RandFloat32InBounds(DAVA::float32 lower, DAVA::float32 upper)
{
    return lower + (upper - lower) * float32(RandFloat());
}

//inline void Random::Save( uint32* saveArray ) const
//{
//	register const uint32 *s = state;
//	register uint32 *sa = saveArray;
//	register int32 i = N;
//	for( ; i--; *sa++ = *s++ ) {}
//	*sa = left;
//}
//
//inline void Random::Load( uint32 *const loadArray )
//{
//	register uint32 *s = state;
//	register uint32 *la = loadArray;
//	register int32 i = N;
//	for( ; i--; *s++ = *la++ ) {}
//	left = *la;
//	pNext = &state[N-left];
//}
//
//inline std::ostream& operator<<( std::ostream& os, const Random& mtrand )
//{
//	register const	uint32 *s = mtrand.state;
//	register int32 i = mtrand.N;
//	for( ; i--; os << *s++ << "\t" ) {}
//	return os << mtrand.left;
//}
//
//inline std::istream& operator>>( std::istream& is, Random& mtrand )
//{
//	register uint32 *s = mtrand.state;
//	register int32 i = mtrand.N;
//	for( ; i--; is >> *s++ ) {}
//	is >> mtrand.left;
//	mtrand.pNext = &mtrand.state[mtrand.N-mtrand.left];
//	return is;
//}
//
//inline Random& Random::operator=( const Random& o )
//{
//	if( this == &o ) return (*this);
//	register const uint32 *t = o.state;
//	register uint32 *s = state;
//	register int32 i = N;
//	for( ; i--; *s++ = *t++ ) {}
//	left = o.left;
//	pNext = &state[N-left];
//	return (*this);
//}
};

#endif // MERSENNETWISTER_H

// Change log:
//
// v0.1 - First release on 15 May 2000
//      - Based on code by Makoto Matsumoto, Takuji Nishimura, and Shawn Cokus
//      - Translated from C to C++
//      - Made completely ANSI compliant
//      - Designed convenient interface for initialization, seeding, and
//        obtaining numbers in default or user-defined ranges
//      - Added automatic seeding from /dev/urandom or time() and clock()
//      - Provided functions for saving and loading generator state
//
// v0.2 - Fixed bug which reloaded generator one step too late
//
// v0.3 - Switched to clearer, faster reload() code from Matthew Bellew
//
// v0.4 - Removed trailing newline in saved generator format to be consistent
//        with output format of built-in types
//
// v0.5 - Improved portability by replacing static const int32's with enum's and
//        clarifying return values in seed(); suggested by Eric Heimburg
//      - Removed MAXINT constant; use 0xffffffffUL instead
//
// v0.6 - Eliminated seed overflow when uint32 is larger than 32 bits
//      - Changed integer [0,n] generator to give better uniformity
//
// v0.7 - Fixed operator precedence ambiguity in reload()
//      - Added access for real numbers in (0,1) and (0,n)
//
// v0.8 - Included time.h header to properly support time_t and clock_t
//
// v1.0 - Revised seeding to match 26 Jan 2002 update of Nishimura and Matsumoto
//      - Allowed for seeding with arrays of any length
//      - Added access for real numbers in [0,1) with 53-bit resolution
//      - Added access for real numbers from normal (Gaussian) distributions
//      - Increased overall speed by optimizing twist()
//      - Doubled speed of integer [0,n] generation
//      - Fixed out-of-range number generation on 64-bit machines
//      - Improved portability by substituting literal constants for long enum's
//      - Changed license from GNU LGPL to BSD
//
// v1.1 - Corrected parameter label in randNorm from "variance" to "stddev"
//      - Changed randNorm algorithm from basic to polar form for efficiency
//      - Updated includes from deprecated <xxxx.h> to standard <cxxxx> forms
//      - Cleaned declarations and definitions to please Intel compiler
//      - Revised twist() operator to work on ones'-complement machines
//      - Fixed reload() function to work when N and M are unsigned
//      - Added copy constructor and copy operator from Salvador Espana
