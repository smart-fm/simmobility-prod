'''Implement decimal arithmetic on very large integers.

Example
=============================================================================
>>> import DecInt
>>> bigprime = DecInt.DecInt(2) ** 25964951 -1
>>> len(str(bigprime))
7816230
>>>

History
=============================================================================
DecInt was inspired by a Tim Peters posting on comp.lang.python of a minimal
BigDec class that was used to find the decimal representation of Mersenne
primes.

Performance
=============================================================================
Conversion to/from a decimal string in O(n) instead of O(n^2). This was the
primary motivation for developing DecInt.

For small- to medium-sized numbers, multiplication (kmult) uses a combination
of 2x2 (aka Karatsuba), 3x3, and 4x4 Toom-Cook multiplication algorithms.
The resulting algorithm is O(n^~1.4) compared to O(n^~1.585) for the
built-in multiplication. The kmult function is measurably faster than the
native Python multiplication when working with integers of approximately
200,000 digits.

For large numbers, Nussbaumer convolution (nussmult) is used for multiplica-
tion. The running time for Nussbaumer convolution is O(n ln(n)). The cross-
over point between Toom-Cook and Nussbaumer multiplication is approximately
300,000 digits.

Division uses two different algorithms. For a (relatively) small divisor,
David M. Smith's O(n^2) algorithm (smithdiv) is used. It quite efficient
since it doesn't require propagation of carries during intermediate steps.

For larger divisors, division uses a new algorithm (kdiv) that modifies David
M. Smith's O(n^2) algorithm with very fast intermediate steps. The running
time of this algorithm is O(n ln(n)^2).

For improved performance, GMPY is used when it is available.

Interestingly, for certain input values, the division algorithm here is
faster than GMP's division alogithm. The running time for GMP's division is
not smooth. There is a large increase in GMP's running time when divisor is
between 1/4 to just less than 1/2 the length of the dividend. If you choose
a worst-case divisor (just larger the 1/3 the length of the dividend), the
Python implementation can be more than 20x faster than GMP. At it's best,
GMP is about 4x faster than the Python+GMPY algorithm.

If GMPY is available, it used as the native underlying data type instead
of Python longs. The presence of GMPY will increase the performance of
DecInt by approximately 8x.

References
=============================================================================

1) Crandall, R. E. "Topics in Advanced Scientific Computation", Springer-
   Verlag
2) Hollerback, U. "Fast Multiplication & Divisior of Very Large Numbers."
   sci.math.research posting, Jan. 23, 1996.
3) Knuth, D. E. "The Art of Computer Programming, Vol. 2: Seminumerical
   Algorithms, 3rd ed.", Addison-Wesley
4) Smith, D. M. "A Multiple-Precision Division Algorithm", Mathematics of
   Computation (1996)

Release History
=============================================================================
Version 0.1
-----------
This was the first release.

Version 0.2
-----------
A bug fix release.

Version 0.3
-----------
Renamed to DecInt from BigDecimal to avoid confusion with the decimal module
included with Python.

Multiplication of very large numbers now uses Nussbaumer convolution.

Various performance tweaks.

Version 0.4
-----------
Converted to a single file.

Bug fixes.

Added examples: pell()

Requirements
=============================================================================
Python 2.4 or later
gmpy and psyco are used if present. gmpy 1.01 is required.

Todo
=============================================================================
 1) More examples.
 2) Faster gcd() and related functions.
 3) Allow use of an arbitrary radix. The underlying code in mpops.py
    should work with an arbitrary radix. I'm not sure of a reasonable
    format to use for the __str__ representation for an arbitrary radix.

License
=============================================================================

Copyright (c) 2004-2005, Case Van Horsen
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * The name of the author may not be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================

The Nussbaumer convolution code is based on software released under the
following license:

Copyright (c) 2005 Perfectly Scientific, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * The name of the author may not be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Contact
=============================================================================
casevh@comcast.net

'''

# Import random so we can create large integers with random digits.
import random
_rr = random.Random(42)

import operator
import math

try:
    # Try to use the gmpy module to get faster operations.
    import gmpy
    # gmpy 1.0.1 is required. Prior versions do not support the // operator.
    if float(gmpy.version()) > 1.0:
        mp = gmpy.mpz
        GMPY_Loaded = True
    else:
        mp = long
        GMPY_Loaded = False
except ImportError:
    mp = long
    GMPY_Loaded = False

# Try to use Psyco.
try:
    import psyco
    psyco.full()
    PSYCO_Loaded = True
except ImportError:
    PSYCO_Loaded = False

# Initialize some commonly used values.
ZERO = mp(0)
ONE = mp(1)
TWO = mp(2)
THREE = mp(3)
FOUR = mp(4)
FIVE = mp(5)
SIX = mp(6)
EIGHT = mp(8)
NINE = mp(9)
TWO7 = mp(27)
TWO4 = mp(24)
ONE20 = mp(120)
EIGHT1 = mp(81)
SEVEN29 = mp(729)
ONE6 = mp(16)
SIX4 = mp(64)
M_ONE = mp(-1)
M_TWO = mp(-2)
M_THREE = mp(-3)
M_FOUR = mp(-4)
M_EIGHT = mp(-8)
M_NINE = mp(-9)
M_ONE6 = mp(-16)
M_TWO7 = mp(-27)
M_EIGHT1 = mp(-81)

# Division is noticably faster when the length of quotient consists of a
# small number of large and small powers of 2. DIVFAST controls whether or
# not the number of digits per term is adjusted to make the length of the
# quotient optimal.
#
# DIV_CUTOFF defines the threshold above which optimization occurs. The
# value is the product of the number of terms in the divisor and quotient.
DIVFAST = True
DIV_CUTOFF = 1000 * 100

# Define constants for the Nussbaumer convolution algorithm.
CYC_CUTOFF = 8
NEG_CUTOFF = 8

# Define some helper functions that do element-wise operations on lists.
# The lengths of the two lists are assumed to be equal. The map() command
# is the fastest way to add or subtract two lists if you are NOT using psyco.
# If psyco is available, a plain old loop is fastest.

def _add_python(x, y):
    '''Return elementwise sum of two lists.'''
    return map(operator.add, x, y)

def _add_psyco(x, y):
    '''Return elementwise sum of two lists.'''
    res = [None] * len(x)
    for i in range(len(x)):
        res[i] = x[i] + y[i]
    return res

def _sub_python(x, y):
    '''Return elementwise difference of two lists.'''
    return map(operator.sub, x, y)

def _sub_psyco(x, y):
    '''Return elementwise difference of two lists.'''
    res = [None] * len(x)
    for i in range(len(x)):
        res[i] = x[i] - y[i]
    return res

def _minus_python(x):
    '''Return list with all elements negated.'''
    return map(operator.neg, x)

def _minus_psyco(x):
    '''Return list with all elements negated.'''
    res = [None] * len(x)
    for i in range(len(x)):
        res[i] = -x[i]
    return res

# Bind the most efficient function to the actual name.

if PSYCO_Loaded:
    add = _add_psyco
    sub = _sub_psyco
    minus = _minus_psyco
else:
    add = _add_python
    sub = _sub_python
    minus = _minus_python

def mul(n, xlst):
    '''Return elementwise multiplication of a list and a number.'''
    return [n * i for i in xlst]

def shiftright(n, xlst):
    '''Return list with all elements shifted right n bits.'''
    return [i >> n for i in xlst]

def shiftleft(n, xlst):
    '''Return list with all elements shifted left n bits.'''
    return [i << n for i in xlst]

# Optimize some frequent combinations.

def lcomb2(a, x, b, y):
    '''Return the linear combination ax + by.'''
    res = [None] * len(x)
    for i in range(len(x)):
        res[i] = a * x[i] + b * y[i]
    return res

def lcomb2a(x, b, y):
    '''Return the linear combination x + by.'''
    res = [None] * len(x)
    for i in range(len(x)):
        res[i] = x[i] + b * y[i]
    return res

def lcomb3(a, x, b, y, c, z):
    '''Return the linear combination ax + by + cz.'''
    res = [None] * len(x)
    for i in range(len(x)):
        res[i] = a * x[i] + b * y[i] + c * z[i]
    return res

def lcomb3a(x, b, y, c, z):
    '''Return the linear combination x + by + cz.'''
    res = [None] * len(x)
    for i in range(len(x)):
        res[i] = x[i] + b * y[i] + c * z[i]
    return res

def lcomb4a(x, b, y, c, z, d, t):
    '''Return the linear combination x + by + cz + dt.'''
    res = [None] * len(x)
    for i in range(len(x)):
        res[i] = x[i] + b * y[i] + c * z[i] + d * t[i]
    return res

# Define functions used to convert to/from a single number.

# To improve the speed of "tolong2" which repeatedly squares the radix
# value, we keep a cache of radix values and we only pass the base and power
# around.

# radix is a tuple that defines the radix used. The tuple contains base and
# power of the radix. For example, to work with groups of one hundred
# base-10 digits, use (100,10).

RadixCache = {}

