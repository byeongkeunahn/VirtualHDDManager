
#pragma once

/* Mathematical Functions */

// RoundDown, RoundUp (iRound를 uiBase에 기준하여)
template <typename T1, typename T2> T1 RoundDown(T1 iRound, T2 uiBase)
{
	if (!uiBase)
		return (((T1)(1)) << (sizeof(T1) * 8 - 1));

	iRound /= uiBase;
	iRound *= uiBase;

	return iRound;
}

template <typename T1, typename T2> T1 RoundUp(T1 iRound, T2 uiBase)
{
	if (!uiBase)
		return (((T1)(1)) << (sizeof(T1)* 8 - 1));

	iRound--;
	iRound /= uiBase;
	iRound++;
	iRound *= uiBase;

	return iRound;

}

/* Byte Order Conversion Functions */

// Big Endian --> Native Endian conversion
template <typename T> T BToN(T val)
{
	T ret = 0;
	unsigned char *s = (unsigned char *)&val;
	for (int i = 0; i < sizeof(T); ++i) ret |= ((T)s[sizeof(T)-1 - i]) << (i * 8);

	return ret;
}

// Native Endian --> Big Endian conversion
template <typename T> T NToB(T val)
{
	T ret = 0;
	unsigned char *d = (unsigned char *)&ret;
	for (int i = 0; i < sizeof(T); ++i) d[sizeof(T)-1 - i] = (unsigned char)((ret >> (i * 8)) & 0xFF);

	return ret;
}

// Little Endian --> Native Endian conversion
template <typename T> T LToN(T val)
{
	T ret = 0;
	unsigned char *s = (unsigned char *)&val;
	for (int i = 0; i < sizeof(T); ++i) ret |= ((T)s[i]) << (i * 8);

	return ret;
}

// Native Endian --> Little Endian conversion
template <typename T> T NToL(T val)
{
	T ret = 0;
	unsigned char *d = (unsigned char *)&ret;
	for (int i = 0; i < sizeof(T); ++i) d[i] = (unsigned char)((ret >> (i * 8)) & 0xFF);

	return ret;
}

// Byteswap
template <typename T> T Byteswap(T val)
{
	T ret = 0;
	unsigned char *s = (unsigned char *)&val;
	unsigned char *d = (unsigned char *)&ret;
	for (int i = 0; i < sizeof(T); ++i) d[i] = s[sizeof(T)-1 - i];

	return ret;
}
