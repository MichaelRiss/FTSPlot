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


#include "Pow2SpinBox.h"

using namespace FTSPlot;

Pow2SpinBox::Pow2SpinBox ( QWidget* parent ) : QSpinBox ( parent )
{

}

QValidator::State Pow2SpinBox::validate ( QString& input, int& pos ) const
{
    return QSpinBox::validate ( input, pos );
}

int Pow2SpinBox::valueFromText ( const QString& text ) const
{
    int origval = QSpinBox::valueFromText ( text );
    int val = origval - 1;
    val |= val >> 1;  // handle  2 bit numbers
    val |= val >> 2;  // handle  4 bit numbers
    val |= val >> 4;  // handle  8 bit numbers
    val |= val >> 8;  // handle 16 bit numbers
    val |= val >> 16; // handle 32 bit numbers
    val++;
    if ( ! ( origval & ( val>>2 ) ) )
    {
        val = val >> 1;
    }
    return val;
}

void Pow2SpinBox::stepBy ( int steps )
{
    switch ( steps )
    {
    case 1:
        setValue ( value() << 1 );
        break;

    case -1:
        setValue ( value() >> 1 );
        break;

    default:
        qDebug ( "Pow2SpinBox::stepBy: input not handled." );
    }
}

