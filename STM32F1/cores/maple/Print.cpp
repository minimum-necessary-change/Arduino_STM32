/*
 * Print.cpp - Base class that provides print() and println()
 * Copyright (c) 2008 David A. Mellis.  All right reserved.
 * Copyright (c) 2011 LeafLabs, LLC.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * Modified 23 November 2006 by David A. Mellis
 * Modified 12 April 2011 by Marti Bolivar <mbolivar@leaflabs.com>
 */

#include "Print.h"

#include "wirish_math.h"
#include "limits.h"

#ifndef LLONG_MAX
/*
 * Note:
 *
 * At time of writing (12 April 2011), the limits.h that came with the
 * newlib we distributed didn't include LLONG_MAX.  Because we're
 * staying away from using templates (see /notes/coding_standard.rst,
 * "Language Features and Compiler Extensions"), this value was
 * copy-pasted from a println() of the value
 *
 *     std::numeric_limits<long long>::max().
 */
#define LLONG_MAX 9223372036854775807LL
#endif

/*
 * Public methods
 */

size_t Print::write(const char *str) {
    if (str == NULL) return 0;
	return write((const uint8_t *)str, strlen(str));
}

size_t Print::write(const void *buffer, uint32 size) {
	size_t n = 0;
    uint8 *ch = (uint8*)buffer;
    while (size--) {
        write(*ch++);
        n++;
    }
	return n;
}

size_t Print::print(uint8 b, int base) {
    return print((uint64)b, base);
}

size_t Print::print(const String &s)
{
  return write(s.c_str(), s.length());
}

size_t Print::print(char c) {
    return write(c);
}

size_t Print::print(const char str[]) {
    return write(str);
}

size_t Print::print(int n, int base) {
    return print((long long)n, base);
}

size_t Print::print(unsigned int n, int base) {
    return print((unsigned long long)n, base);
}

size_t Print::print(long n, int base) {
    return print((long long)n, base);
}

size_t Print::print(unsigned long n, int base) {
    return print((unsigned long long)n, base);
}

size_t Print::print(long long n, int base) {
    if (n < 0) {
        print('-');
        n = -n;
    }
    return printNumber(n, base);
}

size_t Print::print(unsigned long long n, int base) {
	return printNumber(n, base);
}

size_t Print::print(double n, int digits) {
    return printFloat(n, digits);
}

size_t Print::print(const __FlashStringHelper *ifsh)
{
  return print(reinterpret_cast<const char *>(ifsh));
}

size_t Print::print(const Printable& x)
{
  return x.printTo(*this);
}

size_t Print::println(void) 
{
	size_t n =  print('\r');
    n += print('\n');
	return n;
}

size_t Print::println(const String &s)
{
  size_t n = print(s);
  n += println();
  return n;
}

size_t Print::println(char c) {
    size_t n = print(c);
    n += println();
	return n;
}

size_t Print::println(const char c[]) {
    size_t n = print(c);
    n += println();
	return n;
}

size_t Print::println(uint8 b, int base) {
    size_t n = print(b, base);
	n += println();
	return n;
}

size_t Print::println(int n, int base) {
    size_t s = print(n, base);
    s += println();
	return s;
}

size_t Print::println(unsigned int n, int base) {
    size_t s = print(n, base);
    s += println();
	return s;
}

size_t Print::println(long n, int base) {
    size_t s = print((long long)n, base);
    s += println();
	return s;
}

size_t Print::println(unsigned long n, int base) {
    size_t s = print((unsigned long long)n, base);
    s += println();
	return s;
}

size_t Print::println(long long n, int base) {
    size_t s = print(n, base);
    s += println();
	return s;
}

size_t Print::println(unsigned long long n, int base) {
    size_t s = print(n, base);
    s += println();
	return s;
}

size_t Print::println(double n, int digits) {
    size_t s = print(n, digits);
    s += println();
	return s;
}

size_t Print::println(const __FlashStringHelper *ifsh)
{
  size_t n = print(ifsh);
  n += println();
  return n;
}

size_t Print::println(const Printable& x)
{
  size_t n = print(x);
  n += println();
  return n;
}

#ifdef SUPPORTS_PRINTF
#include <stdio.h>
#include <stdarg.h>
// Work in progress to support printf.
// Need to implement stream FILE to write individual chars to chosen serial port
int Print::printf (__const char *__restrict __format, ...)
 {
FILE *__restrict __stream;
     int ret_status = 0;


     va_list args;
     va_start(args,__format);
     ret_status = vfprintf(__stream, __format, args);
     va_end(args);
     return ret_status;
 }
 #endif

/*
 * Private methods
 */

size_t Print::printNumber(unsigned long long n, uint8 base) {
    unsigned char buf[CHAR_BIT * sizeof(long long)];
    unsigned long i = 0;
	size_t s=0;
    if (n == 0) {
        print('0');
        return 1;
    }

    while (n > 0) {
        buf[i++] = n % base;
        n /= base;
    }

    for (; i > 0; i--) {
        s += print((char)(buf[i - 1] < 10 ?
                     '0' + buf[i - 1] :
                     'A' + buf[i - 1] - 10));
    }
	return s;
}


