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


#ifndef __EVENTEDITORUTILS_HPP__
#define __EVENTEDITORUTILS_HPP__

#include <QString>
#include <QStringList>
#include <stdlib.h>


#if BRANCHFACTOR > BLOCKFACTOR
#error "BRANCHFACTOR cannot be bigger than BLOCKFACTOR"
#endif

// height is defined as the directory level above the block files
// e.g.: blockfiles: height == 0
// first directory above: height == 1
// ...

#define TOTALHEIGHT (((int)((64-BLOCKFACTOR) / BRANCHFACTOR) < ((float)(64-BLOCKFACTOR) / (float)BRANCHFACTOR))?\
                     (int)((64-BLOCKFACTOR) / BRANCHFACTOR)+1:(int)((64-BLOCKFACTOR) / BRANCHFACTOR))


// Declarations
int height2NodePower( int height );
int height2Power( int height );
quint64 height2mask( int height );
quint64 dirName2sliceMin( QString dirName, int height, quint64 pathValue );
quint64 dirName2sliceMax( QString dirName, int height, quint64 pathValue );
QStringList generatePathComponents ( quint64 event, int height = 0 );
quint64 nodeMask( int height );
quint64 event2dirValue( quint64 event, int height );
QString event2dirName( quint64 event, int height );

template <typename T>
class InlineVec
{
public:
    InlineVec();
    ~InlineVec();

    void append( T& element );
    T& operator[]( long idx );
    InlineVec<T>& operator= (const InlineVec<T> & other);
    long getMaxIdx();
private:
    T* buffer;
    int bufLength;
    int maxIdx;
};

template <typename T>
inline InlineVec<T>::InlineVec()
{
    bufLength = 1;
    maxIdx = 0;
    buffer = (T*) malloc( 1 * sizeof(T) );
}

template <typename T>
inline InlineVec<T>::~InlineVec()
{
    if ( buffer != NULL )
    {
        free( buffer );
        buffer = NULL;
    }
}

template <typename T>
inline void InlineVec<T>::append(T& element)
{
    if ( maxIdx + 1 > bufLength )
    {
        bufLength *= 2;
        buffer = (T*) realloc( buffer, bufLength*sizeof(T) );
    }
    buffer[maxIdx++] = element;
}

template <typename T>
inline T& InlineVec<T>::operator[](long idx)
{
    return buffer[idx];
}

template <typename T>
inline InlineVec< T >& InlineVec<T>::operator=(const InlineVec< T >& other)
{
    bufLength = other.bufLength;
    maxIdx = other.maxIdx;
    buffer = (T*) realloc( buffer, bufLength*sizeof(T) );
    for( quint64 i = 0; i < maxIdx; i++ )
    {
      buffer[i] = other.buffer[i];
    }
    return *this;
}


template <typename T>
inline long InlineVec<T>::getMaxIdx()
{
    return maxIdx;
}


// inline definitions
inline int height2NodePower(int height)
{
    return height * BRANCHFACTOR;
}

inline int height2Power(int height)
{
    return BLOCKFACTOR + height * BRANCHFACTOR;
}

inline quint64 height2mask(int height)
{
    int shiftwidth = BLOCKFACTOR+height*BRANCHFACTOR;
    quint64 mask;
    if ( shiftwidth > 63 )
    {
        mask = 0;
    }
    else
    {
        mask = (~((quint64)0)) << shiftwidth;
    }
    return mask;
}

inline quint64 dirName2sliceMin( QString dirName, int height, quint64 pathValue )
{
    quint64 sliceValueBegin;

    if ( height2Power(height) < 64 )
    {
        sliceValueBegin = pathValue | ( dirName.toULongLong ( NULL, 16 ) << height2Power(height) );
    }
    else
    {
        sliceValueBegin = 0;
    }
    return sliceValueBegin;
}

inline quint64 dirName2sliceMax( QString dirName, int height, quint64 pathValue )
{
    quint64 sliceValueEnd;

    if ( height2Power(height) < 64 )
    {
        sliceValueEnd = pathValue | ~ ( ( ~dirName.toULongLong ( NULL, 16 ) ) << height2Power(height) );
    }
    else
    {
        sliceValueEnd = ~((quint64) 0);
    }
    return sliceValueEnd;
}

inline QStringList generatePathComponents ( quint64 event, int height )
{
    QStringList result;

    quint64 branchmask = (((quint64)1)<<BRANCHFACTOR)-1;
    int restwidth = 64-(BLOCKFACTOR+height*BRANCHFACTOR);
    event  >>= BLOCKFACTOR+height*BRANCHFACTOR;

    for ( int tmpheight = height; tmpheight < TOTALHEIGHT; tmpheight++ )
    {
        quint64 value;
        int textWidth;
        value = event & branchmask;
        event >>= BRANCHFACTOR;
        textWidth = qMin( BRANCHFACTOR/4, restwidth/4 );
        restwidth -= BRANCHFACTOR;
        result.prepend ( QString ( "%1" ).arg ( value, textWidth, 16, QChar ( '0' ) ) );
    }

    return result;
}

inline quint64 nodeMask(int height)
{
    int shiftwidth = height*BRANCHFACTOR;
    quint64 mask;
    if ( shiftwidth > 63 )
    {
        mask = 0;
    }
    else
    {
        mask = (~((quint64)0)) << shiftwidth;
    }
    return mask;
}

inline quint64 event2dirValue( quint64 event, int height )
{
    quint64 mask;
    mask = ((((quint64)1) << BRANCHFACTOR) - 1) << (BLOCKFACTOR+(height-1)*BRANCHFACTOR);

    return event & mask;
}

inline QString event2dirName( quint64 event, int height )
{
    quint64 value = event2dirValue( event, height );
    value >>= BLOCKFACTOR+(height-1)*BRANCHFACTOR;
    int textWidth = qMin( (64 - BLOCKFACTOR - (height-1) * BRANCHFACTOR)/4, BRANCHFACTOR/4);
    QString result = QString ( "%1" ).arg ( value, textWidth, 16, QChar ( '0' ) );
    return result;
}

#endif // __EVENTEDITORUTILS_HPP__
