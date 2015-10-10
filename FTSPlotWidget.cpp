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


#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <FTSPlotWidget.h>
#include <cmath>
#include "commonDefs.h"
#include "EventEditor.h"
#include "IntervalEditor.h"
#include "TimeSeriesPlot.h"
#include "ui_FileOpenFailed.h"

using namespace std;
using namespace FTSPlot;

#define MAX(a,b) a<b?b:a
#define MIN(a,b) a<b?a:b

#define QINT64_MAX LLONG_MAX
#define QINT64_MIN LLONG_MIN


Qt::GlobalColor FTSPlotWidget::stdColors[] = { Qt::black, Qt::red, Qt::green, Qt::blue, Qt::magenta, Qt::cyan,
        Qt::darkYellow, Qt::darkGray
                                                };

FTSPlotWidget::FTSPlotWidget ( QWidget* parent ) :
        QOpenGLWidget ( parent ), svm ( this )
{
    windowWidth = 0;
    windowHeight = 0;

    XdataBegin = 0;
    XdataEnd = 1;
    power = 1;
    Xcursor = 0.0;
    Xscale = 1.0;
    Ymin = -1.0;
    Ymax = 1.0;
    YFrustMax = 1.0;
    YFrustMin = -1.0;


    NoStdColors = sizeof ( stdColors ) / sizeof ( Qt::GlobalColor );
    colorIdx = 0;
    changeDispListSet = false;
    requested = false;
    updateNeeded = false;

    setMouseTracking ( true );

    activeInputModule = NULL;
    setFocusPolicy ( Qt::StrongFocus );

    EventListCounter = 0;
    IntervalListCounter = 0;
}

FTSPlotWidget::~FTSPlotWidget()
{
    for ( int i = modules.size() - 1; i >= 0; i-- )
    {
        deleteModule ( modules[i].module );
    }
}

void FTSPlotWidget::initializeGL()
{
	initializeOpenGLFunctions();
    glClearColor ( 1.0, 1.0, 1.0, 1.0 );
    glShadeModel ( GL_FLAT );

    glEnable ( GL_LINE_SMOOTH );
    glEnable ( GL_BLEND );
    glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glHint ( GL_LINE_SMOOTH_HINT, GL_DONT_CARE );
    glLineWidth ( 1.0 );
}


void FTSPlotWidget::resizeGL ( int w, int h )
{
    if ( w == 0 || h == 0 )
    {
        doRender = false;
    }
    else
    {
        doRender = true;
        windowWidth = w;
        windowHeight = h;

        setupViewportProjection();
    }
}


void FTSPlotWidget::setupViewportProjection()
{
    makeCurrent();
    // Left and right window edges in graph coordinates
    long double XleftEdge = Xcursor - ( long double ) XdataBegin;
    long double XrightEdge = XleftEdge + windowWidth * Xscale;

    glViewport ( 0, 0, ( GLint ) windowWidth, ( GLint ) windowHeight );

    glMatrixMode ( GL_PROJECTION );
    glLoadIdentity ();

    GLdouble xbegin = XleftEdge / ( ( qint64 ) 1<<dispPower );

    GLdouble xend = XrightEdge / ( ( qint64 ) 1<<dispPower );

    //gluOrtho2D ( xbegin, xend, Ymin, Ymax );
    glOrtho ( xbegin, xend, Ymin, Ymax, -1.0, 1.0 );
}


