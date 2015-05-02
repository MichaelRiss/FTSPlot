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
#include <QFile>
#include <QTime>
#include <QDir>
#include "EventEditorLoader.h"

#if defined(BENCHMARK) && defined(COUNT_TILES)
#include "benchmarks/benchmarkHelper.h"
#endif

#define MAX(a,b) a<b?b:a
#define MIN(a,b) a<b?a:b

using namespace FTSPlot;

EventEditorLoader::EventEditorLoader ( QOpenGLContext* glcontext ) :
		red( 0.0 ), green( 0.0 ), blue( 0.0 )
{
    //glwidget = parent;
	myOpenGLContext = new QOpenGLContext( this );
	myOpenGLContext->setFormat( glcontext->format() );
	myOpenGLContext->setScreen( glcontext->screen() );
	myOpenGLContext->setShareContext( glcontext->shareContext() );
	if( !myOpenGLContext->create() ){
		qDebug() << "Cannot create new context.";
		exit(1);
	}


    //myGLwidget = new QGLWidget( glwidget->format(), NULL, glwidget );
    //if ( myGLwidget->format() != glwidget->format() )
	if( !QOpenGLContext::areSharing( glcontext, myOpenGLContext ) )
    {
        qDebug() << "EventEditorLoader: Cannot get the same GL context format as main widget. Exiting!";
        exit(1);
    }

    //glwidget->makeCurrent();
	myOpenGLContext->makeCurrent(  glcontext->surface() );
    //myGLwidget->makeCurrent();

    // We also have to initialize this OpenGL Context to use alpha blending for example
    glClearColor ( 1.0, 1.0, 1.0, 1.0 );
    glShadeModel ( GL_FLAT );

    glEnable ( GL_LINE_SMOOTH );
    glEnable ( GL_BLEND );
    glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glHint ( GL_LINE_SMOOTH_HINT, GL_DONT_CARE );
    glLineWidth ( 1.0 );

    displayLists[0] = glGenLists ( 1 );
    displayLists[1] = glGenLists ( 1 );
    if ( displayLists[0] == 0 || displayLists[1] == 0 )
    {
        qDebug() << "Error: Cannot reserve display list. Exiting.";
        exit ( 1 );
    }


    useList = 0;
    genList = 1;
    eventLoopTestCounter0 = 0;
    eventLoopTestCounter1 = 0;

    connect ( this, SIGNAL ( checkEventLoopSignal ( int ) ), this, SLOT ( checkEventLoop ( int ) ) );
    myOpenGLContext->doneCurrent();
    //moveToThread ( this );
    //start();
}

EventEditorLoader::~EventEditorLoader()
{
    //quit();
    //wait();
    // delete displaylists
    //glwidget->makeCurrent();
    //myGLwidget->makeCurrent();
    myOpenGLContext->makeCurrent( myOpenGLContext->surface() );
    glDeleteLists ( displayLists[1], 1 );
    glDeleteLists ( displayLists[0], 1 );
    //myGLwidget->doneCurrent();
    myOpenGLContext->doneCurrent();
    delete( myOpenGLContext );
    //delete( myGLwidget );
}


void EventEditorLoader::paintGL()
{
    glColor3f ( red, green, blue );
    glCallList ( displayLists[useList] );
}

void EventEditorLoader::genDisplayList ( qint64 reqXdataBegin, qint64 reqXdataEnd, int reqDispPower, QString baseDirName, double ymin, double ymax )
{
    quint64 XdataBegin;
    quint64 XdataEnd;
    if ( reqXdataBegin < 0 )
    {
        XdataBegin = 0;
    }
    else
    {
        XdataBegin = reqXdataBegin;
    }
    if ( reqXdataEnd < 0 )
    {
        XdataEnd = 0;
    }
    else
    {
        XdataEnd = reqXdataEnd;
    }

#ifdef COUNT_TILES
    vertexCount = 0;
    lineCount = 0;
#endif // COUNT_TILES
    
    if( !myOpenGLContext->isValid() ){
    	myOpenGLContext->makeCurrent( myOpenGLContext->surface() );
    	if( !myOpenGLContext->isValid() ){
    		qDebug() << "Invalid GL context!";
    		return;
    	}
    }
    //myGLwidget->makeCurrent();
    // call recursive function
    glNewList ( displayLists[genList], GL_COMPILE );
    if ( !baseDirName.isEmpty() )
    {
        glBegin ( GL_LINES );
        getRecursiveEvents ( XdataBegin, XdataEnd, baseDirName, 0, TOTALHEIGHT, reqDispPower, reqXdataBegin, ymin, ymax );
        glEnd();
    }
    glEndList();
    glFlush();
    
#if defined(COUNT_TILES) && !defined(BENCHMARK)
    qDebug() << "EventEditorLoader: generated" << vertexCount << "vertexes and" << lineCount << "lines.";
#endif

#if defined(BENCHMARK) && defined(COUNT_TILES)
    benchmarkHelper::vertexCount = vertexCount;
    benchmarkHelper::lineCount = lineCount;
    benchmarkHelper::quadCount = 0;
#endif

    emit notifyListUpdate();
}


