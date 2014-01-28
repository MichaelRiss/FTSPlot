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
#include <QDir>
#include "IntervalListLoader.h"

#if defined(BENCHMARK) && defined(COUNT_TILES)
#include "benchmarks/benchmarkHelper.h"
#endif

using namespace FTSPlot;

IntervalListLoader::IntervalListLoader ( QGLWidget* parentarg )
{
    glwidget = parentarg;

    
    myGLwidget = new QGLWidget( glwidget->format(), NULL, glwidget );
    if ( myGLwidget->format() != glwidget->format() )
    {
        qDebug() << "IntervalListLoader: Cannot get the same GL context format as main widget. Exiting!";
        exit(1);
    }
    
    
    glwidget->makeCurrent();
    

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
    moveToThread ( this );
    start();
}

IntervalListLoader::~IntervalListLoader()
{
    quit();
    wait();
    // delete displaylists
    //glwidget->makeCurrent();
    myGLwidget->makeCurrent();
    glDeleteLists ( displayLists[1], 1 );
    glDeleteLists ( displayLists[0], 1 );
    myGLwidget->doneCurrent();
    delete( myGLwidget );
}

void IntervalListLoader::paintGL()
{
    glCallList ( displayLists[useList] );
}

void IntervalListLoader::run()
{
    myGLwidget->makeCurrent();
    exec();
    myGLwidget->doneCurrent();
}

void IntervalListLoader::genDisplayList ( qint64 reqXdataBegin, qint64 reqXdataEnd, int reqDispPower, QString treeDirName, double ymin, double ymax )
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
    
    // call recursive function

    InlineVec<GLdouble> BoxArray;
    InlineVec<GLdouble> LineArray;

    if ( !treeDirName.isEmpty() )
    {
        // read root block file

        QString rootBlockName = treeDirName + ".block";
        QFile blockFile ( rootBlockName );
        if ( blockFile.open ( QIODevice::ReadOnly ) )
        {
            Interval* blockData = ( Interval* ) blockFile.map ( 0, blockFile.size() );
            if ( blockData != NULL )
            {
                qint64 oldBegin = -1;
                qint64 oldEnd = -1;
                qint64 endJ = blockFile.size() / sizeof ( Interval );
                for ( qint64 j = 0; j < endJ; j++ )
                {
                    if ( blockData[j].begin > XdataEnd )
                    {
                        break;
                    }
                    if ( XdataBegin <= blockData[j].end )
                    {
                        qint64 begin = ( (qint64)blockData[j].begin >> reqDispPower) - (reqXdataBegin >> reqDispPower);
                        qint64 end = ( (qint64)blockData[j].end >> reqDispPower) - (reqXdataBegin >> reqDispPower);
                        if ( begin != oldBegin || end != oldEnd )
                        {
                            double xlow = begin;
                            double xhigh = end;
                            
                            //qDebug() << "xlow:" << xlow << "xhigh:" << xhigh;

                            if ( xlow != xhigh )
                            {
                                BoxArray.append( xlow );
                                BoxArray.append( xhigh );
                                LineArray.append( xlow );
                                LineArray.append( xhigh );
                            }
                            else
                            {
                                LineArray.append( xlow );
                            }

                            oldBegin = begin;
                            oldEnd = end;
                        }
                    }
                }
                blockFile.unmap ( ( uchar* ) blockData );
            }
            blockFile.close();
        }

        getRecursiveEvents ( BoxArray, LineArray, XdataBegin, XdataEnd, treeDirName, 0, reqDispPower, reqXdataBegin, ymin, ymax, TOTALHEIGHT );
    }

#ifdef COUNT_TILES
    qint64 vertexCount = 0;
    qint64 lineCount = 0;
    qint64 quadCount = 0;
#endif // COUNT_TILES

    
    
    myGLwidget->makeCurrent();
    glNewList ( displayLists[genList], GL_COMPILE );


    glColor4f ( red, green, blue, 0.5 );
    glBegin ( GL_QUADS );
    for ( long i = 0; i < BoxArray.getMaxIdx(); i += 2 )
    {
        glVertex2d ( BoxArray[i], ymin );
        glVertex2d ( BoxArray[i+1], ymin );
        glVertex2d ( BoxArray[i+1], ymax );
        glVertex2d ( BoxArray[i], ymax );
        
#ifdef COUNT_TILES
        vertexCount += 4;
        quadCount++;
#endif // COUNT_TILES
        
    }
    glEnd();


    glColor4f ( red, green, blue, 1.0 );
    glBegin ( GL_LINES );
    for ( long i = 0; i < LineArray.getMaxIdx(); i++ )
    {
        glVertex2d ( LineArray[i], ymin );
        glVertex2d ( LineArray[i], ymax );
        
#ifdef COUNT_TILES
        vertexCount += 2;
        lineCount++;
#endif // COUNT_TILES 
        
    }
    glEnd();


    glEndList();
    glFlush();
    