void FTSPlotWidget::paintGL()
{
	if ( changeDispListSet )
    {
        useList.clear();
        for ( int i = 0; i < vizList.size(); i++ )
        {
            useList.append ( vizList[i] );
        }
        XdataBegin = reqXdataBegin;
        XdataEnd = reqXdataEnd;
        YFrustMin = reqYFrustMin;
        YFrustMax = reqYFrustMax;
        power = reqPower;
        dispPower = reqDispPower;
        changeDispListSet = false;
        setupViewportProjection();
    }

    if ( !doRender )
    {
        qDebug() << "doRender == false, not painting ...";
        return;
    }

#define OS_LOW 0
#define OS_HIGH 1

    if ( !requested )
    {
        // generate the 2^ factor of Xscale
        // compare it with the current 2^ factor
        // if too big or too small => trigger newDisplayList

        unsigned int currPower = ( unsigned int ) round ( log2 ( Xscale + 1 ) );
        if ( ( currPower < power + OS_LOW ) || ( currPower > power + OS_HIGH ) )
        {
            requestNewLists();
        }

        // if Xcursor is too close to the borders of the current tile
        // => trigger newDisplayList
        if ( ( ( Xcursor < XdataBegin + windowWidth * 0.5 * Xscale ) && ( XdataBegin > QINT64_MIN ) ) ||
                ( ( Xcursor > XdataEnd - windowWidth * 1.5 * Xscale ) && ( XdataEnd < QINT64_MAX ) ) )
        {
            requestNewLists();
        }

        // if visible area gets too close to or far away from the frustum borders in Y direction
        // => trigger newDisplayList
        if ( ( ( YFrustMax - Ymax ) < ( Ymax - Ymin ) ) ||
                ( ( YFrustMax - Ymax ) > 100 * ( Ymax - Ymin ) ) ||
                ( ( Ymin - YFrustMin ) < ( Ymax - Ymin ) ) ||
                ( ( Ymin - YFrustMin ) > 100 * ( Ymax - Ymin ) ) )
        {
            requestNewLists();
        }

        if ( updateNeeded )
        {
            for ( int i = 0; i < updateList.size(); i++ )
            {
                requestNewList ( updateList[i] );
            }
            updateList.clear();
            updateNeeded = false;
        }
    }

#ifdef BENCHMARK
    struct timespec ts_start;
    struct timespec ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);
#endif // BENCHMARK

    glClear ( GL_COLOR_BUFFER_BIT );

    for ( int i = 0; i < useList.size(); i++ )
    {
        useList[i]->paintGL();
    }

    glFlush();

#ifdef BENCHMARK
    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    long diffTime = ts_end.tv_nsec - ts_start.tv_nsec +
                    (ts_end.tv_sec - ts_start.tv_sec) * 1000000000;
    emit paintTime( diffTime );
    if ( !requested )
        emit paintingDone();
#endif // BENCHMARK

    GLenum err = glGetError();
    if ( err != GL_NO_ERROR )
    {
        qDebug() << "FTSPlotWidget paintGL error number: " << err;
    }
}

void FTSPlotWidget::mousePressEvent ( QMouseEvent* event )
{
    if ( activeInputModule )
    {
        activeInputModule->mousePressEvent ( event );
        if ( event->isAccepted() )
        {
            return;
        }
    }

    Qt::MouseButtons mbs = event->buttons();
    if ( mbs & Qt::LeftButton )
    {
        MouseX = event->x();
        MouseY = event->y();
    }
    event->accept();
}

void FTSPlotWidget::requestNewLists()
{
    if ( !requested )
    {
        // disable external updates they get served now anyway
        updateList.clear();
        updateNeeded = false;

#define powerfac 2
        reqPower = ( unsigned int ) round ( log2 ( MAX ( Xscale, 1 ) ) );
        reqDispPower = MAX ( ( int ) reqPower - powerfac, 0 );

        reqXdataBegin = MAX ( Xcursor - ( MAX ( windowWidth*Xscale, 2 ) ), QINT64_MIN );
        reqXdataEnd = MIN ( Xcursor + ( MAX ( 2*windowWidth*Xscale, 2 ) ), QINT64_MAX );

        reqYFrustMax = Ymax + 10 * (Ymax - Ymin);
        reqYFrustMin = Ymin - 10 * (Ymax - Ymin);

        reqSet.clear();
        reqList.clear();
        for ( int i = 0; i < vizList.size(); i++ )
        {
            reqSet.insert ( vizList[i] );
            reqList.append ( vizList[i] );
            vizList[i]->genDisplayList ( reqXdataBegin, reqXdataEnd, reqDispPower, reqYFrustMin, reqYFrustMax );
        }
        if ( reqSet.size() > 0 )
        {
            requested = true;
#ifdef BENCHMARK
            clock_gettime( CLOCK_MONOTONIC, &displayList_startTime );
#endif // BENCHMARK
        }
        else
        {
            changeDispListSet = true;
            update();
        }
    }
}

