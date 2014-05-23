Produced by config version 4.1, CWI, Amsterdam
Compiled without signed char
Compiler does not claim to be ANSI C

Char = 8 bits, signed
Short=16 int=32 long=32 float=32 double=64 bits 
Char pointers = 32 bits
Int pointers = 32 bits
Overflow of a short does not generate a trap
Maximum short = 32767 (= 2**15-1)
Minimum short = -32768
Overflow of an int does not generate a trap
Maximum int = 2147483647 (= 2**31-1)
Minimum int = -2147483648
Overflow of a long does not generate a trap
Maximum long = 2147483647 (= 2**31-1)
Minimum long = -2147483648
Maximum unsigned short = 65535
Maximum unsigned int = 4294967295
Maximum unsigned long = 4294967295
Alignments used for char=1 short=2 int=4 long=4
Character order:
    short: AB
    int:   ABCD
    long:  ABCD
Strings are not shared

PROPERTIES OF FLOAT:
Base = 2
Significant base digits = 24 (= at least 6 decimal digits)
Arithmetic rounds towards nearest
   Tie breaking rounds to even
Smallest x such that 1.0-base**x != 1.0 = -24
Small x such that 1.0-x != 1.0 = 5.96046448e-08
Smallest x such that 1.0+base**x != 1.0 = -23
Smallest x such that 1.0+x != 1.0 = 1.19209290e-07
Number of bits used for exponent = 8
Minimum normalised exponent = -125
Minimum normalised positive number = 1.17549435e-38
The smallest numbers are not kept normalised
Smallest unnormalised positive number = 1.40129846e-45
There is an 'infinite' value
Maximum exponent = 128
Maximum number = 3.40282347e+38
Arithmetic uses a hidden bit
It looks like single length IEEE format

PROPERTIES OF DOUBLE:
Base = 2
Significant base digits = 53 (= at least 15 decimal digits)
Arithmetic rounds towards nearest
   Tie breaking rounds to even
Smallest x such that 1.0-base**x != 1.0 = -53
Small x such that 1.0-x != 1.0 = 1.1102230246251565e-16
Smallest x such that 1.0+base**x != 1.0 = -52
Smallest x such that 1.0+x != 1.0 = 2.2204460492503131e-16
Number of bits used for exponent = 11
Minimum normalised exponent = -1021
Minimum normalised positive number = 2.2250738585072014e-308
The smallest numbers are not kept normalised
Smallest unnormalised positive number = 4.9406564584124654e-324
There is an 'infinite' value
Maximum exponent = 1024
Maximum number = 1.7976931348623157e+308
Arithmetic uses a hidden bit
It looks like double length IEEE format

Float expressions are evaluated in double precision
Double expressions are evaluated in double precision
Memory mallocatable ~= 18996 Kbytes