#if defined(COUNT_TILES) && !defined(BENCHMARK)
    qDebug() << "IntervalEditorLoader: generated" << vertexCount << "vertexes and" << lineCount << "lines and" << quadCount << "quads.";
#endif // COUNT_TILES
    
#if defined(BENCHMARK) && defined(COUNT_TILES)
    benchmarkHelper::vertexCount = vertexCount;
    benchmarkHelper::lineCount = lineCount;
    benchmarkHelper::quadCount = quadCount;
    
    benchmarkHelper::BoxArray = BoxArray;
    benchmarkHelper::LineArray = LineArray;
#endif
    

    emit notifyListUpdate();
}

void IntervalListLoader::getRecursiveEvents ( InlineVec<GLdouble>& BoxArray, InlineVec<GLdouble>& LineArray, quint64 beginIdx, quint64 endIdx, QString path, quint64 pathValue, int reqDispPower, qint64 reqXdataBegin, double ymin, double ymax, int height )
{
    int power = height2NodePower( height );
    /*
     Traverse down the tree
     If above reqDispPower read all block files and search entries spanning over the desired range and add them
     If below get next node files and add all entries in the desired range
     */

    if ( power > reqDispPower )
    {
        QDir currentDir ( path );
        QStringList subDirList = currentDir.entryList ( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );

        QStringList blockFileList = currentDir.entryList( QStringList( QString( "*.block" ) ), QDir::Files | QDir::NoDotAndDotDot, QDir::Name );

        // iterate over all subdirectories
        int blockFileIdx = 0;
        for ( int i = 0; i < subDirList.size(); i++ )
        {

            // consume all block files before current subdirectory
            QString testblockName = subDirList[i] + ".block";
            while ( blockFileIdx < blockFileList.size() && blockFileList[blockFileIdx] <= testblockName )
            {
                QString stem = blockFileList[blockFileIdx].left( blockFileList[blockFileIdx].size() - 6 );
                quint64 sliceValueBegin = dirName2sliceMin( stem, height-1, pathValue );
                quint64 sliceValueEnd = dirName2sliceMax( stem, height-1, pathValue );

                if ( sliceValueBegin > endIdx )
                {
                    break;
                }
                if ( beginIdx <= sliceValueEnd )
                {
                    // do file
                    QFile blockFile ( path + "/" + blockFileList[blockFileIdx] );
                    if ( blockFile.open ( QIODevice::ReadOnly ) )
                    {
                        Interval* blockData = ( Interval* ) blockFile.map ( 0, blockFile.size() );
                        if ( blockData != NULL )
                        {
                            qint64 oldBegin = -1;
                            qint64 oldEnd = -1;
                            qint64 endJ = blockFile.size() / sizeof ( Interval );
                            for ( quint64 j = 0; j < endJ; j++ )
                            {
                                if ( blockData[j].begin > endIdx )
                                {
                                    break;
                                }
                                if ( beginIdx <= blockData[j].end )
                                {
                                    qint64 begin = ((qint64)blockData[j].begin >> reqDispPower) - (reqXdataBegin >> reqDispPower);
                                    qint64 end = ((qint64)blockData[j].end >> reqDispPower) - (reqXdataBegin >> reqDispPower);
                                    if ( begin != oldBegin || end != oldEnd )
                                    {
                                        double xlow = begin;
                                        double xhigh = end;
                                        if ( xlow != xhigh )
                                        {
                                            BoxArray.append( xlow );
                                            BoxArray.append( xhigh );
                                            LineArray.append( xlow );
                                            LineArray.append( xhigh );
                                        }
                                        else
                                        {
                                            LineArray.append( xlow );
                                        }

                                        oldBegin = begin;
                                        oldEnd = end;
                                    }
                                }
                            }
                            blockFile.unmap ( ( uchar* ) blockData );
                        }
                        blockFile.close();
                    }
                }

                blockFileIdx++;
            }


            // traverse the subdirs

            quint64 sliceValueBegin = dirName2sliceMin( subDirList[i], height-1, pathValue );
            quint64 sliceValueEnd = dirName2sliceMax( subDirList[i], height-1, pathValue );

            if ( sliceValueBegin > endIdx )
            {
                break;
            }
            if ( beginIdx <= sliceValueEnd )
            {
                getRecursiveEvents ( BoxArray, LineArray, beginIdx, endIdx, path + "/" + subDirList[i], sliceValueBegin, reqDispPower, reqXdataBegin, ymin, ymax, height-1 );
            }

        }


        // finishing the rest of the blockfiles
        for ( int i = blockFileIdx; i < blockFileList.size(); i++ )
        {
            QString stem = blockFileList[i].left( blockFileList[i].size() - 6 );
            quint64 sliceValueBegin = dirName2sliceMin( stem, height-1, pathValue );
            quint64 sliceValueEnd = dirName2sliceMax( stem, height-1, pathValue );

            if ( sliceValueBegin > endIdx )
            {
                break;
            }
            if ( beginIdx <= sliceValueEnd )
            {
                // do file
                QFile blockFile ( path + "/" + blockFileList[i] );
                if ( blockFile.open ( QIODevice::ReadOnly ) )
                {
                    Interval* blockData = ( Interval* ) blockFile.map ( 0, blockFile.size() );
                    if ( blockData != NULL )
                    {
                        qint64 oldBegin = -1;
                        qint64 oldEnd = -1;
                        qint64 endJ = blockFile.size() / sizeof ( Interval );
                        for ( quint64 j = 0; j < endJ; j++ )
                        {
                            if ( blockData[j].begin > endIdx )
                            {
                                break;
                            }
                            if ( beginIdx <= blockData[j].end )
                            {
                                qint64 begin = ( (qint64)blockData[j].begin >> reqDispPower) - (reqXdataBegin >> reqDispPower);
                                qint64 end = ( (qint64)blockData[j].end >> reqDispPower) - (reqXdataBegin >> reqDispPower);
                                if ( begin != oldBegin || end != oldEnd )
                                {
                                    double xlow = begin;
                                    double xhigh = end;
                                    if ( xlow != xhigh )
                                    {
                                        BoxArray.append( xlow );
                                        BoxArray.append( xhigh );
                                        LineArray.append( xlow );
                                        LineArray.append( xhigh );
                                    }
                                    else
                                    {
                                        LineArray.append( xlow );
                                    }

                                    oldBegin = begin;
                                    oldEnd = end;
                                }
                            }
                        }
                        blockFile.unmap ( ( uchar* ) blockData );
                    }
                    blockFile.close();
                }
            }
        }

    }
    else
    {
        QFile nodeFile ( path + ".node" );
        if ( nodeFile.open ( QIODevice::ReadOnly ) )
        {
            Interval* nodeData = ( Interval* ) nodeFile.map ( 0, nodeFile.size() );
            if ( nodeData != NULL )
            {
                qint64 oldBegin = -1;
                qint64 oldEnd = -1;
                qint64 endJ = nodeFile.size() / sizeof ( Interval );
                for ( quint64 j = 0; j < endJ; j++ )
                {
                    if ( nodeData[j].begin > endIdx )
                    {
                        break;
                    }
                    if ( beginIdx <= nodeData[j].end )
                    {
                        qint64 begin = ( (qint64)nodeData[j].begin >> reqDispPower) - (reqXdataBegin >> reqDispPower);
                        qint64 end = ( (qint64)nodeData[j].end >> reqDispPower) - (reqXdataBegin >> reqDispPower);
                        if ( begin != oldBegin || end != oldEnd )
                        {
                            double xlow = begin;
                            double xhigh = end;

                            if ( xlow != xhigh )
                            {
                                BoxArray.append( xlow );
                                BoxArray.append( xhigh );
                                LineArray.append( xlow );
                                LineArray.append( xhigh );
                            }
                            else
                            {
                                LineArray.append( xlow );
                            }

                            oldBegin = begin;
                            oldEnd = end;
                        }
                    }
                }
                nodeFile.unmap ( ( uchar* ) nodeData );
            }
            nodeFile.close();
        }
    }
}

void IntervalListLoader::setColor ( QColor color )
{
    myColor = color;
    red = myColor.redF();
    green = myColor.greenF();
    blue = myColor.blueF();
}

void IntervalListLoader::toggleLists()
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

void IntervalListLoader::eventLoopAlive()
{
    int myTicket = ++eventLoopTestCounter0;
    emit checkEventLoopSignal ( myTicket );
    lock.lock();
    while ( eventLoopTestCounter1 != myTicket )
    {
        waitCond.wait ( &lock );
    }
    lock.unlock();
}

void IntervalListLoader::checkEventLoop ( int counter )
{
    lock.lock();
    eventLoopTestCounter1 = counter;
    waitCond.wakeAll();
    lock.unlock();
}

IntervalListLoader_Suspend::IntervalListLoader_Suspend ( IntervalListLoader* lockarg )
{
    lock = lockarg;
    lock->quit();
    lock->wait();
}

IntervalListLoader_Suspend::~IntervalListLoader_Suspend()
{
    lock->start();
}


// kate: indent-mode cstyle; space-indent on; indent-width 0; 