void FTSPlotWidget::requestNewList ( GL_Layer* mod )
{
    // This is the routine to update a single Layer
    if ( !vizList.contains ( mod ) )
    {
        // We get into this case when a hidden module requests an update. As it is hidden 
        // there is no need to regenerate its display list. We could also handle this condition 
        // separately in the modules but it's better to handle it centrally here.
        
        return; // Here we might want to check if "mod" exist and throw an error otherwise.
    }
    reqPower = power;
    reqXdataBegin = XdataBegin;
    reqXdataEnd = XdataEnd;
    reqDispPower = MAX ( ( int ) reqPower - powerfac, 0 ) ;
    reqSet.insert ( mod );
    reqList.append ( mod );
    mod->genDisplayList ( reqXdataBegin, reqXdataEnd, reqDispPower, YFrustMin, YFrustMax );
    requested = true;
}


void FTSPlotWidget::mouseMoveEvent ( QMouseEvent* event )
{
    if ( activeInputModule )
    {
        activeInputModule->mouseMoveEvent ( event );
        if ( event->isAccepted() )
        {
            return;
        }
    }

    Qt::MouseButtons mbs = event->buttons();
    if ( mbs & Qt::LeftButton )
    {
        if ( event->modifiers() == Qt::ControlModifier )
        {
            int dx = event->x() - MouseX;
            MouseX = event->x();
            int dy = event->y() - MouseY;
            MouseY = event->y();

            Xcursor -= ( double ) dx * Xscale;
            double Yscale = ( double ) ( Ymax - Ymin ) / windowHeight;
            Ymin = Ymin += dy * Yscale;
            Ymax = Ymax += dy * Yscale;

            setupViewportProjection();
            update();
            emit ( newCoords ( Xcursor, Xscale, Ymin, Ymax ) );
        }
        else
        {
            int dx = event->x() - MouseX;
            MouseX = event->x();
            MouseY = event->y();

            Xcursor -= ( double ) dx * Xscale;

            setupViewportProjection();
            update();
            emit ( newCoords ( Xcursor, Xscale, Ymin, Ymax ) );
        }

    }

    crossHairX = Xscreen2graph ( event->x() );
    crossHairY = Yscreen2graph ( event->y() );
    QString message;
    message = message + QString::number ( ( qint64 ) crossHairX ) + ", "
              + QString::number ( crossHairY );
    emit writeCrossCoords ( message, 0 );
    event->accept();
}


long double FTSPlotWidget::Xscreen2graph ( int x )
{
    long double result = Xcursor + ( double ) ( x ) * Xscale;
    return result;
}

double FTSPlotWidget::Yscreen2graph ( int y )
{
    double result = Ymax - ( ( double ) y / ( double ) windowHeight * ( Ymax - Ymin ) );
    return result;
}

double FTSPlotWidget::Xgraph2GL ( long double x )
{
    double result = ( x - XdataBegin ) / ( 1<<dispPower );
    return result;
}


void FTSPlotWidget::wheelEvent ( QWheelEvent *event )
{
    if ( activeInputModule )
    {
        activeInputModule->wheelEvent ( event );
        if ( event->isAccepted() )
        {
            return;
        }
    }

    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;

    if ( event->orientation() == Qt::Horizontal )
    {
        event->ignore();
    }
    else
    {
        if ( event->modifiers() == Qt::ControlModifier )
        {
            // Get mouse pointer position
            QPoint pos = mapFromGlobal ( QCursor::pos () );

            double ScalePoint = Yscreen2graph ( pos.y() );

            double dmax = Ymax - ScalePoint;
            double dmin = Ymin - ScalePoint;

            double newdmax = dmax * pow ( ( 0.2 + 1 ), -numSteps );
            double newdmin = dmin * pow ( ( 0.2 + 1 ), -numSteps );

            Ymin = ScalePoint + newdmin;
            Ymax = ScalePoint + newdmax;

            setupViewportProjection();
            update();
            emit ( newCoords ( Xcursor, Xscale, Ymin, Ymax ) );
        }
        else
        {
            // Get mouse pointer position
            QPoint pos = mapFromGlobal ( QCursor::pos () );

            // 1. Calculate the "Xcursor - position" under the mouse pointer
            long double ScalePoint = Xscreen2graph ( pos.x() );
            // 2. Calculate the distance
            long double dist = ScalePoint - Xcursor;
            // 3. Multiply distance with new length / old length
            long double newdist = dist * pow ( ( 0.2 + 1 ), -numSteps );
            long double move = dist - newdist;
            // 4. update Xcursor
            Xcursor += move;

            Xscale *= pow ( ( 0.2 + 1 ), -numSteps );

            setupViewportProjection();
            update();
            emit ( newCoords ( Xcursor, Xscale, Ymin, Ymax ) );
        }
        event->accept();
    }
}


