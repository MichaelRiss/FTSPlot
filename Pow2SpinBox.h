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


#ifndef POW2SPINBOX_H
#define POW2SPINBOX_H

#include <QSpinBox>

namespace O1Plot
{

class Pow2SpinBox : public QSpinBox
{

  protected:
    virtual QValidator::State validate(QString& input, int& pos) const;
    virtual int valueFromText(const QString& text) const;

  public:
    Pow2SpinBox(QWidget* parent = 0);
    virtual void stepBy(int steps);
};

}

#endif // POW2SPINBOX_H
