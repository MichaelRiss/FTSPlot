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


#ifndef __BENCHMARKHELPER__
#define __BENCHMARKHELPER__

/*
 * This class is only used for transporting statistics from the worker threads 
 * to the benchmark control logic.
 */

#include <QtCore>
#include <QGLWidget>
#include "mmapFileInfo.h"
#include "EditorUtils.hpp"

namespace FTSPlot
{

class benchmarkHelper
{
public:
  static qint64 vertexCount;
  static qint64 lineCount;
  static qint64 quadCount;
  static QVector<mmapFileInfo>* filesptr;

#ifdef COUNT_TILES
  static InlineVec<GLdouble> BoxArray;
  static InlineVec<GLdouble> LineArray;
#endif
};

}

#endif // __BENCHMARKHELPER__
