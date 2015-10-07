/* FTSPlot - fast time series dataset plotter
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


#ifndef INTERVAL_H
#define INTERVAL_H

#include <QtCore>

namespace FTSPlot
{

class Interval
{
public:
    Interval ( quint64 begin = 0, quint64 end = 1 );
    quint64 begin;
    quint64 end;
    bool operator< ( const Interval & other ) const;
    bool operator> ( const Interval & other ) const;
    bool operator== ( const Interval & other ) const;
    bool operator!= ( const Interval & other ) const;
};


QDebug& operator<< ( QDebug& debug, const Interval& b );

}

#endif // INTERVAL_H