def Radix2Value(radix):
    '''Return radix[1] ** radix[0], caching values for reuse.'''

    return RadixCache.setdefault(radix, mp(radix[1]) ** radix[0])

def tolong(x, radix):
    '''Converts an MP integer into a single number.

    You should really use tolong2, it is much faster.'''

    RADIX = Radix2Value(radix)
    res = ZERO
    for term in reversed(x):
        res = (res * RADIX) + term
    return res

def tolong2(x, radix):
    '''Fast conversion of MP integer into a single number.'''

    power, base = radix
    while len(x) > 1:
        temp = []
        for i in range(0, len(x), 2):
            temp.append(tolong(x[i:i+2], (power, base)))
        x = temp
        power *= 2
    return x[0]

def fromlong(x, radix):
    '''Converts a long into an MP integer.'''

    RADIX = Radix2Value(radix)
    if x == 0L:
        return [ZERO]
    else:
        res = []
        while x:
            x, temp = divmod(x, RADIX)
            res.append(temp)
        return res

def fromlong2(x, radix):
    '''Fast conversion of a long into an MP integer.'''

    # First estimate how many decimal digits there will be.
    try:
        # This works for gmpy.mpz, long, and ints.
        hexrep = hex(x)
        hexlen = len(hexrep)
        if hexrep.endswith('L'):
            hexlen -= 1
    except TypeError:
        raise TypeError, 'Error is conversion to hex().'
    declen = int(math.ceil(hexlen * math.log(16) / math.log(10)))

    # We will recursively divide x into approximately equal sized portions
    # until we get down to the length of an individual term.
    power, base = radix
    divlen = power
    powerlist = [(divlen, base)]
    while divlen < declen:
        divlen *= 2
        powerlist.append((divlen, base))

    temp = [x]
    for divval in reversed(powerlist):
        temp1 = []
        for bv in temp:
            temp1.extend(divmod(bv, Radix2Value(divval)))
        if temp1[0] == 0:
            temp = temp1[1:]
        else:
            temp = temp1
    temp.reverse()
    return temp

def smalldiv(lst, n, radix):
    '''Divide the elements of a list by a small number.

    Since all the uses require that the final remainder is zero, there is
    an assert test to verify that.

    Note: does not remove leading zeros from the result because kmult
    assumes the lengths of the list do not change.'''

    RADIX = Radix2Value(radix)
    res = [ZERO] * len(lst)
    rem = ZERO
    for indx in reversed(range(len(lst))):
        res[indx], rem = divmod(lst[indx] + rem * RADIX, n)
    assert rem == ZERO
    return res

def subsmalldiv(lsta, lstb, n, radix):
    '''Return (lsta - lstb) div n.

    Same as smalldiv(sub(lsta, lstb), n, radix).'''

    RADIX = Radix2Value(radix)
    res = [ZERO] * len(lsta)
    rem = ZERO
    for indx in reversed(range(len(lsta))):
        res[indx], rem = divmod(lsta[indx] - lstb[indx] + rem * RADIX, n)
    assert rem == ZERO
    return res

def addsmalldiv(lsta, lstb, n, radix):
    '''Return (lsta + lstb) div n.

    Same as smalldiv(add(lsta, lstb), n, radix).'''

    RADIX = Radix2Value(radix)
    res = [ZERO] * len(lsta)
    rem = ZERO
    for indx in reversed(range(len(lsta))):
        res[indx], rem = divmod(lsta[indx] + lstb[indx] + rem * RADIX, n)
    assert rem == ZERO
    return res

def smalldivmod(lst, n , radix):
    '''Divide elements of a list by a small number and return remainder.

    Just like smalldiv about except that it does remove leading zeros and
    it also returns the remainder.
    '''

    RADIX = Radix2Value(radix)
    res = [ZERO] * len(lst)
    rem = ZERO
    for indx in reversed(range(len(lst))):
        res[indx], rem = divmod(lst[indx] + rem * RADIX, n)
    while len(res) > 1 and res[-1] == ZERO:
        res.pop()
    return (res, rem)

# Begin Karatsuba and Toom-Cook multiplication here!!!

def kmul2x2s(x, y):
    '''Perform a 2x2 Toom-Cook (aka Karatsuba) multiplication.

    Multiply two 2-element lists using three multiplications and
    four additions.
    '''

    z = [ZERO] * 3
    x0, x1 = x
    y0, y1 = y
    z[0] = x0 * y0
    z[2] = x1 * y1
    z[1] = (x0 + x1) * (y0 + y1) - z[0] - z[2]
    return z

def kmul3x3s(x, y):
    '''Perform a 3x3 Toom-Cook multiplication of two 3-element lists.

    Multiply two 3-element lists using five multiplications, some
    additions and some divisions by a small value.
    '''

    z = [ZERO] * 5
    x0, x1, x2 = x
    y0 ,y1, y2 = y
    z[0] = x0 * y0
    z[4] = x2 * y2
    t1 = (x0 + x1 + x2) * (y0 + y1 + y2)
    t2 = (x0 - x1 + x2) * (y0 - y1 + y2)
    t3 = (x0 + (x1 << ONE) + (x2 << TWO)) * (y0 + (y1 << ONE) + (y2 << TWO))
    z[2] = ((t1 + t2) >> ONE) - z[0] - z[4]
    t4 = t3 - z[0] - (z[2] << TWO) - (z[4] << FOUR)
    z[3] = (t4 - t1 + t2) // SIX
    z[1] = ((t1 - t2) >> ONE) - z[3]
    return z

def kmul4x4s(x, y):
    '''Perform a 4x4 Toom-Cook multiplication of two 4-element lists.

    Multiply two 4-element lists using seven multiplications, some
    additions and some divisions by a small value.

    The code looks ugly, but it is just solving five equations with five
    unknowns.
    '''

    z = [ZERO] * 7
    x0, x1, x2, x3 = x
    y0, y1, y2, y3 = y
    z[0] = x0 * y0
    z[6] = x3 * y3
    t0 = z[0] + z[6]
    xeven = x0 + x2
    xodd = x1 + x3
    yeven = y0 + y2
    yodd = y1 + y3
    t1 = ((xeven + xodd) * (yeven + yodd)) - t0
    t2 = ((xeven - xodd) * (yeven - yodd)) - t0
    xeven = x0 + (x2 << TWO)
    xodd = (x1 << ONE) + (x3 << THREE)
    yeven = y0 + (y2 << TWO)
    yodd = (y1 << ONE) + (y3 << THREE)
    t0 = z[0] + (z[6] << SIX)
    t3 = (xeven + xodd) * (yeven + yodd) - t0
    t4 = (xeven - xodd) * (yeven - yodd) - t0
    t5 = ((x0 + THREE * x1 + NINE * x2 + TWO7 * x3) *
          (y0 + THREE * y1 + NINE * y2 + TWO7 * y3)) -\
          (z[0] + SEVEN29 * z[6])
    t6 = t1 + t2
    t7 = t3 + t4
    z[4] = (t7 - (t6 << TWO)) // TWO4
    z[2] = (t6 >> ONE) - z[4]
    t8 = t1 - z[2] - z[4]
    t9 = t3 - (z[2] << TWO) - (z[4] << FOUR)
    t10 = t5 - NINE * z[2] - EIGHT1 * z[4]
    t11 = t10 - THREE * t8
    t12 = t9 - (t8 << ONE)
    z[5] = (t11 - (t12 << TWO)) // ONE20
    z[3] = ((t12 << THREE) - t11) // TWO4
    z[1] = t8 - z[3] - z[5]
    return z

def kmul2x2l(x, y, radix):
    '''Do the recursion for the 2Nx2N Karatsuba multiply.
    '''

    ysplit = (len(y) + 1) // 2
    z = [ZERO] * (len(x) + len(y) - 1)
    x0 = x[: ysplit]
    x1 = x[ysplit :]
    y0 = y[: ysplit]
    y1 = y[ysplit :]
    z0 = kmult(x0, y0, radix)
    z2 = kmult(x1, y1, radix)
    z[2 * ysplit :] = z2
    x1.extend([ZERO] * (ysplit - len(x1)))
    y1.extend([ZERO] * (ysplit - len(y1)))
    z2.extend([ZERO] * (2*ysplit - 1 - len(z2)))
    t1 = kmult(add(x0, x1), add(y0, y1), radix)
    z1 = sub(sub(t1, z0), z2)
    z[: len(z0)] = z0
    while len(z1) > 1 and z1[-1] == ZERO:
        z1.pop()
    z[ysplit : ysplit + len(z1)] = add(z1, z[ysplit : ysplit + len(z1)])
    return z

def kmul8x8l(x, y, radix):
    '''Special case multiplication for 8x8 lists.'''

    z = [ZERO] * (15)
    x0, x1 = x[:4], x[4:]
    y0, y1 = y[:4], y[4:]
    z0 = kmul4x4s(x0, y0)
    z2 = kmul4x4s(x1, y1)
    z1 = kmul4x4s(add(x0, x1), add(y0, y1))
    z[8:] = z2
    z[:7] = z0
    for i, term in enumerate(z1):
        z[4 + i] += term - z0[i] - z2[i]
    return z

