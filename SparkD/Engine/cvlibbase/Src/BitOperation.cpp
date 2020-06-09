/*!
 * \file	BitOperation.cpp
 * \ingroup base
 * \brief	
 * \author	
 */

#include "BitOperation.h"

namespace cvlib
{

#define MAXINT	( 1 << 31 )

unsigned int BitOperation::ror(unsigned int c, int n, int bits )
{
	return (c >> n) | ((c & ((1 << n) - 1)) << (bits - n));
}

unsigned int BitOperation::rol(unsigned int c, int n, int bits )
{
	return (c << n) | (((c & (((1 << n) - 1) << (bits - n))) >> (bits - n)) & ((1 << bits) - 1));
}

int BitOperation::oneCount(unsigned int c, int bits)
{
	int count = 0, base = 1;
	for (int i=0; i<bits;i++)
		{
			if (c & base) count++;
			base <<= 1;
		}
	return count;
}

int BitOperation::transitions(unsigned int c, int bits)
{
	int base = 1;
	int current = c & base, current2, changes = 0;
	for (int i=1;i<bits;i++)
		{
			base <<= 1;
			current2 = (c & base) >> i;
			if (current ^ current2) changes++;
			current = current2;
		}
	return changes; //(changes <= 2)? 1 : 0;
}

unsigned int BitOperation::rotMin(unsigned int n, int bits)
{
	unsigned int tmp = n << sizeof(int)*4;
	unsigned int lowmask = (unsigned int)(MAXINT >> (sizeof(int)*4-1));
	unsigned int min = tmp;
	int minIndex=0;
	for (int i=1;i<bits;i++)
		{
			tmp >>= 1;
			tmp |= (tmp & lowmask) << bits;
			tmp &= ~lowmask;
			if (tmp<min)
				{
					min = tmp;
					minIndex = i;
				}
		}
	tmp = (n << sizeof(int)*4) - minIndex;
	tmp |= (tmp & lowmask) << bits;
	tmp &= ~lowmask;
	return tmp >> sizeof(int)*4;
}

int BitOperation::hammingDistance(unsigned int a, unsigned int b, int bits)
{
	int c = a^b;
	int dist = 0, base = 1;
	for (int i=0;i<bits;i++)
		{
			if (base & c) dist++;
			base <<= 1;
		}
	return dist;
}

}
