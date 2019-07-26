#ifndef TIFFCONF_H_MULTILIB
#define TIFFCONF_H_MULTILIB

#ifndef MAC_OSX
#include <bits/wordsize.h>
#else
#include <stdint.h>
#endif

#if __WORDSIZE == 32
# include "tiffconf-32.h"
#elif __WORDSIZE == 64
# include "tiffconf-64.h"
#else
# error "unexpected value for __WORDSIZE macro"
#endif

#endif