void FTSPlotWidget::enterEvent ( QEvent * event )
{
    if ( activeInputModule )
    {
        activeInputModule->enterEvent ( event );
        if ( event->isAccepted() )
        {
            return;
        }
    }
    QPoint pos = mapFromGlobal ( QCursor::pos () );
    MouseX = pos.x();
    MouseY = pos.y();
    event->ignore();
}

void FTSPlotWidget::leaveEvent ( QEvent * event )
{
    if ( activeInputModule )
    {
        activeInputModule->leaveEvent ( event );
        if ( event->isAccepted() )
        {
            return;
        }
    }
    event->ignore();
    emit clearCrossCoords();
}

void FTSPlotWidget::keyPressEvent ( QKeyEvent * event )
{
    if ( activeInputModule )
    {
        activeInputModule->keyPressEvent ( event );
        if ( event->isAccepted() )
        {
            return;
        }
    }
}

void FTSPlotWidget::keyReleaseEvent ( QKeyEvent * event )
{
    if ( activeInputModule )
    {
        activeInputModule->keyReleaseEvent ( event );
        if ( event->isAccepted() )
        {
            return;
        }
    }
}


GL_Layer* FTSPlotWidget::addTimeSeries ( QString cfgFileName )
{
	if( !context()->isValid() ){
		qDebug() << "FTSPlotWidget::addTimeSeries: context invalid, exiting. Make sure to \"show\" the FTSPlotWidget before adding modules.";
		exit(1);
	}
    TimeSeriesPlot* newTS = new TimeSeriesPlot ( this );
    if ( !newTS->openFile ( cfgFileName ) )
    {
        delete ( newTS );
        return NULL;
    }
    // The TimeSeriesPlot was able to open the dataset
    vizModule tmp;
    tmp.module = newTS;
    QStringList pathComponents = cfgFileName.split ( "/" );
    tmp.name = pathComponents.last();
    tmp.color = stdColors[colorIdx%NoStdColors];
    colorIdx++;
    tmp.module->setColor ( tmp.color );
    tmp.visible = true;
    tmp.editable = false;
    connect ( tmp.module, SIGNAL ( notifyListUpdate ( GL_Layer* ) ),
              this, SLOT ( registerCompleted ( GL_Layer* ) ) );
    connect ( tmp.module, SIGNAL ( triggerRepaint() ),
              this, SLOT ( update() ) );
    connect ( tmp.module, SIGNAL ( GUIshowhide() ), dataModel(), SLOT ( GUIupdate() ) );
    svm.prepend ( tmp );
    updateVizList();
    return newTS;
}


void FTSPlotWidget::addTimeSeries()
{
    bool failureOccurred = false;
    
    QStringList fileNames = QFileDialog::getOpenFileNames ( this, "Open Dataset(s)", cachedTimeSeriesDirectory,
                            "DataSet(*.cfg);; Raw Binary File(*.bin);; All Files(*)" );
    if ( fileNames.empty() )
    {
        // User clicked cancel => return
        return;
    }

    QFileInfo dirInfo( fileNames.first() );
    cachedTimeSeriesDirectory = dirInfo.path();

    for ( int i = 0; i < fileNames.size(); i++ )
    {
        if ( addTimeSeries ( fileNames[i] ) == NULL )
        {
            failureOccurred = true;
        }
    }
    if ( failureOccurred )
    {
        // Show failure message
        QDialog* msg = new QDialog;
        Ui::FileOpenFailedDialog ui;
        ui.setupUi ( msg );
        msg->show();
        msg->exec();
        delete ( msg );
    }
    
    return;
}

