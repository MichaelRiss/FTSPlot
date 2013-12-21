/* O1Plot - fast time series dataset plotter
   Copyright (C) 2013  Michael Riss <Michael.Riss@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA */


#include <limits>

#define FloatNaN std::numeric_limits<float>::quiet_NaN()
#define DoubleNaN std::numeric_limits<double>::quiet_NaN()
#define LongDoubleNaN std::numeric_limits<long double>::quiet_NaN()

#define FloatInfinity std::numeric_limits<float>::infinity()
#define DoubleInfinity std::numeric_limits<double>::infinity()
#define LongDoubleInfinity std::numeric_limits<long double>::infinity()

template<typename T>
inline bool is_nan(T value)
{
	return value != value;
}