/* According to snprintf(),
 *
 * nextafter((double)numeric_limits<long long>::max(), 0.0) ~= 9.22337e+18
 *
 * This slightly smaller value was picked semi-arbitrarily. */
#define LARGE_DOUBLE_TRESHOLD (9.1e18)

/* THIS FUNCTION SHOULDN'T BE USED IF YOU NEED ACCURATE RESULTS.
 *
 * This implementation is meant to be simple and not occupy too much
 * code size.  However, printing floating point values accurately is a
 * subtle task, best left to a well-tested library function.
 *
 * See Steele and White 2003 for more details:
 *
 * http://kurtstephens.com/files/p372-steele.pdf
 */
size_t Print::printFloat(double number, uint8 digits) {
size_t s=0;
    // Hackish fail-fast behavior for large-magnitude doubles
    if (abs(number) >= LARGE_DOUBLE_TRESHOLD) {
        if (number < 0.0) {
            s=print('-');
        }
        s+=print("<large double>");
        return s;
    }

    // Handle negative numbers
    if (number < 0.0) {
        s+=print('-');
        number = -number;
    }

    // Simplistic rounding strategy so that e.g. print(1.999, 2)
    // prints as "2.00"
    double rounding = 0.5;
    for (uint8 i = 0; i < digits; i++) {
        rounding /= 10.0;
    }
    number += rounding;

    // Extract the integer part of the number and print it
    long long int_part = (long long)number;
    double remainder = number - int_part;
    s+=print(int_part);

    // Print the decimal point, but only if there are digits beyond
    if (digits > 0) {
        s+=print(".");
    }

    // Extract digits from the remainder one at a time
    while (digits-- > 0) {
        remainder *= 10.0;
        int to_print = (int)remainder;
        s+=print(to_print);
        remainder -= to_print;
    }
	return s;
}

#if defined(STM32F103_USE_LEAN_PRINTF)
//------------------------------------------------
// ref: https://www.stm32duino.com/viewtopic.php?f=18&t=1014&sid=b5b2a990c12312751dbde58f9e3802fb

#ifdef toupper
#undef toupper
#endif
#ifdef tolower
#undef tolower
#endif
#ifdef islower
#undef islower
#endif
#ifdef isdigit
#undef isdigit
#endif

#define toupper(c) ((c)&=0xDF)
#define tolower(c) ((c)|=0x20)
#define islower(c) ((unsigned char)c >= (unsigned char)'a' && (unsigned char)c <= (unsigned char)'z')
#define isdigit(c) ((unsigned char)c >= (unsigned char)'0' && (unsigned char)c <= (unsigned char)'9')

typedef union {
    unsigned char  byte[5];
    long           l;
    unsigned long  ul;
    float          f;
    const char     *ptr;
} value_t;


size_t Print::printDigit(unsigned char n, bool lower_case)
{
    register unsigned char c = n + (unsigned char)'0';

    if (c > (unsigned char)'9') {
        c += (unsigned char)('A' - '0' - 10);
        if (lower_case)
            c += (unsigned char)('a' - 'A');
    }
    return write(c);
}

static void calculateDigit (value_t* value, unsigned char radix)
{
    unsigned long ul = value->ul;
    unsigned char* pb4 = &value->byte[4];
    unsigned char i = 32;

    do {
        *pb4 = (*pb4 << 1) | ((ul >> 31) & 0x01);
        ul <<= 1;

        if (radix <= *pb4 ) {
            *pb4 -= radix;
            ul |= 1;
        }
    } while (--i);
    value->ul = ul;
}