void EventEditorLoader::getRecursiveEvents ( quint64 beginIdx, quint64 endIdx, QString path, quint64 pathValue, int height, int reqDispPower, qint64 reqXdataBegin, double ymin, double ymax )
{
    // height == 0 => read blockfile
    if ( height == 0 )
    {
        QFile blockFile ( path );
        if ( !blockFile.open ( QIODevice::ReadOnly ) )
        {
            qDebug() << "Cannot open" << blockFile.fileName();
            return;
        }
        // map file
        quint64* blockData = ( quint64* ) blockFile.map ( 0, blockFile.size() );
        if ( blockData == NULL )
        {
            qDebug() << "Cannot map" << blockFile.fileName();
            return;
        }

        // read and copy all values between beginIdx and endIdx to displayList
        qint64 oldValue = -1;
        qint64 maxblockFileIdx = blockFile.size() / sizeof( quint64 );
        for ( int i = 0; i < maxblockFileIdx; i++ )
        {
            if ( endIdx < blockData[i] )
            {
                break;
            }
            if ( beginIdx <= blockData[i] )
            {
                qint64 newValue = ( blockData[i] - reqXdataBegin ) >> reqDispPower;
                if ( newValue != oldValue )
                {
                    double xcoord = newValue;
                    glVertex2d ( xcoord, ymin );
                    glVertex2d ( xcoord, ymax );

#ifdef COUNT_TILES
                    vertexCount += 2;
                    lineCount++;
#endif // COUNT_TILES

                    oldValue = newValue;
                }
            }
        }

        // unmap and close file
        blockFile.unmap ( ( uchar* ) blockData );
        blockFile.close();
    }
    // height == 1 => either node file or recurse into block files
    else if ( height == 1 )
    {
        if ( reqDispPower >= height2NodePower( height ) )
        {
            // use node file
            QFile nodeFile( path + ".node" );
            if ( !nodeFile.open( QIODevice::ReadOnly ) )
            {
                qDebug() << "Cannot open" << nodeFile.fileName();
                return;
            }
            // map cache file
            quint64* nodeData = ( quint64* ) nodeFile.map ( 0, nodeFile.size() );
            if ( nodeData == NULL )
            {
                qDebug() << "Cannot map" << nodeFile.fileName();
                return;
            }
            // read cache file
            qint64 oldValue = -1;
            qint64 maxNodeFileIdx = nodeFile.size() / sizeof ( quint64 );
            for ( int i = 0; i < maxNodeFileIdx; i++ )
            {
                // add values within limits to display list
                if ( endIdx < nodeData[i] )
                {
                    break;
                }
                if ( beginIdx <= nodeData[i] )
                {
                    qint64 newValue = ( nodeData[i] - reqXdataBegin ) >> reqDispPower;
                    if ( newValue != oldValue )
                    {
                        double xcoord = newValue;
                        glVertex2d ( xcoord, ymin );
                        glVertex2d ( xcoord, ymax );

#ifdef COUNT_TILES
                        vertexCount += 2;
                        lineCount++;
#endif // COUNT_TILES                        

                        oldValue = newValue;
                    }
                }
            }
        }
        else
        {
            // recurse down to block files
            QDir currentDir ( path );
            currentDir.setNameFilters( QStringList( QString( "*.block" ) ) );
            QStringList blockFileList = currentDir.entryList ( QDir::Files | QDir::NoDotAndDotDot, QDir::Name );

            for ( int i = 0; i < blockFileList.size(); i++ )
            {
                QString coreName = blockFileList[i];
                coreName.chop( 6 );

                quint64 sliceValueBegin = dirName2sliceMin( coreName, height-1, pathValue );
                quint64 sliceValueEnd = dirName2sliceMax( coreName, height-1, pathValue );

                if ( sliceValueBegin > endIdx )
                {
                    break;
                }
                if ( beginIdx <= sliceValueEnd )
                {
                    getRecursiveEvents ( beginIdx, endIdx, path + "/" + blockFileList[i], sliceValueBegin, height-1, reqDispPower, reqXdataBegin, ymin, ymax );
                }
            }
        }
    }
    // height > 1 => either node file or recurse into dirs
    else
    {
        if ( reqDispPower >= height2NodePower( height ) )
        {
            // use node file
            QFile nodeFile( path + ".node" );
            if ( !nodeFile.open( QIODevice::ReadOnly ) )
            {
                qDebug() << "Cannot open" << nodeFile.fileName();
                return;
            }
            // map cache file
            quint64* nodeData = ( quint64* ) nodeFile.map ( 0, nodeFile.size() );
            if ( nodeData == NULL )
            {
                qDebug() << "Cannot map" << nodeFile.fileName();
                return;
            }
            // read cache file
            qint64 oldValue = -1;
            qint64 maxNodeFileIdx = nodeFile.size() / sizeof ( quint64 );
            for ( int i = 0; i < maxNodeFileIdx; i++ )
            {
                // add values within limits to display list
                if ( endIdx < nodeData[i] )
                {
                    break;
                }
                if ( beginIdx <= nodeData[i] )
                {
                    qint64 newValue = ( nodeData[i] - reqXdataBegin ) >> reqDispPower;
                    if ( newValue != oldValue )
                    {
                        double xcoord = newValue;
                        glVertex2d ( xcoord, ymin );
                        glVertex2d ( xcoord, ymax );

#ifdef COUNT_TILES
                        vertexCount += 2;
                        lineCount++;
#endif // COUNT_TILES                        

                        oldValue = newValue;
                    }
                }
            }
        }
        else
        {
            // recurse into subdirs
            QDir currentDir ( path );
            QStringList dirNames = currentDir.entryList ( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );

            for ( int i = 0; i < dirNames.size(); i++ )
            {
                quint64 sliceValueBegin = dirName2sliceMin( dirNames[i], height-1, pathValue );
                quint64 sliceValueEnd = dirName2sliceMax( dirNames[i], height-1, pathValue );

                if ( sliceValueBegin > endIdx )
                {
                    break;
                }
                if ( beginIdx <= sliceValueEnd )
                {
                    getRecursiveEvents ( beginIdx, endIdx, path + "/" + dirNames[i], sliceValueBegin, height-1, reqDispPower, reqXdataBegin, ymin, ymax );
                }
            }
        }
    }
}