GL_Layer* FTSPlotWidget::genEventEditor()
{
	if( !context()->isValid() ){
		qDebug() << "FTSPlotWidget::genEventEditor: context invalid, exiting. Make sure to \"show\" the FTSPlotWidget before adding modules.";
		exit(1);
	}

    EventEditor* ed = new EventEditor ( this );
    vizModule tmp;
    tmp.module = ed;
    tmp.name = QString ( "EventList " ) + QString::number ( EventListCounter++ );
    tmp.color = stdColors[colorIdx%NoStdColors];
    colorIdx++;
    tmp.module->setColor ( tmp.color );
    tmp.visible = true;
    tmp.editable = true;
    svm.append ( tmp );
    connect ( tmp.module, SIGNAL ( notifyListUpdate ( GL_Layer* ) ),
              this, SLOT ( registerCompleted ( GL_Layer* ) ) );
    connect ( tmp.module, SIGNAL ( triggerRepaint ( GL_Layer* ) ),
              this, SLOT ( ExtLayerUpdate ( GL_Layer* ) ) );
    connect ( tmp.module, SIGNAL ( centerCoordsOn ( long double,double,double,double ) ),
              this, SLOT ( displaySpot ( long double,double,double,double ) ) );
    connect ( tmp.module, SIGNAL ( GUIshowhide() ), dataModel(), SLOT ( GUIupdate() ) );
    activeInputModule = tmp.module;
    updateVizList();
    return ed;
}


GL_Layer* FTSPlotWidget::addEventEditorModule ( QString TreeDirName )
{
    EventEditor* ed = ( EventEditor* ) genEventEditor();
    ed->openTreeDir ( TreeDirName );
    return ed;
}


void FTSPlotWidget::addEventEditor()
{
    genEventEditor();
}

GL_Layer* FTSPlotWidget::genIntervalEditor()
{
	if( !context()->isValid() ){
		qDebug() << "FTSPlotWidget::genIntervalEditor: context invalid, exiting. Make sure to \"show\" the FTSPlotWidget before adding modules.";
		exit(1);
	}

    IntervalEditor* id = new IntervalEditor ( this );
    vizModule tmp;
    tmp.module = id;
    tmp.name = QString ( "IntervalList " ) + QString::number ( IntervalListCounter++ );
    tmp.color = stdColors[colorIdx%NoStdColors];
    colorIdx++;
    tmp.module->setColor ( tmp.color );
    tmp.visible = true;
    tmp.editable = true;
    svm.append ( tmp );
    connect ( tmp.module, SIGNAL ( notifyListUpdate ( GL_Layer* ) ),
              this, SLOT ( registerCompleted ( GL_Layer* ) ) );
    connect ( tmp.module, SIGNAL ( triggerRepaint ( GL_Layer* ) ),
              this, SLOT ( ExtLayerUpdate ( GL_Layer* ) ) );
    connect ( tmp.module, SIGNAL ( centerOn ( long double,double,double,double ) ),
              this, SLOT ( displaySpot ( long double,double,double,double ) ) );
    connect ( tmp.module, SIGNAL ( fitOn ( long double,long double ) ),
              this, SLOT ( displayRange ( long double,long double ) ) );
    connect ( tmp.module, SIGNAL ( GUIshowhide() ), dataModel(), SLOT ( GUIupdate() ) );
    activeInputModule = tmp.module;
    updateVizList();
    return id;
}

GL_Layer* FTSPlotWidget::addIntervalEditorModule ( QString TreeDirName )
{
    IntervalEditor* id = ( IntervalEditor* ) genIntervalEditor();
    id->openTreeDir ( TreeDirName );
    return id;
}


void FTSPlotWidget::addIntervalEditor()
{
    genIntervalEditor();
}

QList< vizModule > FTSPlotWidget::listModules()
{
    return modules;
}