size_t Print::printf(const char *format, ...)
{
    va_list ap;
    bool   left_justify;
    bool   zero_padding;
    bool   prefix_sign;
    bool   prefix_space;
    bool   signed_argument;
    bool   char_argument;
    bool   long_argument;
    bool   lower_case;
    value_t value;
    int charsOutputted;
    bool   lsd;

    unsigned char radix;
    unsigned char  width;
    signed char decimals;
    unsigned char  length;
    char           c;
    // reset output chars
    charsOutputted = 0;

    va_start(ap, format);
    while( c=*format++ ) {
        if ( c=='%' ) {
            left_justify    = 0;
            zero_padding    = 0;
            prefix_sign     = 0;
            prefix_space    = 0;
            signed_argument = 0;
            char_argument   = 0;
            long_argument   = 0;
            radix           = 0;
            width           = 0;
            decimals        = -1;

get_conversion_spec:
            c = *format++;

            if (c=='%') {
                charsOutputted+=write(c);
                continue;
            }

            if (isdigit(c)) {
                if (decimals==-1) {
                    width = 10*width + c - '0';
                    if (width == 0) {
                        zero_padding = 1;
                    }
                } else {
                    decimals = 10*decimals + c - '0';
                }
                goto get_conversion_spec;
            }
            if (c=='.') {
                if (decimals==-1)
                    decimals=0;
                else
                    ; // duplicate, ignore
                goto get_conversion_spec;
            }
            if (islower(c)) {
                c = toupper(c);
                lower_case = 1;
            } else
                lower_case = 0;

            switch( c ) {
            case '-':
                left_justify = 1;
                goto get_conversion_spec;
            case '+':
                prefix_sign = 1;
                goto get_conversion_spec;
            case ' ':
                prefix_space = 1;
                goto get_conversion_spec;
            case 'B': /* byte */
                char_argument = 1;
                goto get_conversion_spec;
//      case '#': /* not supported */
            case 'H': /* short */
            case 'J': /* intmax_t */
            case 'T': /* ptrdiff_t */
            case 'Z': /* size_t */
                goto get_conversion_spec;
            case 'L': /* long */
                long_argument = 1;
                goto get_conversion_spec;

            case 'C':
                if( char_argument )
                    c = va_arg(ap,char);
                else
                    c = va_arg(ap,int);
                charsOutputted+=write(c);
                break;

            case 'S':
                value.ptr = va_arg(ap,const char *);

                length = strlen(value.ptr);
                if ( decimals == -1 ) {
                    decimals = length;
                }
                if ( ( !left_justify ) && (length < width) ) {
                    width -= length;
                    while( width-- != 0 ) {
                        charsOutputted+=write(' ');
                    }
                }

                while ( (c = *value.ptr)  && (decimals-- > 0)) {
                    charsOutputted+=write(c);
                    value.ptr++;
                }

                if ( left_justify && (length < width)) {
                    width -= length;
                    while( width-- != 0 ) {
                        charsOutputted+=write(' ');
                    }
                }
                break;

            case 'D':
            case 'I':
                signed_argument = 1;
                radix = 10;
                break;

            case 'O':
                radix = 8;
                break;

            case 'U':
                radix = 10;
                break;

            case 'X':
                radix = 16;
                break;

            default:
                // nothing special, just output the character
                charsOutputted+=write(c);
                break;
            }

            if (radix != 0) {
                unsigned char store[6];
                unsigned char *pstore = &store[5];

                if (char_argument) {
                    value.l = va_arg(ap, char);
                    if (!signed_argument) {
                        value.l &= 0xFF;
                    }
                } else if (long_argument) {
                    value.l = va_arg(ap, long);
                } else { // must be int
                    value.l = va_arg(ap, int);
                    if (!signed_argument) {
                        value.l &= 0xFFFF;
                    }
                }

                if ( signed_argument ) {
                    if (value.l < 0)
                        value.l = -value.l;
                    else
                        signed_argument = 0;
                }

                length=0;
                lsd = 1;

                do {
                    value.byte[4] = 0;
                    calculateDigit(&value, radix);
                    if (!lsd) {
                        *pstore = (value.byte[4] << 4) | (value.byte[4] >> 4) | *pstore;
                        pstore--;
                    } else {
                        *pstore = value.byte[4];
                    }
                    length++;
                    lsd = !lsd;
                } while( value.ul );
                if (width == 0) {
                    // default width. We set it to 1 to output
                    // at least one character in case the value itself
                    // is zero (i.e. length==0)
                    width = 1;
                }
                /* prepend spaces if needed */
                if (!zero_padding && !left_justify) {
                    while ( width > (unsigned char) (length+1) ) {
                        charsOutputted+=write(' ');
                        width--;
                    }
                }
                if (signed_argument) { // this now means the original value was negative
                    charsOutputted+=write('-');
                    // adjust width to compensate for this character
                    width--;
                } else if (length != 0) {
                    // value > 0
                    if (prefix_sign) {
                        charsOutputted+=write('+');
                        // adjust width to compensate for this character
                        width--;
                    } else if (prefix_space) {
                        charsOutputted+=write(' ');
                        // adjust width to compensate for this character
                        width--;
                    }
                }
                /* prepend zeroes/spaces if needed */
                if (!left_justify) {
                    while ( width-- > length ) {
                        charsOutputted+=write( zero_padding ? '0' : ' ');
                    }
                } else {
                    /* spaces are appended after the digits */
                    if (width > length)
                        width -= length;
                    else
                        width = 0;
                }
                 /* output the digits */
                while( length-- ) {
                    lsd = !lsd;
                    if (!lsd) {
                        pstore++;
                        value.byte[4] = *pstore >> 4;
                    } else {
                        value.byte[4] = *pstore & 0x0F;
                    }
                    charsOutputted+=printDigit(value.byte[4], lower_case);
                }
            }
        } else {
            charsOutputted+=write(c);
        }
    }
    va_end(ap);
    return (size_t)charsOutputted;
}
#endif