def kmul3x3l(x, y, radix):
    '''Do the recursion for the 3Nx3N Toom-Cook multiply.
    '''

    ysplit = (len(y) + 2) // 3
    y0 = y[: ysplit]
    y1 = y[ysplit : 2*ysplit]
    y2 = y[2*ysplit :]
    z = [ZERO] * (len(x) + len(y) - 1)
    x0 = x[: ysplit]
    x1 = x[ysplit : 2*ysplit]
    x2 = x[2*ysplit :]
    z0 = kmult(x0, y0, radix)
    z4 = kmult(x2, y2, radix)
    z[4*ysplit :] = z4
    x2.extend([ZERO] * (ysplit - len(x2)))
    y2.extend([ZERO] * (ysplit - len(y2)))
    z4.extend([ZERO] * (2*ysplit - len(z4) - 1))
    xeven = add(x0, x2)
    yeven = add(y0, y2)
    t1 = kmult(add(xeven, x1), add(yeven, y1), radix)
    t2 = kmult(sub(xeven, x1), sub(yeven, y1), radix)
    t3 = kmult(add(x0, add(add(x1, x1), mul(FOUR, x2))),
               add(y0, add(add(y1, y1), mul(FOUR, y2))), radix)
    z2 = sub(addsmalldiv(t1, t2, TWO, radix), add(z0, z4))
    t5 = sub(sub(t3, z0), add(mul(FOUR, z2), mul(ONE6, z4)))
    t6 = sub(t1, t2)
    z3 = subsmalldiv(t5, t6, SIX, radix)
    z1 = sub(smalldiv(t6, TWO, radix), z3)
    z[: len(z0)] = z0
    z[2 * ysplit : 4*ysplit - 1] = z2
    z[ysplit : 3*ysplit - 1] = add(z1, z[ysplit : 3*ysplit - 1])
    while len(z3) > 1 and z3[-1] == ZERO:
        z3.pop()
    z[3*ysplit : 3*ysplit + len(z3)] = add(z3, z[3*ysplit : 3*ysplit + len(z3)])
    return z

def kmul4x4l(x, y, radix):
    '''Do the recursion for the 4Nx4N Toom-Cook multiply.
    '''

    z = [ZERO] * (len(x) + len(y) - 1)
    ysplit = (len(y) + 3) // 4
    # Consider x and y as polynomials:
    # x(t) = x0 + x1*t + x2*t^2 + x3*t^3
    # y(t) = y0 + y1*t + y2*t^2 + y3*t^3
    y0 = y[:ysplit]
    y1 = y[ysplit:2*ysplit]
    y2 = y[2*ysplit:3*ysplit]
    y3 = y[3*ysplit:]
    x0 = x[:ysplit]
    x1 = x[ysplit:2*ysplit]
    x2 = x[2*ysplit:3*ysplit]
    x3 = x[3*ysplit:]
    # Compute z(t) = x(t) * y(t) at several values of t, then solve for the
    # coefficients of z(t). There are 7 unknowns, but two (z0 and z6) are
    # trivial to solve. This means we need to solve 5 equations with 5
    # unknowns.
    z0 = kmult(x0, y0, radix)
    z6 = kmult(x3, y3, radix)
    z[6*ysplit :] = z6
    # Need to fix the situation when the length of x and y is not exactly
    # divisible by 4. The algorithm would be a cleaner if the lengths were
    # ZERO-padded first, but this is a little faster.
    x3.extend([ZERO] * (ysplit - len(x3)))
    y3.extend([ZERO] * (ysplit - len(y3)))
    z6.extend([ZERO] * (2 * ysplit - 1 - len(z6)))
    # Compute z(1) & z(-1).
    xeven = add(x0, x2)
    xodd = add(x1, x3)
    yeven = add(y0, y2)
    yodd = add(y1, y3)
    tmp = add(z0, z6)
    t1 = sub(kmult(add(xeven, xodd), add(yeven, yodd), radix), tmp)
    t2 = sub(kmult(sub(xeven, xodd), sub(yeven, yodd), radix), tmp)
    # Compute z(2) & z(-2).
    xeven = lcomb2a(x0, FOUR, x2)
    xodd = lcomb2(TWO, x1, EIGHT, x3)
    yeven = lcomb2a(y0, FOUR, y2)
    yodd = lcomb2(TWO, y1, EIGHT, y3)
    t0 = lcomb2a(z0, SIX4, z6)
    t3 = subsmalldiv(kmult(add(xeven, xodd),
                           add(yeven, yodd), radix),
                     t0, TWO, radix)
    t4 = subsmalldiv(kmult(sub(xeven, xodd),
                           sub(yeven, yodd), radix),
                     t0, TWO, radix)
    # Compute z(-3).
    t5 = subsmalldiv(kmult(lcomb4a(x0, THREE, x1, NINE, x2, TWO7, x3),
                           lcomb4a(y0, THREE, y1, NINE, y2, TWO7, y3), radix),
                     lcomb2a(z0, SEVEN29, z6), THREE, radix)
    # Brute force solving for 5 unknowns from 5 equations.
    t6 = addsmalldiv(t1, t2, TWO, radix)
    t7 = addsmalldiv(t3, t4, FOUR, radix)
    z4 = subsmalldiv(t7, t6, THREE, radix)
    z2 = sub(t6, z4)
    t8 = lcomb3a(t1, M_ONE, z2, M_ONE, z4)
    t9 = lcomb3a(t3, M_TWO, z2, M_EIGHT, z4)
    t10 = lcomb3a(t5, M_THREE, z2, M_TWO7, z4)
    t11 = subsmalldiv(t10, t8, EIGHT, radix)
    t12 = subsmalldiv(t9, t8, THREE, radix)
    z5 = subsmalldiv(t11, t12, FIVE, radix)
    z3 = sub(add(t12, t12), t11)
    z1 = sub(t8, add(z3, z5))
    z[:len(z0)] = z0
    z[2*ysplit : 4*ysplit - 1] = z2
    z[4*ysplit : 6*ysplit - 1] = z4
    z[ysplit : 3*ysplit - 1] = add(z1, z[ysplit : 3*ysplit - 1])
    z[3*ysplit : 5*ysplit - 1] = add(z3, z[3*ysplit : 5*ysplit - 1])
    while len(z5) > 1 and z5[-1] == ZERO:
        z5.pop()
    z[5*ysplit : 5*ysplit + len(z5)] = add(z5, z[5*ysplit : 5*ysplit + len(z5)])
    return z

def kmult(x, y, radix):
    '''Perform recursive Toom-Cook multiplication on two lists of longs.

    kmult uses a combination of 2x2 (Karatsuba), 3x3, and 4x4 Toom-Cook
    multiplication algorithms.
    '''

    # Let x be the longest number.
    if len(x) < len(y):
        x, y = y, x

    xlen = len(x)
    ylen = len(y)

    # Begin by handling some of the simple special cases.
    if ylen == 1:
        return mul(y[0], x)

    if ylen == 2:
        if xlen == 2:
            return kmul2x2s(x, y)
        else:
            # Do 2xN multiplication.
            z = [ZERO]
            for ptr in range(0, xlen, 2):
                temp = kmult(x[ptr : ptr + 2], y, radix)
                z[-1] += temp[0]
                z.extend(temp[1:])
            return z

    if ylen == 3:
        if xlen == 3:
            return kmul3x3s(x, y)
        else:
            # Do 3xN multiplication.
            z = [ZERO] * 2
            for ptr in range(0, xlen, 3):
                temp = kmult(x[ptr : ptr + 3], y, radix)
                z[-2] += temp[0]
                z[-1] += temp[1]
                z.extend(temp[2:])
            return z

    if ylen == 4:
        if xlen == 4:
            return kmul4x4s(x, y)
        else:
            # Do 4xN multiplication.
            z = [ZERO] * 3
            for ptr in range(0, xlen, 4):
                temp = kmult(x[ptr : ptr + 4], y, radix)
                z[-3] += temp[0]
                z[-2] += temp[1]
                z[-1] += temp[2]
                z.extend(temp[3:])
            return z

    # All the special cases have been done. Now for the real work.
    if xlen == ylen:
        if xlen < 8:
            return kmul2x2l(x, y, radix)
        elif xlen == 8:
            return kmul8x8l(x, y, radix)
        elif xlen <= 12:
            return kmul3x3l(x, y, radix)
        else:
            return kmul4x4l(x, y, radix)
    else:
        z = []
        for ptr in range(0, xlen, ylen):
            temp = kmult(x[ptr : ptr + ylen], y, radix)
            z[ptr:] = add(temp[:len(z) - ptr], z[ptr:])
            z.extend(temp[len(z) - ptr:])
        return z

# End of Karatsuba and Toom-Cook multiplication.

# Begin Nussbaumer convolution code here!!!

# References:
#  1) "The Art of Computer Programming, Volume 2: Seminumerical Algorithms",
#     Donald Knuth, Section 4.6.4, Problem 59
#  2) "Topics in Advanced Scientific Computation", Richard E. Crandall
#  3) http://www.perfsci.com/freegoods.htm#fastalgorithms

