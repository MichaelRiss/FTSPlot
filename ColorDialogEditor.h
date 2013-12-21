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


#ifndef COLORDIALOGEDITOR_H
#define COLORDIALOGEDITOR_H

#include <QColorDialog>

namespace O1Plot
{

class ColorDialogEditor : public QColorDialog
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor USER true)
public:
    ColorDialogEditor(QWidget *parent = 0);
    QColor color() const;
    void setColor(QColor c);

signals:

public slots:

};

}

#endif // COLORDIALOGEDITOR_H
