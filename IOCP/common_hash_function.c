#include "common_hash_function.h"


unsigned long hash_integer(long key)
{
	//unsigned long m = 16384;
	//double A = 2654435769/(0x80000000);
	//return (unsigned long)m*((key*A)%1);
	//return key & 65537;

	unsigned long ret = key * 2654435769;
	return ret;// >> (32 - 14);

}

unsigned long hash_float(double key)
{
	return 0;
}

unsigned long hash_string(const char *key)
{
	return 0;
}