def log2(n):
    '''Return int(log-base-2(n))'''

    res = -1
    while n:
        res += 1
        n >>= 1
    return res

def cyclit(x, y, radix):
    '''Return cyclic convolution of lists x and y.

    Uses Toom-Cook multiplication as defined above.'''

    assert len(x) == len(y)
    n = len(x)
    if n == 8:
        temp = kmul8x8l(x, y, radix)
    elif n == 4:
        temp = kmul4x4s(x, y)
    else:
        temp = kmult(x, y, radix)
    res = temp[:n]
    res[:n-1] = add(res[:n-1], temp[n:])
    return res

def neglit(x, y, radix):
    '''Return nega-cyclic convolution of lists x and y.

    Uses Toom-Cook multiplication as defined above.'''

    assert len(x) == len(y)
    n = len(x)
    if n == 8:
        temp = kmul8x8l(x, y, radix)
    elif n == 4:
        temp = kmul4x4s(x, y)
    else:
        temp = kmult(x, y, radix)
    res = temp[:n]
    res[:n-1] = sub(res[:n-1], temp[n:])
    return res

def makepoly(a, r, m):
    # Create the zero extended polynomials for Nussbaumer convolution.
    res = []
    for i in range(m):
        res.append(a[i::m])
    res.extend([ [ZERO] * r for i in range(m) ])
    return res

def unpoly(a, r, m):
    # Inverse of makepoly.
    res = []
    for i in range(r):
        for j in range(m):
            res.append(a[j][i])
    return res

def twist(aterm, r, q):
    q %= 2*r
    if q == 0:
        return aterm
    elif q < r:
        v = r - q
        return minus(aterm[v:]) + aterm[:v]
    else:
        q -= r
        v = r - q
        return aterm[v:] + minus(aterm[:v])

