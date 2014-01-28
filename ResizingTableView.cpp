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


#include <QDebug>
#include <QHeaderView>
#include "ResizingTableView.h"

using namespace FTSPlot;

ResizingTableView::ResizingTableView ( QWidget* parent ) : QTableView ( parent )
{
    resizeColumnsToContents();
}


QSize ResizingTableView::minimumSizeHint() const
{
    int width = 2 + verticalHeader()->width();
    for( int i = 0; i < 5; i++ )
    {
      width += columnWidth( i );
    }
    QSize tmp = QAbstractScrollArea::minimumSizeHint();
    tmp.setWidth( width );
    return tmp;
}

