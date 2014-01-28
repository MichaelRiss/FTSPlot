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


#ifndef COLORDIALOGDELEGATE_H
#define COLORDIALOGDELEGATE_H

#include <QStyledItemDelegate>
#include "ColorDialogEditor.h"

namespace FTSPlot
{

class ColorDialogDelegate : public QStyledItemDelegate
{

public:
    virtual QWidget* createEditor ( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
    virtual void setEditorData ( QWidget* editor, const QModelIndex& index ) const;
    virtual void setModelData ( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const;
};

}

#endif // COLORDIALOGDELEGATE_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
