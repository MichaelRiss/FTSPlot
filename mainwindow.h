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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_FTSPlotMainWindow.h"
#include "ui_MenuBox.h"
#include "ColorDialogDelegate.h"

using namespace FTSPlot;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
public:
    MainWindow( QWidget * parent = 0 );
    ~MainWindow();

protected:

private:
    long double Xcursor;
    double Xscale;
    double Ymin;
    double Ymax;
    long double Xmax;
    QWidget menuWidget;
    Ui::MenuBox mb;
    ColorDialogDelegate myDelegate;
    bool selectionMutex;

signals:
  void XscaleUpdate( long double Xcursor, double Xscale );
  void YscaleUpdate( double Ymin, double Ymax );
  void GLPlotUpdate( long double Xcursor, double Xscale, double Ymin, double Ymax );

public slots:
  void updateCoords( long double Xcursor, double Xscale, double Ymin, double Ymax );
  void handleMenu();
  void menuBoxSelection( const QItemSelection & selected, const QItemSelection & deselected );
  void upButtonHandler();
  void downButtonHandler();
  void deleteHandler();
protected:
  void closeEvent( QCloseEvent* );
};

#endif // MAINWINDOW_H
