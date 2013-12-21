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


#include <QDebug>
#include "ColorDialogDelegate.h"

using namespace O1Plot;

QWidget* ColorDialogDelegate::createEditor ( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    if ( index.column() == 4 )
    {
        return new ColorDialogEditor ( parent );
    }
    else
    {
        return QStyledItemDelegate::createEditor ( parent, option, index );
    }
}

void ColorDialogDelegate::setEditorData ( QWidget* editor, const QModelIndex& index ) const
{
    if ( index.column() == 4 )
    {
        ( ( ColorDialogEditor* ) editor )->setColor ( index.data ( Qt::DecorationRole ).value<QColor>() );
    }
    else
    {
        QStyledItemDelegate::setEditorData ( editor, index );
    }
}

void ColorDialogDelegate::setModelData ( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const
{
    if ( index.column() == 4 )
    {
        bool success = model->setData ( index, QVariant ( ( ( ColorDialogEditor* ) editor )->color() ), Qt::DecorationRole );
    }
    else
    {
        QStyledItemDelegate::setModelData ( editor, model, index );
    }
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; 