bool FTSPlotWidget::deleteModule ( GL_Layer* mod )
{
    // search module entry
    bool success = false;
    int idx = -1;
    for ( int i = 0; i < modules.size(); i++ )
    {
        if ( modules[i].module == mod )
        {
            idx = i;
            success = true;
            break;
        }
    }
    // not found => return false
    if ( !success )
    {
        return false;
    }
    // found => delete, success
    svm.deleteModule ( idx );
    return true;
}

bool FTSPlotWidget::showModule ( GL_Layer* mod )
{
    // search module entry
    bool success = false;
    int idx = -1;
    for ( int i = 0; i < modules.size(); i++ )
    {
        if ( modules[i].module == mod )
        {
            idx = i;
            success = true;
            break;
        }
    }
    // not found => return false
    if ( !success )
    {
        return false;
    }
    // found => show, success
    svm.setData ( svm.index ( idx, 0 ), QVariant ( Qt::Checked ), Qt::CheckStateRole );
    return true;
}

QString FTSPlotWidget::getModuleName(GL_Layer* mod)
{
    // search module entry
    bool success = false;
    int idx = -1;
    for ( int i = 0; i < modules.size(); i++ )
    {
        if ( modules[i].module == mod )
        {
            idx = i;
            success = true;
            break;
        }
    }
    // not found => return false
    if ( !success )
    {
        return QString();
    }
    // found => get name
    QString result = svm.data ( svm.index ( idx, 3 ), Qt::EditRole ).toString();
    return result;
}

bool FTSPlotWidget::setModuleName(GL_Layer* mod, QString name)
{
    // search module entry
    bool success = false;
    int idx = -1;
    for ( int i = 0; i < modules.size(); i++ )
    {
        if ( modules[i].module == mod )
        {
            idx = i;
            success = true;
            break;
        }
    }
    // not found => return false
    if ( !success )
    {
        return false;
    }
    // found => set name
    svm.setData ( svm.index ( idx, 3 ), QVariant ( name ), Qt::EditRole );
    return true;
}


bool FTSPlotWidget::hideModule ( GL_Layer* mod )
{
    // search module entry
    bool success = false;
    int idx = -1;
    for ( int i = 0; i < modules.size(); i++ )
    {
        if ( modules[i].module == mod )
        {
            idx = i;
            success = true;
            break;
        }
    }
    // not found => return false
    if ( !success )
    {
        return false;
    }
    // found => hide, success
    svm.setData ( svm.index ( idx, 0 ), QVariant ( Qt::Unchecked ), Qt::CheckStateRole );
    return true;
}

bool FTSPlotWidget::isModuleVisible ( GL_Layer* mod )
{
    // search module entry
    bool success = false;
    int idx = -1;
    for ( int i = 0; i < modules.size(); i++ )
    {
        if ( modules[i].module == mod )
        {
            idx = i;
            success = true;
            break;
        }
    }
    // not found => return false
    if ( !success )
    {
        return false;
    }
    // found => getVisibility, success
    return modules[idx].visible;
}

bool FTSPlotWidget::showModuleGUI(GL_Layer* mod)
{
    // search module entry
    bool success = false;
    int idx = -1;
    for ( int i = 0; i < modules.size(); i++ )
    {
        if ( modules[i].module == mod )
        {
            idx = i;
            success = true;
            break;
        }
    }
    // not found => return false
    if ( !success )
    {
        return false;
    }
    // found => show, success
    svm.setData ( svm.index ( idx, 1 ), QVariant ( Qt::Checked ), Qt::CheckStateRole );
    return true;
}

bool FTSPlotWidget::hideModuleGUI(GL_Layer* mod)
{
    // search module entry
    bool success = false;
    int idx = -1;
    for ( int i = 0; i < modules.size(); i++ )
    {
        if ( modules[i].module == mod )
        {
            idx = i;
            success = true;
            break;
        }
    }
    // not found => return false
    if ( !success )
    {
        return false;
    }
    // found => hide, success
    svm.setData ( svm.index ( idx, 1 ), QVariant ( Qt::Unchecked ), Qt::CheckStateRole );
    return true;
}