def neg(x, y, radix):
    '''Recursive negacyclic convolution.'''

    assert len(x) == len(y)
    n = len(x)
    if n <= NEG_CUTOFF:
        return neglit(x, y, radix)
    pow = log2(n)
    assert 2 ** pow == n
    m = 1 << (pow // 2)
    r = n // m
    w = r // m
    a = makepoly(x, r, m)
    b = makepoly(y, r, m)
    k = m
    # Do the forward FFT.
    while k > 0:
        v = w * m // k
        u = 0
        for j in range(k):
            i = j
            while i < 2 * m:
                s = i
                t = i + k
                as1 = a[s]
                at = a[t]
                bs = b[s]
                bt = b[t]
                for g in range(r):
                    as1[g], at[g] = as1[g] + at[g], as1[g] - at[g]
                    bs[g], bt[g] = bs[g] + bt[g], bs[g] - bt[g]
                a[t] = twist(a[t], r, u)
                b[t] = twist(b[t], r, u)
                i += 2 * k
            u += v
        k //= 2
    # Do the recursion!
    c = [ neg(a[i], b[i], radix) for i in range(2*m) ]
    # Do the DIT IFFT.
    k = 1
    while k < 2* m:
        v = -w * m // k
        u = 0
        for j in range(k):
            i = j
            while i < 2*m:
                s = i
                t = i + k
                c[t] = twist(c[t], r, u)
                ct = c[t]
                cs = c[s]
                for g in range(r):
                    cs[g], ct[g] = cs[g] + ct[g], cs[g] - ct[g]
                i += 2*k
            u += v
        k *= 2
    for k in range(m-1):
        s = k
        t = k + m
        c[t] = twist(c[t], r, 1)
        ct = c[t]
        cs = c[s]
        for g in range(r):
            cs[g] += ct[g]
    d = unpoly(c, r, m)
    return shiftright(mp(1 + pow // 2), d)

def neg_sqr(x, radix):
    '''Recursive negacyclic convolution squaring.'''

    n = len(x)
    if n <= NEG_CUTOFF:
        return neglit(x, x, radix)
    pow = log2(n)
    assert 2 ** pow == n
    m = 1 << (pow // 2)
    r = n // m
    w = r // m
    a = makepoly(x, r, m)
    k = m
    # Do the forward FFT.
    while k > 0:
        v = w * m // k
        u = 0
        for j in range(k):
            i = j
            while i < 2 * m:
                s = i
                t = i + k
                as1 = a[s]
                at = a[t]
                for g in range(r):
                    as1[g], at[g] = as1[g] + at[g], as1[g] - at[g]
                a[t] = twist(a[t], r, u)
                i += 2 * k
            u += v
        k //= 2
    # Do the recursion!
    c = [ neg_sqr(a[i], radix) for i in range(2*m) ]
    # Do the DIT IFFT.
    k = 1
    while k < 2* m:
        v = -w * m // k
        u = 0
        for j in range(k):
            i = j
            while i < 2*m:
                s = i
                t = i + k
                c[t] = twist(c[t], r, u)
                ct = c[t]
                cs = c[s]
                for g in range(r):
                    cs[g], ct[g] = cs[g] + ct[g], cs[g] - ct[g]
                i += 2*k
            u += v
        k *= 2
    for k in range(m-1):
        s = k
        t = k + m
        c[t] = twist(c[t], r, 1)
        ct = c[t]
        cs = c[s]
        for g in range(r):
            cs[g] += ct[g]
    d = unpoly(c, r, m)
    return shiftright(mp(1 + pow // 2), d)

def cyc(x, y, radix):
    '''Recursive cyclic convolution (Nussbaumer convolution).'''

    assert len(x) == len(y)
    n = len(x)
    assert 2 ** log2(n) == n
    nhalf = n >> 1
    if n <= CYC_CUTOFF:
        return cyclit(x, y, radix)
    rcyc = cyc(add(x[:nhalf], x[nhalf:]), add(y[:nhalf], y[nhalf:]), radix)
    rneg = neg(sub(x[:nhalf], x[nhalf:]), sub(y[:nhalf], y[nhalf:]), radix)
    return shiftright(1, add(rcyc, rneg) + sub(rcyc, rneg))

def cyc_sqr(x, radix):
    '''Recursive cyclic convolution squaring.'''

    n = len(x)
    assert 2 ** log2(n) == n
    nhalf = n >> 1
    if n <= CYC_CUTOFF:
        return cyclit(x, x, radix)
    xplus = add(x[:nhalf], x[nhalf:])
    xminus = sub(x[:nhalf], x[nhalf:])
    rcyc = cyc_sqr(xplus, radix)
    rneg = neg_sqr(xminus, radix)
    return shiftright(1, add(rcyc, rneg) + sub(rcyc, rneg))

def nussmult(x, y, radix):
    '''Multiply two lists of longs using the Nussbaumer convolution algorithm.

    Still need to add more intelligence about when to use kmult and how
    to handle x and y with radically different lengths. Currently, the shorter
    list is padded to the same length as the padded longer list.'''

    n = max(len(x), len(y))
    pow = log2(n)
    if 2 ** pow < n:
        pow += 1
    newn = 2 ** (pow + 1)
    z = cyc(x + ([ZERO] * (newn - len(x))),
            y + ([ZERO] * (newn - len(y))), radix)
    return z[:len(x) + len(y) - 1]

def nuss_square(x, radix):
    '''Square a list of longs using the Nussbaumer convolution algorithm.

    Still need to add more intelligence about when to use kmult and how
    to handle x and y with radically different lengths. Currently, the shorter
    list is padded to the same length as the padded longer list.'''

    n = len(x)
    pow = log2(n)
    if 2 ** pow < n:
        pow += 1
    newn = 2 ** (pow + 1)
    z = cyc_sqr(x + ([ZERO] * (newn - n)), radix)
    return z[:2 * n  - 1]

# End of Nussbaumer convolution.

# Begin division algorithms here!!!

def changedigits(x, oldradix, newradix):
    '''Change the internal representation to a different radix.

    Only the power of the radix can be changed. The base value cannot be
    changed.'''

    if newradix == oldradix:
        return x[:]
    # We can't change the base value.
    oldpower, oldbase  = oldradix
    newpower, newbase = newradix
    assert newbase == oldbase
    result = []
    NEWRADIX = Radix2Value(newradix)

    acc = ZERO
    acclen = 0
    for term in x:
        if acclen < newpower:
            acc += term * Radix2Value((acclen, oldbase))
            acclen += oldpower
        while acclen >= newpower:
            acc, newterm = divmod(acc, NEWRADIX)
            result.append(newterm)
            acclen -= newpower
    result.append(acc)
    # Remove any insignificant leading zeroes.
    while len(result) > 1 and result[-1] == ZERO:
        result.pop()
    return result

def divnorm(x, radix):
    '''Normalize the contents of an MP integer.

    It assumes the normalized result is positive.

    Note: Function modifies the input!
    '''

    RADIX = Radix2Value(radix)
    carry = ZERO
    for i, term in enumerate(x):
        carry, x[i] = divmod(carry + term, RADIX)
    assert carry >= ZERO
    while carry >= RADIX:
        carry, term = divmod(carry, RADIX)
        x.append(term)
    if carry:
        x.append(carry)
    # Strip insignificant leading 0 digits.
    while len(x) > 1 and x[-1] == ZERO:
        x.pop()

def ge(x,y):
    '''Returns true if list x greater than or equal to list y.

    The comparison is done from the last (most significant) end of the
    list down to the beginning of the list.

    They are assumed to be equal length.'''

    assert len(x) == len(y)
    for i in reversed(range(len(x))):
        if y[i] > x[i]:
            return False
        elif y[i] < x[i]:
            return True
    return True

def remainder_norm(rem, divisor, radix):
    '''Normalizes the remainder after division.

    Returns the normalized remainder and a count of multiples of
    the divisor that needed to be added or subtracted from the
    unnormalized remainder.

    Warning: The algorithm assumes that divisor is properly normalized.
    Specifically, the normalized length of the remainder must be less than
    or equal to the length of the divisor.

    Note: The adjustment 'count' is almost always 0 or less than 10. But
    sometimes it can be quite large. Large values of 'count' are more
    frequent when digits is small.

    The following test cases will test both the "while carry != ZERO" suite
    and the "while ge(x, divisor)" suite.

    !!! Need to fix this. The following example is old.
    DIVFAST = True
    DIV_CUTOFF = 0
    kdiv([37] * 194, [29] * 76, 2)
    kdiv([37] * 194, [29] * 77, 2)
    !!! This is more likely correct.
    DIVFAST = False
    DIV_CUTOFF = 0
    x = [37] * 194
    y = [29] * 76
    change digits from 2 to 3
    kdiv(x, y, (3,10))
    etc.
    '''

    # Normalize the remainder and make sure it is the same length as the
    # divisor.
    RADIX = Radix2Value(radix)
    x = rem[:]
    x.extend([ZERO] * (len(divisor) - len(x)))
    count = 0
    carry = ZERO
    for i, term in enumerate(x):
        carry, x[i] = divmod(carry + term, RADIX)
    x[-1] += (carry * RADIX)

    # This will adjust the remainder to be >= 0 and no carry past the last
    # term.
    while carry != 0:
        scale = tolong(x[-2:], radix) // tolong(divisor[-2:], radix)
        count -= scale
        x = lcomb2a(x, -scale, divisor)
        carry = ZERO
        for i, term in enumerate(x):
            carry, x[i] = divmod(carry + term, RADIX)
        x[-1] += (carry * RADIX)

    # Here is where we handle a too large remainder that doesn't overflow
    # into the carry.
    if ge(x, divisor):
        scale = tolong(x[-2:], radix) // tolong(divisor[-2:], radix)
        count -= scale
        x = lcomb2a(x, -scale, divisor)
        carry = ZERO
        for i, term in enumerate(x):
            carry, x[i] = divmod(carry + term, RADIX)
        x[-1] += (carry * RADIX)
        # The correction factor above should almost always be right, but
        # it may still be off a little bit. So do the slow, but always
        # valid correction.
        while carry < ZERO:
            count += 1
            x = add(x, divisor)
            carry = ZERO
            for i, term in enumerate(x):
                carry, x[i] = divmod(carry + term, RADIX)
            x[-1] += (carry * RADIX)

        while ge(x, divisor):
            count -= 1
            x = sub(x, divisor)
            carry = ZERO
            for i, term in enumerate(x):
                carry, x[i] = divmod(carry + term, RADIX)
            x[-1] += (carry * RADIX)

    # Strip insignificant leading 0 digits.
    while len(x) > 1 and x[-1] == ZERO:
        x.pop()
    return (count, x)

def smithdiv(x, y, radix):
    '''Perform division using the efficient algorithm by David Smith.

    This algorithm is still O(n^2) but is quite efficient since it doesn't
    require intermediate normalization or correction of slightly wrong
    guesses for the next quotient digit.

    kdiv is much faster for large values of n. However, smithdiv is faster
    when y is much smaller than x.

    Return quotient (x/y) and remainder (x % y).'''

    RADIX = Radix2Value(radix)
    if len(y) > len(x):
        return ([ZERO], x)
    elif len(y) == len(x) == 1:
        # Just use the builtin divmod routine.
        q, r = divmod(x[0], y[0])
        return ([q], [r])
    elif len(y) <= 2:
        q, r = smalldivmod(x, tolong(y, radix), radix)
        return (q, fromlong(r, radix))
    else:
        # Work by taking 2 at a time. If you want to use a very small
        # radix (10, or 100), the quotient guesses will be more accurate
        # if you change tsize to 4.
        if RADIX <= 100:
            temp = 4
        elif RADIX <= 10000:
            temp = 3
        else:
            temp = 2
        tsize = min(temp, len(y) - 1)
        r = x[:]
        q = []
        yguess = tolong(y[-tsize:], radix)
        for ptr in range(len(x) - tsize, len(y) - tsize - 1, -1):
            xguess = tolong(r[ptr:ptr + tsize], radix)
            # Can't use // because mpz doesn't support it.
            # Fixed in gmpy 1.0.1. Will remove this workaround soon.
            qdigit = xguess // yguess
            q.append(qdigit)
            start = ptr + tsize - len(y)
            r[start:start + len(y)] = sub(r[start:start + len(y)], mul(qdigit, y))
            # Normalize the leading digits of the quotient.
            r[-2] += (RADIX * r[-1])
            r.pop()
        q.reverse()
        borrow, r = remainder_norm(r, y, radix)
        q[0] -= borrow
        divnorm(q, radix)
        return (q, r)

def first2power(n):
    '''Returns the first (highest) power of 2 in a number.'''

    res = 1
    while n > 1 and n % 2 == 0:
        n //= 2
        res *= 2
    return res

def all2power(n):
    '''Returns a list containing all the powers of two in a number.
    '''
    res = []
    power = 1
    while n:
        n, parity = divmod(n, 2)
        if parity:
            res.append(power)
        power *= 2
    return res

def calcqlen(xdig, ydig, dgts):
    '''Calculate the number of terms in the quotient.
    '''
    return ((xdig + dgts - 1) // dgts) - ((ydig + dgts - 1) // dgts) + 1

def divoptall(xdig, ydig, oldradix):
    '''Calculates possible quotient lengths.

    Given values for the length of x, y, and the initial number of digits
    per term, return all the possible values for the quotient length that
    can be made by changed the number of digits per term.

    The function "divopt" uses this list to return the best value for
    digits.'''

    # ndmin is the lower bound.
    ndmin = int(1 * oldradix)

    # ndmax is the upper bound. 2 used to be 1.5.
    ndmax = int(2 * oldradix) + 1

    ndset = {}
    for dgts in range(ndmin, ndmax):
        # Drop small powers of 2, < 2^5
        ndset[32 * (calcqlen(xdig, ydig, dgts) // 32)] = dgts
    qlens = []
    for nqlen, dgts in ndset.iteritems():
        invsum = 0.0
        plist = [pwr for pwr in all2power(nqlen) if pwr >3]
        plist.reverse()
        plist = tuple(plist)
        for p in plist:
            invsum += 1.0 / p
        qlens.append((invsum, dgts, nqlen, plist))
    # Choose the best quotient length as the one with the smallest sum of
    # the inverse of the powers.
    qlens.sort()
    return qlens

def divopt(xdig, ydig, oldradix):
    '''Chooses the optimal value for rescaling a division operation.

    This isn't guaranteed to be optimal. Emperically, it seems to work
    well.

    Division seems to be optimal when the length of the quotient consists
    of two large, consectutive powers of two. divoptall sorts the possible
    quotient lengths so that a large single power of two is first in the list.

    divopt tries to find quotient lengths that consist of 2 or 3 large
    powers of two. If it can't find one, it just uses the first entry in the
    list created by divoptall.'''

    qlist = divoptall(xdig, ydig, oldradix)
    newtemp = [(t[3], t[1]) for t in qlist]
    scratch = dict(newtemp)
    try:
        mx2p = newtemp[0][0][0]
    except IndexError:
        return oldradix
    if scratch.has_key((mx2p//2, mx2p//4)):
        temp = scratch[(mx2p//2, mx2p//4)]
    elif scratch.has_key((mx2p//2, mx2p//4, mx2p//8)):
        temp = scratch[(mx2p//2, mx2p//4, mx2p//8)]
    elif scratch.has_key((mx2p, mx2p//2)):
        temp = scratch[(mx2p, mx2p//2)]
    else:
        try:
            temp = qlist[0][1]
        except IndexError:
            temp = oldradix
    # Should add some logic to compare newradix to oldradix and see
    # if the change is worth it.
    return temp

def kdiv(x, y, radix):
    '''Fast extended precision division returning quotient and remainder.

    Algorithm (overview)
    Step 0)
      Handle all the special cases.
    Step 1)
      Adjust the number of digits per term to optimize the quotient length.
    Step 2)
      Calculate the first quotient_length % 8 terms of the quotient,
      multiply times the divisor, and subtract. The rest of the algorithm
      can now work in blocks of 8 quotient terms.
    Step 3)
      Calculate the next 8 quotient terms, then multiply the 8 quotient
      terms times the first 16 terms of the divisor. Extract 8 terms and
      subtract them. Using kmult, we get the 8 terms for 42 multiplies
      instead of the 64 multiplies for the straightforward approach.
    Step 4)
      Calculate the next 16 quotient terms, then multiply the 16 quotient
      terms times the first 32 terms of the divisor. Extract 16 terms and
      subtract them. Using kmult, we get the 16 terms for 98 multiplies
      instead of the 256 multiplies for the straightforward approach.
    Step 5)
      Compute the next 8, 32, 8, 16, 8, 64, 8, 16, 8, 32, 8, 16, 8, 128, ...
      terms. When the number of partial quotient terms equals the lowest
      power of two in the total remaining quotient digits, perform a full
      multiply/subtract.

    The devil is in the details.
    '''

    if len(y) > len(x):
        return ([ZERO], x)

    if len(y) == len(x) == 1:
        # Just use the builtin divmod routine.
        q, r = divmod(x[0], y[0])
        return ([q], [r])

    if len(y) <= 2:
        q, r = smalldivmod(x, tolong(y, radix), radix)
        return (q, fromlong(r, radix))

    RADIX = Radix2Value(radix)
    POWER, BASE = radix
    DIV_SCALED = False

    # Don't bother to optimise division unless the numbers are really big.
    # These parameters may need to be tweaked.
    if DIVFAST and \
            len(y) > 128 and \
            (len(x) - len(y)) > 128 and \
            (len(y) * (len(x) - len(y))) > DIV_CUTOFF:
        xdigits = POWER * (len(x) - 1) + int(math.log(long(x[-1]), BASE)) + 1
        ydigits = POWER * (len(y) - 1) + int(math.log(long(y[-1]), BASE)) + 1
        newradix = (divopt(xdigits, ydigits, POWER), BASE)
        r = changedigits(x, radix, newradix)
        ynew = changedigits(y, radix, newradix)
        DIV_SCALED = True
        RADIX = Radix2Value(newradix)
    else:
        ynew = y
        r = x[:]
        newradix = radix

    # Compute just enough of the quotient so that the rest of the code
    # can assume that the remainder of quotient is a multiple of 8.

    qleft = len(r) - len(ynew) + 1
    qmain, qpartial = divmod(qleft, 8)
    if qpartial:
        powerlist = [qpartial] + all2power(qmain * 8)
        stepsize = qpartial
    else:
        powerlist = all2power(qmain * 8)
        stepsize = 8
    q = []
    yguess = tolong(ynew[-2:], newradix)
    qcounter = 0
    while qleft:
        qleft -= stepsize
        qcounter += stepsize
        for i in range(1, stepsize + 1):
            qdigit = tolong(r[-2:], newradix) // yguess
            q.append(qdigit)
            n = stepsize + 2 - i
            temp = mul(qdigit, ynew[-min(len(ynew), n):])
            m = min(n, len(temp))
            r[-m:] = sub(r[-m:], temp[-m:])
            r[-2] += (RADIX * r[-1])
            r.pop()
        # Now to complete the multiply/subtract.
        if stepsize < 8:
            bsize = qpartial
            stepsize = 8
        else:
            bsize = first2power(qcounter)
        qtemp = q[-bsize:]
        qtemp.reverse()
        if bsize == powerlist[0]:
            # Do a full multiply/subtract.
            tsub = fastmult(qtemp, ynew, newradix)[:-bsize-1]
            r[-len(tsub)-1: -1] = sub(r[-len(tsub)-1: -1], tsub)
            qcounter = 0
            powerlist = powerlist[1:]
        else:
            tsub = fastmult(qtemp, ynew[-(2*bsize+1):-1], newradix)[-2*bsize : -bsize]
            r[-len(tsub)-1: -1] = sub(r[-len(tsub)-1: -1], tsub)

    q.reverse()
    borrow, r = remainder_norm(r, ynew, newradix)
    q[0] -= borrow
    divnorm(q, newradix)
    if DIV_SCALED:
        r = changedigits(r, newradix, radix)
        q = changedigits(q, newradix, radix)
    return (q, r)

# Reasonably standard entry points. These are the only functions whose
# API is not expected to change.

def fastmult(x, y, radix):
    '''Multiply two lists of longs using the fasted method.

    This function will invoke Toom-Cook multiplication for small operands and
    Nussbaumer convolution for larger operands.'''

    # x is the longer number.
    if len(x) < len(y):
        x, y = y, x
    xlen = len(x)
    ylen = len(y)
    # Threshold below which everything is just sent to kmult.
    if GMPY_Loaded:
        threshold = 50
    else:
        threshold = 100

    if (ylen <= threshold):
        return kmult(x, y, radix)

    # newn is the padded length of the numbers that would be sent
    # to the Nussbaumer cyclic convolution algorithm. Also handle the
    # special case 2N x N and N=2^k since that is a common case
    # for division.
    pow = log2(xlen)
    if (xlen == 2 ** pow) and (2 * ylen == xlen):
        z = nussmult(x[:ylen], y, radix)
        temp = nussmult(x[ylen:], y, radix)
        z[ylen:] = add(temp[:ylen - 1], z[ylen:])
        z.extend(temp[ylen - 1:])
        return z
    elif 2 ** pow < xlen:
        pow += 1
    newn = 2 ** (pow + 1)

    # If x and y are similar in length, just call Nussbaumer directly.
    if ylen > (newn >> 2):
        z = nussmult(x, y, radix)
        return z[:len(x) + len(y) - 1]

    # Now multiply by using the smaller number and portions of the larger
    # number. The portions of the larger number will be smallest power of
    # 2 greater than or equal to the length of the original smaller number.
    plen = log2(ylen)
    if 2 ** plen < ylen:
        plen += 1
    portion = 2 ** plen
    z = []
    for ptr in range(0, xlen, portion):
        temp = fastmult(x[ptr : ptr + portion], y, radix)
        z[ptr:] = add(temp[:len(z) - ptr], z[ptr:])
        z.extend(temp[len(z) - ptr:])
    return z

def fastsquare(x, radix):
    '''Square a list of longs using the fastest method.'''

    if len(x) <= 32:
        return kmult(x, x, radix)
    else:
        return nuss_square(x, radix)

def fastdiv(x, y, radix):
    '''Return divmod(x,y) by automatic choice of optimal method.'''
    if len(y) > len(x):
        return ([ZERO], x)

    # smithdiv is more efficient when len(y) is significantly less than
    # len(x). The following works well with gmpy and 1000 digits per term.

    threshold = 1.4 * (math.log(len(x)) ** 2)
    if len(y) < threshold:
        return smithdiv(x, y, radix)
    else:
        return kdiv(x, y, radix)

class DecInt(object):
    '''Implements decimal arithmetic on very large integers.'''

    def __init__(self, initval = None, power = None):
        '''Initialize an instance of DecInt.

        Input:
        initval - The initial value of an integer; can either a string or an
                  integer.
        power   - The number of decimal digits to store per term.

        Internal Structure:
        terms   - A list of positive integers.
        radix   - A tuple containing the power and base of the radix. A value
                  of (100,10) implies DecInt is working with terms of one
                  hundred decimal digits.
        sign    - +1 -> positive number
                - -1 -> negative number
                -  0 -> NaN; raises an exception if used as an operand
        hashval - The cached value of the hash value; None if hash value
                  has not been calculated.
        longval - The cached value of the single number equivalent; this
                  will have the same type as mp.
        '''

        if power:
            try:
                self.radix = (max(2, abs(int(power))), 10)
            except ValueError, TypeError:
                raise ValueError, 'Illegal value for radix.'
        else:
            # Pick reasonable defaults for working with million digit
            # numbers.
            if GMPY_Loaded:
                # 1000 decimal digits per term works well with GMP.
                self.radix = (1000, 10)
            else:
                # 250 decimal digits per term seems to be reasonable for a
                # pure Python implementation.
                self.radix = (250, 10)
        power = self.radix[0]

        self.hashval = None
        self.longval = None

        # Define the types of objects that are considered integers that can
        # be used for input.
        if GMPY_Loaded:
            int_types = (int, long, type(gmpy.mpz(1)))
        else:
            int_types = (int, long)

        if isinstance(initval, DecInt):
            self.terms = initval.terms[:]
            self.sign = initval.sign
            # Remember the value of radix used to create the new instance.
            # We force the new instance to use the radix of the new instance.
            temp = self.radix
            self.radix = initval.radix
            self.ChangeRadix(temp)
        elif isinstance(initval, int_types):
            if initval < 0:
                self.sign = -1
                initval = abs(initval)
            else:
                self.sign = 1
            self.terms = fromlong2(mp(initval), self.radix)
        elif initval == None:
            # For consistency with normal Python behaviour, this should
            # return 0. Currently, it returns NaN.
            self.terms = []
            self.sign = 0
        elif isinstance(initval, str):
            if initval == 'NaN':
                self.terms = []
                self.sign = 0
            else:
                # Get rid of all leading and trailing whitespace.
                initval = initval.strip()
                # Check for empty string.
                if initval == '':
                    raise ValueError, 'Error in conversion to DecInt'
                # Check for a negative number.
                if initval[0] == '-':
                    self.sign = -1
                    initval = initval[1:]
                    if initval.strip() != initval:
                        raise ValueError, 'Error in conversion to DecInt'
                # Remove the optional + sign for a positive number.
                elif initval[0] == '+':
                    self.sign = 1
                    initval = initval[1:]
                    if initval.strip() != initval:
                        raise ValueError, 'Error in conversion to DecInt'
                else:
                    # Just assume it is positive.
                    self.sign = 1
                # Do the actual conversion.
                # This may raise an error if invalid data is in the string.
                try:
                    self.terms = [mp(initval[max(0, i - power):i])
                                  for i in xrange(len(initval), 0, - power)]
                except ValueError:
                    self.terms = []
                    self.sign = 0
                    raise ValueError, 'Error in conversion to DecInt'
                # Filter out leading zeros that may have been passed in a
                # string and don't create -0.
                while len(self.terms) >1 and self.terms[-1] == ZERO:
                    self.terms.pop()
                if self.terms == [ZERO]:
                    self.sign = 1

    def _coerce_vars(f):
        '''Decorater to convert int and long to DecInt.'''

        def new_f(*args):
            new_args = []
            for var in args:
                if isinstance(var, (int, long)):
                    new_args.append(DecInt(var))
                else:
                    new_args.append(var)
            new_f.func_name = f.func_name
            new_f.func_doc = f.func_doc
            return f(*tuple(new_args))
        return new_f

    def _validate_vars(f):
        '''Decorator to validate internal consistency.

        Force all instances of DecInt to use the same internal
        representation.

        Additional validity checks may be adding the future.'''

        def new_f(*args):
            req_radix = args[0].radix
            for var in args:
                if isinstance(var, DecInt) and var.sign == 0:
                    raise ValueError, 'DecInt("NaN") used as an operand.'
                if isinstance(var, DecInt) and var.radix != req_radix:
                    var.ChangeRadix(req_radix)
            new_f.func_name = f.func_name
            new_f.func_doc = f.func_doc
            return f(*args)
        return new_f

    def __copy__(self):
        '''Return a copy of an instance of DecInt.'''

        result = DecInt()
        result.sign = self.sign
        result.hashval = self.hashval
        result.longval = self.longval
        result.radix = self.radix
        result.terms = self.terms[:]
        return result

    def __str__(self):
        '''Create the generic string representation.'''

        if self.sign == 0:
            return 'NaN'
        power, base = self.radix
        if base != 10:
            raise ValueError, 'Cannot display non base-10 numbers.'
        temp = [str(i).zfill(power) for i in self.terms]
        temp[-1] = temp[-1].lstrip('0')
        if temp[-1] == '':
            if len(temp) == 1:
                temp[0] = '0'
            else:
                raise ValueError, 'Error in conversion from DecInt'
        temp.reverse()
        if self.sign == -1:
            temp[0] = '-' + temp[0]
        return ''.join(temp)

    def __repr__(self):
        '''Create the canonical string representation.

        To minimize memory usage, this function duplicates much of the code
        from __str__.'''

        if self.sign == 0:
            return 'DecInt("NaN")'
        power, base = self.radix
        temp = ['")']
        temp.extend([str(i).zfill(power) for i in self.terms])
        temp[-1] = temp[-1].lstrip('0')
        if temp[-1] == '':
            if len(temp) == 2:
                temp[1] = '0'
            else:
                raise ValueError, 'Error in conversion from DecInt'
        temp.append('DecInt("')
        temp.reverse()
        if self.sign == -1:
            temp[1] = '-' + temp[1]
        return ''.join(temp)

    def Random(self, randlength):
        '''Return a random integer with randlength digits.

        The actual number of digits may be less than randlength because
        leading zeroes are stripped from the result.'''

        result = DecInt()
        result.radix = self.radix
        result.sign = 1
        result.terms = []
        power, base = self.radix

        full, partial = divmod(randlength, power)
        for i in range(full):
            result.terms.append(mp(_rr.randrange(long(Radix2Value(self.radix)))))
        result.terms.append(mp(_rr.randrange(long(Radix2Value((partial, base))))))
        while len(result.terms) > 1 and result.terms[-1] == ZERO:
            result.terms.pop()
        return result

    def _FastRandom(self, randlength):
        '''Return a (sort of) random integer with randlength digits.

        This function is intended to create numbers for testing so speed
        of creation is more important than randomness.

        Do not use. It may be removed at any time.'''

        result = DecInt()
        result.radix = self.radix
        temp = [mp(_rr.randrange(long(Radix2Value(self.radix)))) for i in range(10)]
        result.sign = 1
        result.terms = []
        power, base = self.radix

        full, partial = divmod(randlength, power)
        for i in range(full):
            result.terms.append(temp[_rr.randrange(10) % 10])
        result.terms.append(mp(_rr.randrange(long(Radix2Value((partial, base))))))
        while len(result.terms) > 1 and result.terms[-1] == ZERO:
            result.terms.pop()
        return result

    @_validate_vars
    def ChangeRadix(self, newradix):
        '''Change the internal representation to a different radix.

        This actually changes the internal representation. It does not
        create a new instance. Hashing must not care about the internal
        representation so it shouldn't be a problem.'''

        if newradix == self.radix:
            return
        self.terms = changedigits(self.terms, self.radix, newradix)
        self.radix = newradix
        return

    @_validate_vars
    def __abs__(self):
        '''Make the number positive.'''

        result = self.__copy__()
        result.sign = abs(result.sign)
        return result

    @_validate_vars
    def __neg__(self):
        '''Make the number negative.'''

        result = self.__copy__()
        # Don't create a -0.
        if result.terms != [ZERO]:
            result.sign = -1 * result.sign
        return result

    @_validate_vars
    def __pos__(self):
        '''Return an unchanged copy of the number.'''

        return self.__copy__()

    @_validate_vars
    def __nonzero__(self):
        '''Return True if the number is +0 or -0.'''

        if (len(self.terms) == 1) and (self.terms[0] == 0):
            return False
        else:
            return True

    @_validate_vars
    @_coerce_vars
    def __eq__(self, other):
        '''Return True the numbers are equal.'''

        # Make sure 0 equals -0.
        if 1 == len(self.terms) == len(other.terms) \
            and (0 == self.terms[0] == other.terms[0]):
            return True
        if self.sign != other.sign:
            return False
        if len(self.terms) != len(other.terms):
            return False
        for sterm, oterm in zip(self.terms, other.terms):
            if sterm != oterm:
                return False
        return True

    def __ne__(self, other):
        '''Return True if the numbers are not equal.'''

        return not __eq__(self, other)

    @_validate_vars
    @_coerce_vars
    def __lt__(self, other):
        '''Return True if self is less than other.'''

        # Check if self is less than other.
        if not self.__nonzero__():
            if not other.__nonzero__():
                return False
            elif other.sign == -1:
                return False
            else:
                return True
        if not other.__nonzero__():
            # The case when self is zero was handled earlier so just skip it now.
            if self.sign == -1:
                return True
            else:
                return False
        # Now we've handled the troublesome case when either self or other is +-0.
        if self.sign == 1:
            if other.sign == -1:
                return False
            else:
                # Verify that both self and other are positive.
                assert self.sign == other.sign == 1
                if len(self.terms) < len(other.terms):
                    return True
                elif len(self.terms) > len(other.terms):
                    return False
                else:
                    # Now we have to compare term-by-term.
                    for sterm, oterm in reversed(zip(self.terms, other.terms)):
                        if sterm < oterm:
                            return True
                        elif sterm > oterm:
                            return False
                    return True
        else:
            assert self.sign == -1
            if other.sign == 1:
                return True
            else:
                assert self.sign == other.sign == -1
                if len(self.terms) < len(other.terms):
                    return False
                elif len(self.terms) > len(other.terms):
                    return True
                else:
                    for sterm, oterm in reversed(zip(self.terms, other.terms)):
                        if sterm < oterm:
                            return False
                        elif sterm > oterm:
                            return True
                    return False

    @_validate_vars
    @_coerce_vars
    def __le__(self, other):
        '''Return True if self is less than or equal to other.'''

        return self.__lt__(other) or self.__eq__(other)

    @_validate_vars
    @_coerce_vars
    def __gt__(self, other):
        '''Return True if self is greater than other.'''

        return other.__lt__(self)

    @_validate_vars
    @_coerce_vars
    def __ge__(self, other):
        '''Return True if self is greater than or equal to other.'''

        return other.__le__(self)

    def _normalize(self):
        '''Normalize the contents of the terms composing the number.

        Warning: This will mutate the contents of self!

        The decorator _validate_vars should not be used with this function
        because it may call _normalize to force consistency on input
        to some other function.

        You should never need to use this!
        '''

        if self.sign == 0:
            raise ValueError, 'Cannot normalize an NaN.'
        RADIX = Radix2Value(self.radix)
        carry = 0
        for i, term in enumerate(self.terms):
            carry, self.terms[i] = divmod(carry + term, RADIX)
        while abs(carry) >= RADIX:
            carry, term = divmod(carry, RADIX)
            self.terms.append(term)
        if carry:
            self.terms.append(carry)
        # Strip insignificant leading 0 digits.
        while len(self.terms) > 1 and self.terms[-1] == ZERO:
            self.terms.pop()
        # Handle a sign change.
        if self.terms[-1] < ZERO:
            self.sign = -1 * self.sign
            radixm1 = RADIX - 1
            borrow = 1
            for i, term in enumerate(self.terms):
                borrow, self.terms[i] = divmod(radixm1 + borrow - term, RADIX)
            while len(self.terms) > 1 and self.terms[-1] == ZERO:
                self.terms.pop()

    @_validate_vars
    @_coerce_vars
    def __add__(self, other):
        '''Add two instances of DecInt.'''

        # Force self to have the larger number of terms.
        if len(self.terms) < len(other.terms):
            self, other = other, self
        result = DecInt()
        if self.sign == other.sign:
            result.terms = add(self.terms[:len(other.terms)], other.terms)
        else:
            result.terms = sub(self.terms[:len(other.terms)], other.terms)
        result.terms += self.terms[len(other.terms):]
        result.sign = self.sign
        result.radix = self.radix
        result._normalize()
        # Don't create a -0 result.
        if not result.__nonzero__():
            result.sign = 1
        return result

    __radd__ = __add__

    @_validate_vars
    @_coerce_vars
    def __sub__(self, other):
        '''Subtract two instances of DecInt.'''

        other.sign = -other.sign
        result = self.__add__(other)
        other.sign = -other.sign
        return result

    @_validate_vars
    @_coerce_vars
    def __rsub__(self, other):
        '''Subtract with reversed operands.'''

        return other.__sub__(self)

    @_validate_vars
    @_coerce_vars
    def __mul__(self, other):
        '''Multiply two instances of DecInt.

        This is just a wrapper for the actual multiply code located in
        the mpops module. Basically, it is a 4-way Toom-Cook
        algorithm.
        '''

        # Force self to have the larger number of terms.
        if len(self.terms) < len(other.terms):
            self, other = other, self
        result = DecInt()
        result.terms = fastmult(self.terms, other.terms, self.radix)
        result.radix = self.radix
        result.sign = self.sign * other.sign
        result._normalize()
        if not result.__nonzero__():
            result.sign = 1
        return result

    __rmul__ = __mul__

    @_validate_vars
    @_coerce_vars
    def square(self):
        '''Square an instance of DecInt.

        '''

        result = DecInt()
        result.terms = fastsquare(self.terms, self.radix)
        result.radix = self.radix
        result.sign = 1
        result._normalize()
        if not result.__nonzero__():
            result.sign = 1
        return result

    def __pow__(self, n, modulo = None):
        '''Calculate x^n or x^n mod(z).

        n is assumed to be a positive interger, not a DecInt.

        This code is based a posting to c.l.py by Tim Peters.
        '''

        # Because this function has a unique signature, the input error
        # checking is done here instead of using decorators.

        # "The usual" left-to-right efficient exponentiation algorithm.
        # n must be vanilla integer, not DecInt.

        # Validate the exponent.
        if isinstance(n, DecInt):
            n = long(n)
        if not isinstance(n, (int, long)):
            raise TypeError, 'Exponent must be a positive integer.'
        if n <= 0:
            raise TypeError, 'Exponent must be a positive integer.'

        # Validate the modulo.
        if modulo and isinstance(modulo, (int, long, str)):
            modulo = DecInt(modulo)
        if modulo and not isinstance(modulo, DecInt):
            raise TypeError, 'Modulo must be a number.'

        mask = 1L
        while mask <= n:
            mask <<= 1
        mask >>= 1  # position of n's most-significant bit
        power, base = self.radix
        result = DecInt(1, power)
        if modulo:
            while mask:
                result = result.square()
                if n & mask:
                    result *= self
                result = result.__divmod__(modulo)[1]
                mask >>= 1
        else:
            while mask:
                result = result.square()
                if n & mask:
                    result *= self
                mask >>= 1
        return result

    @_validate_vars
    @_coerce_vars
    def __divmod__(self, other):
        '''Return the quotient and remainder after performing division.

        This is primarily a wrapper for kdiv() function. Negative
        values are handled here.
        '''

        # Check if other != 0
        if not other.__nonzero__():
            raise ZeroDivisionError, 'integer division or modulo by zero'
        quot = DecInt()
        rmdr = DecInt()
        quot.terms, rmdr.terms = fastdiv(self.terms, other.terms, self.radix)
        quot.radix = self.radix
        rmdr.radix = self.radix
        if self.sign == other.sign:
            quot.sign = 1
            rmdr.sign = other.sign
        else:
            quot.sign = -1
            rmdr.sign = 1
            if rmdr.terms != [0]:
                rmdr.sign = other.sign
                # Increment the quotient and normailize it.
                carry = 1L
                index = 0
                RADIX = Radix2Value(self.radix)
                while carry and (index < len(quot.terms)):
                    carry, quot.terms[index] = divmod(quot.terms[index] + carry, RADIX)
                    index += 1
                if carry:
                    quot.terms.append(carry)
                # New remainder is "other" - "old remainder".
                temp = sub(other.terms[:len(rmdr.terms)], rmdr.terms)
                temp.extend(other.terms[len(rmdr.terms):])
                rmdr.terms = temp
                divnorm(rmdr.terms, self.radix)
        return (quot, rmdr)

    @_validate_vars
    @_coerce_vars
    def __rdivmod__(self, other):
        return other.__divmod__(self)

    @_validate_vars
    @_coerce_vars
    def __floordiv__(self, other):
        return self.__divmod__(other)[0]

    @_validate_vars
    @_coerce_vars
    def __rfloordiv__(self, other):
        return other.__divmod__(self)[0]

    @_validate_vars
    @_coerce_vars
    def __mod__(self, other):
        return self.__divmod__(other)[1]

    @_validate_vars
    @_coerce_vars
    def __rmod__(self, other):
        return other.__divmod__(self)[1]

    @_validate_vars
    @_coerce_vars
    def __div__(self, other):
        return self.__divmod__(other)[0]

    @_validate_vars
    @_coerce_vars
    def __rdiv__(self, other):
        return other.__divmod__(self)[0]

    @_validate_vars
    def ToLong(self):
        '''Convert the list of terms to a single number.

        The type of the result is the same as the type of mpops.mp.
        This is to allow testing tools to take advantage of faster
        implementations.

        Use __long__ (or long( )) to get a native Python long.'''

        temp = tolong2(self.terms, self.radix)
        if self.sign < 0:
            temp = -temp
        return temp

    @_validate_vars
    def __long__(self):
        '''Convert a DecInt into a native Python long.

        The result is cached so subsequent calls are very fast.'''

        if self.longval == None:
            self.longval = long(self.ToLong())
        return self.longval

    @_validate_vars
    def __hash__(self):
        '''Calculate the hash (dictionary key) value.

        This may not be fast because it may have to convert the number to
        a native Python long integer first.'''

        if self.hashval == None:
            if self.longval == None:
                self.longval = self.__long__()
            self.hashval = hash(self.longval)
        return self.hashval

# ===========================================================================
#
# Examples
#
# ===========================================================================

def gcd(self, other):
    '''Calculate the Greatest Common Divisor.'''

    while self:
        self, other = other % self, self
    return other

def square_root(x):
    '''Caclculate the integer square root.'''

    # Force x to be a DecInt.
    if not isinstance(x, DecInt):
        x = DecInt(x)

    # Starting value for the root.
    root = DecInt('4' * (len(str(x)) // 2))

    # Newton's method
    while 1:
        newroot = (x + root ** 2) // (2 * root)
        if root == newroot:
            break
        root = newroot

    return root

def pell(d, howmany = 1):
    '''Solve x^2 - d y^2 = +/-1.

    Note: d must be square-free!

    The function returns a list of tuples. howmany controls how many distinct
    results are returned. Each tuple contains an (x, y, +/-1) that solves the
    equation. Certain values of d allow a solution for -1; if it exists, it
    will be returned first.'''

    if not isinstance(d, DecInt):
        d = DecInt(d)

    # Algorithm from "Recreations in the Theory of Numbers" by A. H. Beiler.

    pp = ZERO
    qq = ONE
    a = aa = square_root(d)
    p = [ONE, a1]
    q = [ZERO, ONE]
    n = 1
    res = []

    while 1:
        pp = a * qq - pp
        qq = (d - pp ** 2) // qq
        a = (aa + pp) // qq
        p = [p[1], a * p[1] + p[0]]
        q = [q[1], a * q[1] + q[0]]
        n += 1
        if abs(qq) == 1:
            res.append((p[0], q[0], (-1) ** (n-1), n-1))
            if len(res) == howmany: break

    return res

if '__name__' == '__main__':
    pass


