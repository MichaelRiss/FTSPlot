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


#include <QTextStream>
#include <QCoreApplication>
#include "mainwindow.h"
#include <iostream>
#include <QtDebug>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif


using namespace std;

int main ( int argc, char *argv[] )
{
#ifdef Q_WS_X11
    XInitThreads();
#endif

    QApplication app ( argc, argv );


    MainWindow* mainwidget = new MainWindow();

    mainwidget->show();
    return app.exec();
}

