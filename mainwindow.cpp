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


#include "./mainwindow.h"

MainWindow::MainWindow ( QWidget *parent )
        : QMainWindow ( parent )
{
    setupUi ( this );

    connect ( MySimpleViewWidget, SIGNAL ( writeCrossCoords ( const QString &, int ) ),
              statusBar(), SLOT ( showMessage ( const QString&, int ) ) );

    connect ( MySimpleViewWidget, SIGNAL ( clearCrossCoords() ),
              statusBar(), SLOT ( clearMessage() ) );

    connect ( MySimpleViewWidget, SIGNAL ( newCoords ( long double,double,double,double ) ),
              this, SLOT ( updateCoords ( long double,double,double,double ) ) );

    connect ( this, SIGNAL ( XscaleUpdate ( long double, double ) ),
              MyXScaleBar, SLOT ( updateCoords ( long double, double ) ) );

    connect ( this, SIGNAL ( YscaleUpdate ( double, double ) ),
              MyYScaleBar, SLOT ( updateCoords ( double, double ) ) );

    connect ( this, SIGNAL ( GLPlotUpdate ( long double, double, double, double ) ),
              MySimpleViewWidget, SLOT ( updateCoords ( long double, double, double, double ) ) );

    // set up the menu widget
    mb.setupUi ( &menuWidget );
    menuWidget.setAttribute ( Qt::WA_QuitOnClose, false );
    connect ( menuToggle, SIGNAL ( clicked() ), this, SLOT ( handleMenu() ) );
    // set up the table
    mb.moduleList->setModel ( MySimpleViewWidget->dataModel() );
    mb.moduleList->setItemDelegate ( &myDelegate );

    connect ( mb.AddTimeSeries, SIGNAL ( clicked() ), MySimpleViewWidget, SLOT ( addTimeSeries() ) );
    connect ( mb.AddEventList, SIGNAL ( clicked() ), MySimpleViewWidget, SLOT ( addEventEditor() ) );
    connect ( mb.AddIntervalList, SIGNAL ( clicked() ), MySimpleViewWidget, SLOT ( addIntervalEditor() ) );
    connect ( mb.moduleList->selectionModel(), SIGNAL ( selectionChanged ( QItemSelection,QItemSelection ) ),
              this, SLOT ( menuBoxSelection ( QItemSelection,QItemSelection ) ) );
    connect ( mb.upButton, SIGNAL ( clicked() ), this, SLOT ( upButtonHandler() ) );
    connect ( mb.downButton, SIGNAL ( clicked() ), this, SLOT ( downButtonHandler() ) );
    connect ( mb.DeleteModule, SIGNAL ( clicked() ), this, SLOT ( deleteHandler() ) );

    selectionMutex = false;
}

MainWindow::~MainWindow()
{
    // This destructor is apparently never called,
    // the necessary tear down code is in 
    // void closeEvent( QCloseEvent* ) function below.
    
    //delete( MySimpleViewWidget );
}

void MainWindow::updateCoords ( long double Xcursor, double Xscale, double Ymin, double Ymax )
{
    this->Xcursor = Xcursor;
    this->Xscale = Xscale;
    this->Ymin = Ymin;
    this->Ymax = Ymax;


    emit ( XscaleUpdate ( Xcursor, Xscale ) );
    emit ( YscaleUpdate ( Ymin, Ymax ) );
    emit ( GLPlotUpdate ( Xcursor, Xscale, Ymin, Ymax ) );
}

void MainWindow::handleMenu()
{
    if ( menuWidget.isVisible() )
    {
        menuWidget.hide();
    }
    else
    {
        menuWidget.show();
    }
}

void MainWindow::menuBoxSelection ( const QItemSelection& selected, const QItemSelection& deselected )
{
    // go through selected list
    // check for a single selected line
    // check for selections on several lines
    if ( selectionMutex )
    {
        return;
    }
    QItemSelectionModel* sModel = mb.moduleList->selectionModel();
    if ( sModel->selectedIndexes().size() >= 1 )
    {
        int firstRow = sModel->selectedIndexes().begin()->row();
        if ( ( sModel->selectedRows ( firstRow ).size() == 1 ) && ( sModel->selectedIndexes().size() == 5 ) )
        {
            return;
        }
        else
        {
            selectionMutex = true;
            sModel->clearSelection();
            mb.moduleList->selectRow ( firstRow );
            selectionMutex = false;
        }
    }
}

void MainWindow::upButtonHandler()
{
    QItemSelectionModel* sModel = mb.moduleList->selectionModel();
    if ( sModel->selectedRows().size() != 1 )
    {
        return;
    }

    int idx = sModel->selectedRows().begin()->row();
    if ( idx <= 0 )
    {
        return;
    }

    MySimpleViewWidget->dataModel()->swapRows ( idx, idx-1 );
    selectionMutex = true;
    sModel->clearSelection();
    mb.moduleList->selectRow ( idx-1 );
    selectionMutex = false;
}

void MainWindow::downButtonHandler()
{
    QItemSelectionModel* sModel = mb.moduleList->selectionModel();
    if ( sModel->selectedRows().size() != 1 )
    {
        return;
    }

    int idx = sModel->selectedRows().begin()->row();
    if ( idx >= MySimpleViewWidget->dataModel()->rowCount()-1 )
    {
        return;
    }

    MySimpleViewWidget->dataModel()->swapRows ( idx, idx+1 );
    selectionMutex = true;
    sModel->clearSelection();
    mb.moduleList->selectRow ( idx+1 );
    selectionMutex = false;
}

void MainWindow::deleteHandler()
{
    QItemSelectionModel* sModel = mb.moduleList->selectionModel();
    if ( sModel->selectedRows().size() != 1 )
    {
        return;
    }

    int idx = sModel->selectedRows().begin()->row();
    // disconnect - later
    MySimpleViewWidget->dataModel()->deleteModule ( idx );
    // delete - later

}

void MainWindow::closeEvent( QCloseEvent* event )
{
    delete( MySimpleViewWidget );
    event->accept();
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; 