bool FTSPlotWidget::isModuleGUIVisible(GL_Layer* mod)
{
    // search module entry
    bool success = false;
    int idx = -1;
    for ( int i = 0; i < modules.size(); i++ )
    {
        if ( modules[i].module == mod )
        {
            idx = i;
            success = true;
            break;
        }
    }
    // not found => return false
    if ( !success )
    {
        return false;
    }
    // found => getVisibility, success
    return modules[idx].module->GUIvisible();
}


QColor FTSPlotWidget::getModuleColor ( GL_Layer* mod )
{
    // search module entry
    bool success = false;
    int idx = -1;
    for ( int i = 0; i < modules.size(); i++ )
    {
        if ( modules[i].module == mod )
        {
            idx = i;
            success = true;
            break;
        }
    }
    // not found => return false
    if ( !success )
    {
        return QColor();
    }
    // found => getColor, success
    return modules[idx].color;
}


bool FTSPlotWidget::setModuleColor ( GL_Layer* mod, QColor col )
{
    // search module entry
    bool success = false;
    int idx = -1;
    for ( int i = 0; i < modules.size(); i++ )
    {
        if ( modules[i].module == mod )
        {
            idx = i;
            success = true;
            break;
        }
    }
    // not found => return false
    if ( !success )
    {
        return false;
    }
    // found => setColor, success
    svm.setData ( svm.index ( idx, 4 ), QVariant ( col ), Qt::DecorationRole );
    return true;
}

GL_Layer* FTSPlotWidget::getEditModule()
{
    return activeInputModule;
}

bool FTSPlotWidget::setEditModule ( GL_Layer* mod )
{
    // special case mod == NULL
    if ( mod == NULL )
    {
        svm.setData ( svm.index ( 0, 2 ), QVariant ( Qt::Checked ), Qt::CheckStateRole );
        svm.setData ( svm.index ( 0, 2 ), QVariant ( Qt::Unchecked ), Qt::CheckStateRole );
        return true;
    }

    // search module entry
    bool success = false;
    int idx = -1;
    for ( int i = 0; i < modules.size(); i++ )
    {
        if ( modules[i].module == mod )
        {
            idx = i;
            success = true;
            break;
        }
    }
    // not found => return false
    if ( !success )
    {
        return false;
    }
    // found => setEditModule, success
    svm.setData ( svm.index ( idx, 2 ), QVariant ( Qt::Checked ), Qt::CheckStateRole );
    return true;
}

bool FTSPlotWidget::reorderModules ( QList< vizModule > orderedMods )
{
    // first check if it is really just a reordering
    QList<vizModule> tmpOld = modules;
    QList<vizModule> tmpNew = orderedMods;

    for ( int i = tmpNew.size() - 1; i >= 0; i-- )
    {
        vizModule tmp = tmpNew.takeAt( i );
        for ( int j = tmpOld.size() - 1; j >= 0; j-- )
        {
            if ( tmp.module == tmpOld[j].module )
            {
                tmpOld.takeAt( j );
            }
        }
    }
    if ( tmpOld.size() != 0 || tmpNew.size() != 0 )
    {
        return false;
    }

    for ( int i = 0; i < orderedMods.size(); i++ )
    {
        for ( int j = 0; j < modules.size(); j++ )
        {
            if ( modules[j].module == orderedMods[i].module )
            {
                tmpNew.append( modules.takeAt(j) );
            }
        }
    }
    modules = tmpNew;
    svm.GUIupdate();
    return true;
}

void FTSPlotWidget::registerCompleted ( GL_Layer* module )
{
    if ( !reqSet.contains ( module ) )
    {
        qDebug() << "Warning: cannot register module completed if it's not in reqSet";
        return;
    }
    reqSet.remove ( module );
    if ( reqSet.empty() )
    {
        // tell all modules to switch to new displayLists
        for ( int i = 0; i < reqList.size(); i++ )
        {
            reqList[i]->toggleLists();
        }
        reqList.clear();
        changeDispListSet = true;
        requested = false;
#ifdef BENCHMARK
        struct timespec displayList_endTime;
        clock_gettime( CLOCK_MONOTONIC, &displayList_endTime );
        long diffTime = displayList_endTime.tv_nsec - displayList_startTime.tv_nsec +
                        (displayList_endTime.tv_sec - displayList_startTime.tv_sec) * 1000000000;
        emit displayListTime( diffTime );
#endif // BENCHMARK
        update();
    }
}

