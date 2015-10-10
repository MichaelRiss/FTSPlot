/* FTSPlot - fast time series dataset plotter
   Copyright (C) 2015  Michael Riss <Michael.Riss@gmail.com>

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

#ifndef DISPLAYLISTDATA_H_
#define DISPLAYLISTDATA_H_

#include <QOpenGLFunctions_1_0>
#include <QVector>

namespace FTSPlot
{

template <class T>
class displaylistdata {
public:
	GLenum drawtype;
	QVector<T> data;
	int maxIdx;
	displaylistdata();
	void reset();
	void append( T& );
	T& operator[](int i);
};

template <class T>
displaylistdata<T>::displaylistdata() : drawtype(GL_LINES), maxIdx(0){
}

template <class T>
void displaylistdata<T>::reset(){
	maxIdx = 0;
}

template <class T>
void displaylistdata<T>::append( T& item ){
	if( maxIdx < data.size() ){
		data[maxIdx] = item;
	} else {
		data.append( item );
	}
	maxIdx++;
}

template <class T>
T& displaylistdata<T>::operator [](int i){
	return data[i];
}

}

#endif /* DISPLAYLISTDATA_H_ */