//void EventEditorLoader::run()
//{
//    myGLwidget->makeCurrent();
//    exec();
//    myGLwidget->doneCurrent();
//}

void EventEditorLoader::setColor ( QColor color )
{
    myColor = color;
    red = myColor.redF();
    green = myColor.greenF();
    blue = myColor.blueF();
}

void EventEditorLoader::toggleLists()
{
    if ( useList == 0 )
    {
        useList = 1;
        genList = 0;
    }
    else
    {
        useList = 0;
        genList = 1;
    }
}


//void EventEditorLoader::eventLoopAlive()
//{
//    int myTicket = ++eventLoopTestCounter0;
//    emit checkEventLoopSignal ( myTicket );
//    lock.lock();
//    while ( eventLoopTestCounter1 != myTicket )
//    {
//        waitCond.wait ( &lock );
//    }
//    lock.unlock();
//}
//
//
//void EventEditorLoader::checkEventLoop ( int counter )
//{
//    lock.lock();
//    eventLoopTestCounter1 = counter;
//    waitCond.wakeAll();
//    lock.unlock();
//}


EventEditorLoader_Suspend::EventEditorLoader_Suspend ( QThread* thread )
{
    this->lock = thread;
    lock->quit();
    lock->wait();
}

EventEditorLoader_Suspend::~EventEditorLoader_Suspend()
{
    lock->start();
}


// kate: indent-mode cstyle; space-indent on; indent-width 0; 