void FTSPlotWidget::updateVizList()
{
    vizList.clear();

    for ( int i = modules.size() - 1; i >= 0; i-- )
    {
        if ( modules[i].visible )
        {
            vizList.append ( modules[i].module );
        }
    }

    Xmin = LongDoubleInfinity;
    Xmax = -LongDoubleInfinity;
    for ( int i = 0; i < modules.size(); i++ )
    {
        long double tXmin = modules[i].module->getXmin();
        long double tXmax = modules[i].module->getXmax();
        double tYmin = modules[i].module->getMin();
        double tYmax = modules[i].module->getMax();
        if ( tXmin < Xmin )
            Xmin = tXmin;
        if ( tXmax > Xmax )
            Xmax = tXmax;
        if ( tYmin < Ymin )
            Ymin = tYmin;
        if ( tYmax > Ymax )
            Ymax = tYmax;
    }
    if ( Ymin == DoubleInfinity )
        Ymin = -1;
    if ( Ymax == -DoubleInfinity )
        Ymax = 1;

    emit newCoords ( Xcursor, Xscale, Ymin, Ymax );
    requestNewLists();
}

void FTSPlotWidget::ExtLayerUpdate ( GL_Layer* mod )
{
    // Here we just record that an external module has
    // requested an update
    // Then we call paintGL which has to make the decision
    // when to issue the update commands down the chain

    // Has already been requested
    if ( updateList.contains ( mod ) )
    {
        return;
    }

    updateList.append ( mod );
    updateNeeded = true;
    update();
}

int FTSPlotWidget::getWindowHeight()
{
    return windowHeight;
}

void FTSPlotWidget::updateCoords ( long double Xcursor, double Xscale, double Ymin, double Ymax )
{
    if ( !is_nan ( Xcursor ) )
        this->Xcursor = Xcursor;
    if ( !is_nan ( Xscale ) )
        this->Xscale = Xscale;
    if ( !is_nan ( Ymin ) )
        this->Ymin = Ymin;
    if ( !is_nan ( Ymax ) )
        this->Ymax = Ymax;
    makeCurrent();
    setupViewportProjection();
    update();
}

void FTSPlotWidget::displaySpot ( long double Xcursor, double Xscale, double Ymin, double Ymax )
{
    if ( !is_nan ( Xscale ) )
        this->Xscale = Xscale;
    if ( !is_nan ( Xcursor ) )
        this->Xcursor = Xcursor - windowWidth * 0.5 * this->Xscale;
    if ( !is_nan ( Ymin ) )
        this->Ymin = Ymin;
    if ( !is_nan ( Ymax ) )
        this->Ymax = Ymax;

    emit newCoords ( this->Xcursor, this->Xscale, this->Ymin, this->Ymax );
    setupViewportProjection();
    update();
}

void FTSPlotWidget::setXRange ( long double Xbegin, long double Xend )
{
    displayRange ( Xbegin, Xend );
}

QPair< long double, long double > FTSPlotWidget::getXRange()
{
    long double Xbegin;
    long double Xend;
    Xbegin = Xcursor;
    Xend = Xcursor + windowWidth * Xscale;
    return QPair<long double, long double> ( Xbegin, Xend );
}

void FTSPlotWidget::setYRange ( double Ymin, double Ymax )
{
    this->Ymin = Ymin;
    this->Ymax = Ymax;
    emit newCoords ( this->Xcursor, this->Xscale, this->Ymin, this->Ymax );
    setupViewportProjection();
    update();
}

QPair< double, double > FTSPlotWidget::getYRange()
{
    return QPair<double, double> ( Ymin, Ymax );
}

void FTSPlotWidget::displayRange ( long double begin, long double end )
{
    Xscale = ( end - begin ) / windowWidth;
    Xcursor = begin;
    setupViewportProjection();
    update();
    emit newCoords ( Xcursor, Xscale, Ymin, Ymax );
}

FTSPlotModulesModel* FTSPlotWidget::dataModel()
{
    return &svm;
}

