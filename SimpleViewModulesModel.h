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


#ifndef SIMPLEVIEWMODULESMODEL_H
#define SIMPLEVIEWMODULESMODEL_H

namespace FTSPlot
{
    
class vizModule;
class SimpleViewWidget;

}

#include <QAbstractTableModel>

namespace FTSPlot
{

class SimpleViewModulesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SimpleViewModulesModel ( SimpleViewWidget* upper );

    int rowCount ( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount ( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex &index, int role ) const;
    QVariant headerData ( int section, Qt::Orientation orientation,
                          int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags ( const QModelIndex &index ) const;
    bool setData ( const QModelIndex &index, const QVariant &value,
                   int role = Qt::EditRole );
    bool insertRows ( int position, int rows, const QModelIndex &index = QModelIndex() );
    bool removeRows ( int position, int rows, const QModelIndex &index = QModelIndex() );
    void swapRows( int first, int second );
    void append( vizModule& mod );
    void prepend( vizModule& mod );
    void deleteModule( int idx );
private:
    SimpleViewWidget* dataObject;
public slots:
    void GUIupdate();
};

}

#endif // SIMPLEVIEWMODULESMODEL_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
