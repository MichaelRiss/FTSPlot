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


#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QBitmap>
#include "commonDefs.h"
#include "IntervalEditor.h"
#include "DeleteCursorData.xbm"

using namespace FTSPlot;

IntervalEditor::IntervalEditor ( SimpleViewWidget* plotWidget )
{
    gui = new HideNotifyWidget;
    ui.setupUi ( gui );

    connect ( ui.openIntervalListButton, SIGNAL ( clicked() ),
              this, SLOT ( openIntervalList() ) );
    connect ( ui.importButton, SIGNAL ( clicked() ),
              this, SLOT ( importFlatFileSlot() ) );
    connect ( ui.exportButton, SIGNAL ( clicked() ),
              this, SLOT ( exportFlatFileSlot() ) );
    connect ( ui.nextButton, SIGNAL ( clicked() ),
              this, SLOT ( nextIntervalSlot() ) );
    connect ( ui.previousButton, SIGNAL ( clicked() ),
              this, SLOT ( prevIntervalSlot() ) );
    connect ( ui.lowerManualAdd, SIGNAL ( editingFinished() ),
              this, SLOT ( validateManualLow () ) );
    connect ( ui.higherManualAdd, SIGNAL ( editingFinished() ),
              this, SLOT ( validateManualHigh () ) );
    connect ( ui.manualAddButton, SIGNAL ( clicked() ),
              this, SLOT ( manualAddInterval() ) );
    connect ( ui.manualDeleteButton, SIGNAL ( clicked ( bool ) ),
              this, SLOT ( manualDelInterval() ) );
    connect ( ui.lowerMinusButton, SIGNAL ( clicked ( bool ) ),
              this, SLOT ( lowerMinusHandler() ) );
    connect ( ui.lowerPlusButton, SIGNAL ( clicked ( bool ) ),
              this, SLOT ( lowerPlusHandler() ) );
    connect ( ui.higherMinusButton, SIGNAL ( clicked ( bool ) ),
              this, SLOT ( upperMinusHandler() ) );
    connect ( ui.higherPlusButton, SIGNAL ( clicked ( bool ) ),
              this, SLOT ( upperPlusHandler() ) );
    connect ( ui.lowerFineTuneLine, SIGNAL ( editingFinished() ),
              this, SLOT ( validateFineTuneLow() ) );
    connect ( ui.higherFineTuneLine, SIGNAL ( editingFinished() ),
              this, SLOT ( validateFineTuneHigh() ) );
    connect ( ui.lowerFineTuneLine, SIGNAL ( returnPressed() ),
              this, SLOT ( handleFineTuneLow () ) );
    connect ( ui.higherFineTuneLine, SIGNAL ( returnPressed() ),
              this, SLOT ( handleFineTuneHigh () ) );
    connect ( ui.trackBox, SIGNAL ( stateChanged ( int ) ),
              this, SLOT ( toggleTrackBox ( int ) ) );
    connect ( ui.fitScaleBox, SIGNAL ( stateChanged ( int ) ),
              this, SLOT ( toggleFitBox ( int ) ) );
    connect ( gui, SIGNAL ( GUIupdate() ), this, SLOT ( relayGUIupdate() ) );

    gui->setAttribute ( Qt::WA_QuitOnClose, false );
    gui->show();

    svw = plotWidget;
    ill = new IntervalListLoader ( svw );

    connect ( ill, SIGNAL ( notifyListUpdate() ),
              this, SLOT ( threadDone() ) );
    connect ( this, SIGNAL ( requestDisplayList ( qint64,qint64,int,QString,double,double ) ),
              ill, SLOT ( genDisplayList ( qint64,qint64,int,QString,double,double ) ) );


    QBitmap bitmap = QBitmap::fromData ( QSize ( DeleteCursorData_width, DeleteCursorData_height ),
                                         DeleteCursorData_bits );
    QBitmap mask = QBitmap::fromData ( QSize ( DeleteCursorData_width, DeleteCursorData_height ),
                                       DeleteCursorData_bits );

    DeleteCursor = QCursor ( bitmap, mask );

    state = NoTree;
    updateGUI();
    currentInterval.begin = 0;
    currentInterval.end = 1;
    intervalListDirName = "";
    flatFileName = "";
    XdataBegin = 0;
    displayPower = 0;
    firstPointSet = false;
    firstPoint = 0;
    currentPoint = 1;
}

IntervalEditor::~IntervalEditor()
{
    // disconnect some signals to avoid stupid accidents
    disconnect ( ill, SIGNAL ( notifyListUpdate() ), this, SLOT ( threadDone() ) );
    // stop & delete thread
    delete ( ill );
    // delete gui widget
    delete ( gui );
}


void IntervalEditor::paintGL()
{
    if ( state == normal )
    {
        svw->makeCurrent();
        ill->paintGL();
    }

    if ( firstPointSet )
    {
        double xlow = ( ( qint64 ) firstPoint - XdataBegin ) >> displayPower;
        double xhigh = ( ( qint64 ) currentPoint - XdataBegin ) >> displayPower;

        glColor4f ( 1-red, 1-green, 1-blue, 0.5 );
        glBegin ( GL_QUADS );
        glVertex2d ( xlow, ymin );
        glVertex2d ( xhigh, ymin );
        glVertex2d ( xhigh, ymax );
        glVertex2d ( xlow, ymax );
        glEnd();

        glColor4f ( red, green, blue, 1.0 );
        glBegin ( GL_LINES );
        glVertex2d ( xlow, ymin );
        glVertex2d ( xlow, ymax );
        glVertex2d ( xhigh, ymin );
        glVertex2d ( xhigh, ymax );
        glEnd();
    }

    // light up currentInterval
    if ( state == normal )
    {
        double xlow = ( ( qint64 ) currentInterval.begin - XdataBegin ) >> displayPower;
        double xhigh = ( ( qint64 ) currentInterval.end - XdataBegin ) >> displayPower;

        glColor4f ( 1-green, 1-blue, 1-red, 0.5 );
        glBegin ( GL_QUADS );
        glVertex2d ( xlow, ymin );
        glVertex2d ( xhigh, ymin );
        glVertex2d ( xhigh, ymax );
        glVertex2d ( xlow, ymax );
        glEnd();

        glColor4f ( red, green, blue, 1.0 );
        glBegin ( GL_LINES );
        glVertex2d ( xlow, ymin );
        glVertex2d ( xlow, ymax );
        glVertex2d ( xhigh, ymin );
        glVertex2d ( xhigh, ymax );
        glEnd();
    }
}

void IntervalEditor::genDisplayList ( qint64 reqXdataBeginArg, qint64 reqXdataEndArg, int reqDispPowerArg, double reqYFrustMin, double reqYFrustMax )
{
    QString path = treeRootDirName;
    if ( state == NoTree )
    {
        path = "";
    }

    // get ymin and ymax
    ymin = reqYFrustMin;
    ymax = reqYFrustMax;
    reqXdataBegin = reqXdataBeginArg;
    reqDisplayPower = reqDispPowerArg;

    emit requestDisplayList ( reqXdataBegin, reqXdataEndArg, reqDispPowerArg, path, ymin, ymax );
}

void IntervalEditor::toggleLists()
{
    ill->toggleLists();
    XdataBegin = reqXdataBegin;
    displayPower = reqDisplayPower;
}

double IntervalEditor::getMin()
{
    return GL_Layer::getMin();
}

double IntervalEditor::getMax()
{
    return GL_Layer::getMax();
}

bool IntervalEditor::isOverInterval()
{
    // Get screen coordinates
    int mouseX = svw->mapFromGlobal( QCursor::pos() ).x();
    // convert them +/- 1 to graph coordinates
    qint64 xlow = qMax( ( qint64 ) round ( svw->Xscreen2graph ( mouseX - 1 ) ), Q_INT64_C( 0 ) );
    qint64 xhigh = ( qint64 ) round ( svw->Xscreen2graph ( mouseX + 1 ) );

    Interval dummy;
    bool overInterval;
    if ( xhigh < 0 )
    {
        overInterval = false;
    }
    else
    {
        overInterval = findInterval ( dummy, xlow, xhigh );
    }
    return overInterval;
}


void IntervalEditor::mouseMoveEvent ( QMouseEvent* eventArg )
{
    if ( ui.addIntervalButton->isChecked() && firstPointSet )
    {
        currentPoint = svw->Xscreen2graph ( eventArg->x() );
        emit triggerRepaint ( this );
    }
    if ( eventArg->modifiers() == Qt::ShiftModifier )
    {
        // if add mode
        if ( ui.addIntervalButton-> isChecked() )
        {
            if ( svw->cursor().shape() != Qt::CrossCursor )
            {
                svw->setCursor ( Qt::CrossCursor );
            }
        }
        // if select mode
        if ( ui.selectIntervalButton->isChecked() )
        {
            bool overInterval = isOverInterval();

            if ( overInterval && svw->cursor().shape() != Qt::PointingHandCursor )
            {
                svw->setCursor ( Qt::PointingHandCursor );
            }

            if ( !overInterval && svw->cursor().shape() != Qt::CrossCursor )
            {
                svw->setCursor ( Qt::CrossCursor );
            }
        }

        // if delete mode
        if ( ui.delIntervalButton->isChecked() )
        {
            bool overInterval = isOverInterval();

            if ( overInterval && svw->cursor().shape() != Qt::BitmapCursor )
            {
                svw->setCursor ( DeleteCursor );
            }
            
            if ( !overInterval && svw->cursor().shape() != Qt::CrossCursor )
            {
                svw->setCursor ( Qt::CrossCursor );
            }
        }
        eventArg->accept();
    }
    else
    {
        if ( svw->cursor().shape() != Qt::ArrowCursor )
        {
            svw->setCursor ( Qt::ArrowCursor );
        }
        eventArg->ignore();
    }
}

void IntervalEditor::mousePressEvent ( QMouseEvent* eventArg )
{
    if ( eventArg->modifiers() == Qt::ShiftModifier )
    {
        if ( ui.addIntervalButton->isChecked() )
        {
            qint64 xpos = ( qint64 ) round ( svw->Xscreen2graph ( eventArg->x() ) );
            if ( !firstPointSet )
            {
                // so this is the first point we have to set
                // we do this into a temporary variable, without
                // an update to the model
                firstPoint = xpos;
                currentPoint = xpos;
                firstPointSet = true;
                emit triggerRepaint ( this );
            }
            else
            {
                // this is the second point, so we store the new record
                // in the model
                Interval inter;
                inter.begin = qMin ( firstPoint, xpos );
                inter.end = qMax ( firstPoint, xpos );

                addInterval ( inter );
                currentInterval = inter;
                updateGUI();

                firstPointSet = false;
                emit triggerRepaint ( this );
            }
        }

        if ( ui.selectIntervalButton->isChecked() )
        {
            // Get screen coordinates
            int screenx = eventArg->x();
            // convert them +/- 1 to graph coordinates

            quint64 xlow;
            if ( round ( svw->Xscreen2graph ( screenx - 1 ) ) < 0 )
            {
                xlow = 0;
            }
            else
            {
                xlow = ( quint64 ) round ( svw->Xscreen2graph ( screenx - 1 ) );
            }
            quint64 xhigh;
            if ( round ( svw->Xscreen2graph ( screenx + 1 ) ) < 0 )
            {
                xhigh = 0;
            }
            else
            {
                xhigh = ( quint64 ) round ( svw->Xscreen2graph ( screenx + 1 ) );
            }

            // check this interval for events
            Interval inter;
            if ( findInterval( inter, xlow, xhigh ) )
            {
                // good, we have an event in the interval
                currentInterval = inter;
                trackUpdate();
                updateGUI();
                emit triggerRepaint ( this );
            }
        }

        if ( ui.delIntervalButton->isChecked() )
        {
            // Get screen coordinates
            int screenx = eventArg->x();
            // convert them +/- 1 to graph coordinates
            qint64 xlow = ( qint64 ) round ( svw->Xscreen2graph ( screenx - 1 ) );
            qint64 xhigh = ( qint64 ) round ( svw->Xscreen2graph ( screenx + 1 ) );
            // check this interval for events - linear for now

            Interval toDelete;
            if ( findInterval ( toDelete, xlow, xhigh ) )
            {
                delInterval ( toDelete );
            }

            emit triggerRepaint ( this );
        }

        eventArg->accept();
    }
    else
    {
        eventArg->ignore();
    }
}

void IntervalEditor::wheelEvent ( QWheelEvent* eventArg )
{
    GL_Layer::wheelEvent ( eventArg );
}


void IntervalEditor::keyPressEvent ( QKeyEvent* eventArg )
{
    if ( eventArg->key() == Qt::Key_Shift )
    {
        if ( isOverInterval() )
        {
            if ( ui.addIntervalButton->isChecked() )
            {
                svw->setCursor( Qt::CrossCursor );
            }
            if ( ui.selectIntervalButton->isChecked() )
            {
                svw->setCursor( Qt::PointingHandCursor );
            }
            if ( ui.delIntervalButton->isChecked() )
            {
                svw->setCursor( DeleteCursor );
            }
        }
        else
        {
            svw->setCursor ( Qt::CrossCursor );
        }
        eventArg->accept();
    }
    else
    {
        eventArg->ignore();
    }
}

void IntervalEditor::keyReleaseEvent ( QKeyEvent* eventArg )
{
    if ( eventArg->key() == Qt::Key_Shift )
    {
        svw->setCursor ( Qt::ArrowCursor );
        eventArg->accept();
    }
    else
    {
        eventArg->ignore();
    }
}

void IntervalEditor::setColor ( QColor color )
{
    myColor = color;
    red = myColor.redF();
    green = myColor.greenF();
    blue = myColor.blueF();
    ill->setColor ( color );
    emit triggerRepaint( this );
}

QString IntervalEditor::getPath()
{
    return treeRootDirName;
}

bool IntervalEditor::openTreeDir ( QString TreeDirPath )
{
    if ( state != NoTree )
    {
        return false;
    }

    intervalListDirName = QDir( TreeDirPath ).absolutePath();

    // either empty or having a "root" directory

    QDir intervalListDir ( intervalListDirName );
    if ( intervalListDir.count() == 2 ) // if empty ...
    {
        // ... create the "root" directory
        if ( !intervalListDir.mkdir ( "root" ) )
        {
            return false;
        }
    }

    // check for "root" dir
    QFileInfo treeRootInfo( intervalListDirName + "/root" );
    if ( !treeRootInfo.isDir() )
    {
        QMessageBox MyMessBox;
        MyMessBox.setText ( "Root directory " + intervalListDirName + "/root missing." );
        MyMessBox.exec();
        return false;
    }
    treeRootDirName = intervalListDirName + "/root";

    QDir treeDir ( treeRootDirName );
    if ( treeDir.entryInfoList ( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name ).size() == 0 )
    {
        state = EmptyTree;
    }
    else
    {
        // search first entry
        Interval firstInterval;
        if ( findFirstInterval ( firstInterval ) )
        {
            currentInterval = firstInterval;
            state = normal;
        }
        else
        {
            QMessageBox MyMessBox;
            MyMessBox.setText ( "Malformed directory tree: " + treeRootDirName );
            MyMessBox.exec();
            return false;
        }
    }

    updateGUI();
    disconnect ( ui.openIntervalListButton, SIGNAL ( clicked() ),
                 this, SLOT ( openIntervalList() ) );
    connect ( ui.openIntervalListButton, SIGNAL ( clicked() ),
              this, SLOT ( closeIntervalList() ) );
    originalModuleName = svw->getModuleName( this );
    QFileInfo intervalListBaseDir( intervalListDirName );
    if( !svw->setModuleName( this, intervalListBaseDir.baseName() ) )
    {
        qDebug() << "Cannot set module name.";
    }
    
    // if dataset is not empty signal to ViewWidget to update the displaylists
    if ( state == normal )
    {
        emit ( triggerRepaint ( this ) );
    }
    return true;
}


void IntervalEditor::openIntervalList()
{
    IntervalListLoader_Suspend lock ( ill );
    intervalListDirName =
        QFileDialog::getExistingDirectory ( NULL, "IntervalList Tree Directory", intervalListDirName, QFileDialog::ShowDirsOnly );

    if ( !intervalListDirName.isEmpty() )
    {
        openTreeDir ( intervalListDirName );
    }
}

bool IntervalEditor::closeTreeDir()
{
    if ( state == NoTree )
    {
        return false;
    }
    closeIntervalList();
    return true;
}

void IntervalEditor::closeIntervalList()
{
    IntervalListLoader_Suspend lock ( ill );
    state = NoTree;
    updateGUI();
    disconnect ( ui.openIntervalListButton, SIGNAL ( clicked() ),
                 this, SLOT ( closeIntervalList() ) );
    connect ( ui.openIntervalListButton, SIGNAL ( clicked() ),
              this, SLOT ( openIntervalList() ) );
    if( !svw->setModuleName( this, originalModuleName ) )
    {
        qDebug() << "Cannot reset module name.";
    }
    emit triggerRepaint ( this );
}

bool IntervalEditor::findFirstInterval(Interval& inter)
{
    QVector<Interval> result = findFirstInterval( treeRootDirName );
    if ( !result.isEmpty() )
    {
        qSort( result );
        inter = result.first();
        return true;
    }
    else
    {
        return false;
    }
}


QVector<Interval> IntervalEditor::findFirstInterval ( QString path )
{
    QVector<Interval> result;

    // get list of subdirs
    QDir currentDir( path );
    QStringList subDirList = currentDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
    // get list of blockfiles
    QStringList blockFileList = currentDir.entryList( QStringList( QString( "*.block" ) ), QDir::Files | QDir::NoDotAndDotDot, QDir::Name );

    // if( only blockfiles or first blockfile <= first subdir )
    //          add first entry in blockfile and return
    if ( ( !blockFileList.isEmpty() && subDirList.isEmpty() ) ||
            ( !blockFileList.isEmpty() && !subDirList.isEmpty() && blockFileList.first().left( blockFileList.first().size() - 6 ) <= subDirList.first() ) )
    {
        QFile blockFile( path + "/" + blockFileList.first() );
        if ( blockFile.open( QIODevice::ReadOnly ) )
        {
            Interval* blockMap = (Interval*) blockFile.map( 0, blockFile.size() );
            if ( blockMap != NULL )
            {
                result += blockMap[0];
            }
            blockFile.close();
        }
    }

    // if( only subdirs or first subdir <= first blockfile )
    //          go down into subdir
    if ( ( !subDirList.isEmpty() && blockFileList.isEmpty() ) ||
            ( !subDirList.isEmpty() && !blockFileList.isEmpty() && subDirList.first() <= blockFileList.first().left( blockFileList.first().size() - 6 ) ) )
    {
        result += findFirstInterval( path + "/" + subDirList.first() );
    }

    return result;
}

bool IntervalEditor::findLastInterval(Interval& inter)
{
    QVector<Interval> result = findLastInterval( treeRootDirName );
    if ( !result.isEmpty() )
    {
        qSort( result );
        inter = result.last();
        return true;
    }
    else
    {
        return false;
    }
}


QVector<Interval> IntervalEditor::findLastInterval ( QString path )
{
    QVector<Interval> result;

    // get list of subdirs
    QDir currentDir( path );
    QStringList subDirList = currentDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
    // get list of blockfiles
    QStringList blockFileList = currentDir.entryList( QStringList( QString( "*.block" ) ), QDir::Files | QDir::NoDotAndDotDot, QDir::Name );

    // if( only blockfiles or first blockfile >= first subdir )
    //          add last entry in blockfile and return
    if ( ( !blockFileList.isEmpty() && subDirList.isEmpty() ) ||
            ( !blockFileList.isEmpty() && !subDirList.isEmpty() && blockFileList.last().left( blockFileList.last().size() - 6 ) >= subDirList.last() ) )
    {
        QFile blockFile( path + "/" + blockFileList.last() );
        if ( blockFile.open( QIODevice::ReadOnly ) )
        {
            Interval* blockMap = (Interval*) blockFile.map( 0, blockFile.size() );
            if ( blockMap != NULL )
            {
                qint64 lastIntervalIdx = ( blockFile.size() / sizeof( Interval ) ) - 1;
                result += blockMap[lastIntervalIdx];
            }
            blockFile.close();
        }
    }

    // if( only subdirs or first subdir <= first blockfile )
    //          go down into subdir
    if ( ( !subDirList.isEmpty() && blockFileList.isEmpty() ) ||
            ( !subDirList.isEmpty() && !blockFileList.isEmpty() && subDirList.last() >= blockFileList.last().left( blockFileList.last().size() - 6 ) ) )
    {
        result += findLastInterval( path + "/" + subDirList.last() );
    }

    return result;
}

bool IntervalEditor::importFlatFileMain ( QString flatFileName ) throw ( QString )
{
    // delete directory tree
    if ( RemoveDirectory ( treeRootDirName ) )
    {
        throw QString( "Cannot clean " + treeRootDirName );
        return false;
    }
    // delete "root.node" file
    QFile nodeFile( treeRootDirName + ".node" );
    if ( nodeFile.exists() )
    {
        if ( !nodeFile.remove() )
        {
            throw QString( "Cannot delete " + treeRootDirName + ".node" );
            return false;
        }
    }

    // open flat file
    QFile flatFile ( flatFileName );
    if ( !flatFile.open ( QIODevice::ReadOnly ) )
    {
        throw QString( "Cannot open " +  flatFileName + ": " + flatFile.errorString() );
        return false;
    }

    if ( flatFile.size() == 0 )
    {
        state = EmptyTree;
        return true;
    }
    else
    {
        // mmap the flat file
        Interval* flatDataField = ( Interval* ) flatFile.map ( 0, flatFile.size() );
        if ( flatDataField == NULL )
        {
            throw QString( "Cannot mmap " + flatFileName + "." );
            return false;
        }

        // Why this complicated and not just statically defining it?
        // Depending on BLOCKFACTOR and TOTALHEIGHT cacheLine can become quite large,
        // larger than the stack limit => therefore we instantiate it on the heap
        QVector<Interval> blockCache[TOTALHEIGHT+1];
        QVector<Interval> nodeCache[TOTALHEIGHT+1];
        quint64 blockMaskCache[TOTALHEIGHT+1];
        int lowBlockMaskIdx = TOTALHEIGHT+2;


        for ( int i = 0; i < TOTALHEIGHT+1; i++ )
        {
            blockMaskCache[i] = height2mask(i);
        }
        QFile blockFile;

        quint64 endIdx = flatFile.size() / sizeof ( Interval );
        Interval oldValue;
        oldValue.begin = 0;
        oldValue.end = 0;
        for ( quint64 idx = 0; idx < endIdx; idx++ )
        {
            //read value
            Interval value = flatDataField[idx];
            if ( value < oldValue )
            {
                throw QString( "Interval values in Flatfile have to be ordered ascendingly! Exiting." );
                return false;
            }
            else
            {
                oldValue = value;
            }

            // add value
            blockCache[interval2height(value)].append( value );
            if ( interval2height(value) < lowBlockMaskIdx )
            {
                lowBlockMaskIdx = interval2height( value );
            }

            // do we have to write out a block of data?
            if ( idx+1 == endIdx || (flatDataField[idx+1].begin & blockMaskCache[lowBlockMaskIdx]) != (value.begin & blockMaskCache[lowBlockMaskIdx]) )
            {
                // create directories
                QDir treeDir ( treeRootDirName );
                if ( !treeDir.mkpath ( generatePath( value, lowBlockMaskIdx+1 )) )
                {
                    throw QString( "Cannot open create directory path " + generatePath( value, lowBlockMaskIdx+1 ) );
                    return false;
                }

                // open file
                blockFile.setFileName ( generatePath(value, lowBlockMaskIdx, ".block" ) );
                if ( !blockFile.open ( QIODevice::WriteOnly ) )
                {
                    throw QString( "Cannot open " +  blockFile.fileName() + ": " + blockFile.errorString() );
                    return false;
                }

                // write out dataBuffer, reset bufferfill, close filelowBlockMaskIdx
                if ( blockFile.write ( ( char* ) blockCache[lowBlockMaskIdx].data(), blockCache[lowBlockMaskIdx].size() * sizeof ( Interval ) ) != blockCache[lowBlockMaskIdx].size() * sizeof ( Interval ) )
                {
                    throw QString( "Cannot write to " +  blockFile.fileName() + ": " + blockFile.errorString() );
                    return false;
                }
                blockFile.close();

                // generate node files
                QStringList pathComponentsPrefix = generatePathComponents( value.begin, lowBlockMaskIdx );
                QStringList newPathComponentsPrefix;
                if ( idx+1 != endIdx )
                {
                    newPathComponentsPrefix = generatePathComponents( flatDataField[idx+1].begin, lowBlockMaskIdx );
                }
                else
                {
                    newPathComponentsPrefix = generatePathComponents( value.begin, lowBlockMaskIdx );
                    newPathComponentsPrefix.append ("xx"); // this way we will also generate the root.node
                }

                for ( int height = lowBlockMaskIdx+1; height < TOTALHEIGHT+1; height++ )
                {
                    quint64 mask = nodeMask( height );

                    // merging lower level node and block cache into current level node cache
                    int nodeCacheIdx = 0;
                    int blockCacheIdx = 0;
                    while ( nodeCacheIdx < nodeCache[height-1].size() || blockCacheIdx < blockCache[height-1].size() )
                    {
                        if ( nodeCacheIdx < nodeCache[height-1].size() &&
                                ( blockCacheIdx == blockCache[height-1].size() ||
                                  nodeCache[height-1][nodeCacheIdx] < blockCache[height-1][blockCacheIdx] ) )
                        {
                            Interval maskedValue;
                            maskedValue.begin = nodeCache[height-1][nodeCacheIdx].begin & mask;
                            maskedValue.end = nodeCache[height-1][nodeCacheIdx].end & mask;
                            if ( nodeCache[height].size() == 0 || maskedValue != nodeCache[height].last() )
                            {
                                nodeCache[height].append( maskedValue );
                            }
                            nodeCacheIdx++;
                        }
                        else
                        {
                            Interval maskedValue;
                            maskedValue.begin = blockCache[height-1][blockCacheIdx].begin & mask;
                            maskedValue.end = blockCache[height-1][blockCacheIdx].end & mask;
                            if ( nodeCache[height].size() == 0 || maskedValue != nodeCache[height].last() )
                            {
                                nodeCache[height].append( maskedValue );
                            }
                            blockCacheIdx++;
                        }
                    }
                    blockCache[height-1].clear();
                    nodeCache[height-1].clear();

                    // get components
                    // chop last elements
                    // check them!
                    pathComponentsPrefix.removeLast();
                    newPathComponentsPrefix.removeLast();

                    if ( pathComponentsPrefix != newPathComponentsPrefix )
                    {
                        // write out block file
                        if ( !blockCache[height].empty() )
                        {
                            blockFile.setFileName( generatePath( value, height, ".block" ) );
                            if ( blockFile.open( QIODevice::WriteOnly ) )
                            {
                                if ( blockFile.write( (const char*) blockCache[height].data(), blockCache[height].size() * sizeof(Interval) )
                                        != blockCache[height].size() * sizeof(Interval) )
                                {
                                    throw QString( "Cannot write file " + blockFile.fileName() + ": " + blockFile.errorString() );
                                    return false;
                                }
                                blockFile.close();
                            }
                        }

                        // generate node file
                        if ( !nodeCache[height].empty() )
                        {
                            // Write out new cache file
                            QFile nodeFile( generatePath( value, height, ".node" ) );
                            if ( nodeFile.open ( QIODevice::WriteOnly ) )
                            {
                                if ( nodeFile.write ( ( const char* ) nodeCache[height].data(), nodeCache[height].size() * sizeof ( Interval ) )
                                        != nodeCache[height].size() * sizeof ( Interval ) )
                                {
                                    throw QString( "Cannot write file " + blockFile.fileName() + ": " + blockFile.errorString() );
                                    return false;
                                }
                                nodeFile.close();
                            }
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                
                for ( int i = lowBlockMaskIdx+1; i < TOTALHEIGHT; i++ )
                {
                    if ( !blockCache[i].empty() )
                    {
                        lowBlockMaskIdx = i+1;
                        break;
                    }
                }
                
/*                for ( int i = lowBlockMaskIdx+1; i < TOTALHEIGHT+2; i++ )
                {
                    if ( !blockCache[i].empty() )
                    {
                        lowBlockMaskIdx = i;
                        break;
                    }
                }*/
            }
        }

        flatFile.unmap ( ( uchar* ) flatDataField );
        flatFile.close();

        state = normal;

        updateGUI();

        emit ( triggerRepaint ( this ) );

    }
    return true;
}

bool IntervalEditor::importFlatFile ( QString flatFileName )
{
    bool result;
    try
    {
        result = importFlatFileMain ( flatFileName );
    }
    catch ( QString )
    {
        return false;
    }
    return result;
}


void IntervalEditor::importFlatFileSlot()
{
    flatFileName =
        QFileDialog::getOpenFileName ( NULL, "Choose Flat IntervalList file", flatFileName, "Flat IntervalList Files (*.ivl);;All Files (*)", 0, NULL );
    if ( flatFileName.isEmpty() )
        return;

    bool result;
    try
    {
        result = importFlatFileMain ( flatFileName );
    }
    catch ( QString messg )
    {
        QMessageBox myMessBox;
        myMessBox.setText ( messg );
        myMessBox.exec();
    }

    if ( result )
    {
        QMessageBox myMessBox;
        myMessBox.setText ( "Successfully imported " + flatFileName + "." );
        myMessBox.exec();
    }
}


bool IntervalEditor::RemoveDirectory ( QDir aDir )
{
    bool has_err = false;
    if ( aDir.exists() )
    {
        QFileInfoList entries = aDir.entryInfoList ( QDir::NoDotAndDotDot |
                                QDir::Dirs | QDir::Files );
        int count = entries.size();
        for ( int idx = 0; idx < count; idx++ )
        {
            QFileInfo entryInfo = entries[idx];
            QString path = entryInfo.absoluteFilePath();
            if ( entryInfo.isDir() )
            {
                has_err = RemoveDirectory ( QDir ( path ) );
            }
            else
            {
                QFile file ( path );
                if ( !file.remove() )
                    has_err = true;
            }
        }
        if ( !aDir.rmdir ( aDir.absolutePath() ) )
            has_err = true;
    }
    return ( has_err );
}

bool IntervalEditor::exportFlatFileMain ( QString flatFileName ) throw ( QString )
{
    // open flat file
    QFile flatFile ( flatFileName );
    if ( !flatFile.open ( QIODevice::WriteOnly ) )
    {
        throw "Cannot open " +  flatFileName + ": " + flatFile.errorString();
        return false;
    }

    QVector<Interval> blockArray;
    recursiveTreeExport( blockArray, treeRootDirName, flatFile );

    // write out the rest of the blockArray
    if ( flatFile.write( (const char*) blockArray.data(), blockArray.size() * sizeof(Interval) ) !=
            blockArray.size() * sizeof(Interval) )
    {
        throw "Cannot write to " + flatFile.fileName() + ": " + flatFile.errorString() + ".";
        return false;
    }

    flatFile.close();
    return true;
}

bool IntervalEditor::exportFlatFile ( QString flatFileName )
{
    bool result;
    try
    {
        result = exportFlatFileMain ( flatFileName );
    }
    catch ( QString )
    {
        return false;
    }
    return result;
}


void IntervalEditor::exportFlatFileSlot()
{
    flatFileName =
        QFileDialog::getSaveFileName ( NULL, "Choose Flat IntervalList file", flatFileName, "Flat IntervalList Files (*.ivl);;All Files (*)", 0, NULL );
    if ( flatFileName.isEmpty() )
        return;
    if ( !flatFileName.endsWith ( ".ivl" ) )
        flatFileName.append ( ".ivl" );


    bool result;
    try
    {
        result = exportFlatFileMain ( flatFileName );
    }
    catch ( QString messg )
    {
        QMessageBox myMessBox;
        myMessBox.setText ( messg );
        myMessBox.exec();
    }

    if ( result )
    {
        QMessageBox myMessBox;
        myMessBox.setText ( "Successfully exported " + flatFileName + "." );
        myMessBox.exec();
    }
}

bool IntervalEditor::recursiveTreeExport( QVector<Interval>& blockArray, QString currentDirName, QFile& flatFile ) throw (QString)
{
    // get directory listing
    QDir currentDir ( currentDirName );

    // subdir List
    QStringList subDirList = currentDir.entryList ( QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name );

    // blockFileList
    currentDir.setNameFilters( QStringList( QString( "*.block" ) ) );
    QStringList blockFileList = currentDir.entryList( QDir::NoDotAndDotDot | QDir::Files, QDir::Name );
    int blockFileIdx = 0;

    // iterate over subDirs
    for ( int i = 0; i < subDirList.size(); i++ )
    {
        // before recursing down into the current subDir, add all blockFiles
        // on this level that come before the current subDir to the blockArray
        QString testblockName = subDirList[i] + ".block";
        while ( blockFileIdx < blockFileList.size() && blockFileList[blockFileIdx] <= testblockName )
        {
            QFile blockFile( currentDirName + "/" + blockFileList[blockFileIdx] );
            if ( !blockFile.open( QIODevice::ReadOnly) )
            {
                throw "Cannot open " +  blockFile.fileName() + ": " + blockFile.errorString() + ".";
                return false;
            }
            qint64 blockFileSize = blockFile.size();

            Interval* blockFileMap = (Interval*) blockFile.map ( 0, blockFileSize );
            if ( blockFileMap == NULL )
            {
                throw "Cannot memory map " +  blockFile.fileName() + ": " + blockFile.errorString() + ".";
                return false;
            }
            Interval lastInterval;
            for ( quint64 j = 0; j < blockFileSize / sizeof ( Interval ); j++ )
            {
                lastInterval = blockFileMap[j];
                blockArray.append ( lastInterval );
            }
            blockFile.unmap ( ( uchar* ) blockFileMap );
            blockFile.close();

            // if there is no subtree below the current blockfile we write out the
            // blockfile
            if ( blockFileList[blockFileIdx] != testblockName )
            {
                qSort( blockArray );
                qint64 endIdx = blockArray.indexOf( lastInterval );
                if ( flatFile.write( (const char*) blockArray.data(), (endIdx+1) * sizeof(Interval) ) !=
                        (endIdx+1) * sizeof(Interval) )
                {
                    throw "Cannot write to " + flatFile.fileName() + ": " + flatFile.errorString() + ".";
                    return false;
                }
                blockArray.remove( 0, endIdx+1 );
            }

            blockFileIdx++;
        }

        if ( !recursiveTreeExport ( blockArray, currentDirName + "/" + subDirList[i], flatFile ) )
        {
            return false;
        }
    }

    // finish off with the rest of the blockFiles
    for ( int i = blockFileIdx; i < blockFileList.size(); i++ )
    {
        QFile blockFile( currentDirName + "/" + blockFileList[i] );
        if ( !blockFile.open( QIODevice::ReadOnly ) )
        {
            throw "Cannot open " +  blockFile.fileName() + ": " + blockFile.errorString() + ".";
            return false;
        }
        qint64 blockFileSize = blockFile.size();

        Interval* blockFileMap = (Interval*) blockFile.map ( 0, blockFileSize );
        if ( blockFileMap == NULL )
        {
            throw "Cannot memory map " +  blockFile.fileName() + ": " + blockFile.errorString() + ".";
            return false;
        }
        Interval lastInterval;
        for ( quint64 j = 0; j < blockFileSize / sizeof ( Interval ); j++ )
        {
            lastInterval = blockFileMap[j];
            blockArray.append ( lastInterval );
        }
        blockFile.unmap ( ( uchar* ) blockFileMap );
        blockFile.close();

        qSort( blockArray );
        qint64 endIdx = blockArray.indexOf( lastInterval );
        if ( flatFile.write( (const char*) blockArray.data(), (endIdx+1) * sizeof(Interval) ) !=
                (endIdx+1) * sizeof(Interval) )
        {
            throw "Cannot write to " + flatFile.fileName() + ": " + flatFile.errorString() + ".";
            return false;
        }
        blockArray.remove( 0, endIdx+1 );
    }
    return true;
}


void IntervalEditor::updateGUI()
{
    Interval dummy;
    switch ( state )
    {
    case ( NoTree ) :
        ui.openIntervalListButton->setText ( "Open IntervalList" );
        ui.openIntervalListButton->setEnabled ( true );
        ui.importButton->setEnabled ( false );
        ui.exportButton->setEnabled ( false );
        ui.addIntervalButton->setEnabled ( false );
        ui.selectIntervalButton->setEnabled ( false );
        ui.delIntervalButton->setEnabled ( false );
        ui.previousButton->setEnabled ( false );
        ui.nextButton->setEnabled ( false );
        ui.lowerManualAdd->setEnabled ( false );
        ui.higherManualAdd->setEnabled ( false );
        ui.manualAddButton->setEnabled ( false );
        ui.manualDeleteButton->setEnabled ( false );

        ui.lowerFineTuneLine->setEnabled ( false );
        ui.lowerMinusButton->setEnabled ( false );
        ui.lowerPlusButton->setEnabled ( false );

        ui.higherFineTuneLine->setEnabled ( false );
        ui.higherMinusButton->setEnabled ( false );
        ui.higherPlusButton->setEnabled ( false );

        ui.trackBox->setEnabled ( false );
        ui.fitScaleBox->setEnabled ( false );


        break;
    case ( EmptyTree ) :
        ui.openIntervalListButton->setText ( "Close IntervalList" );
        ui.openIntervalListButton->setEnabled ( true );
        ui.importButton->setEnabled ( true );
        ui.exportButton->setEnabled ( true );
        ui.addIntervalButton->setEnabled ( true );
        ui.selectIntervalButton->setEnabled ( false );
        ui.delIntervalButton->setEnabled ( false );
        ui.lowerManualAdd->setEnabled ( true );
        ui.higherManualAdd->setEnabled ( true );
        ui.manualAddButton->setEnabled ( true );
        ui.manualDeleteButton->setEnabled ( false );

        ui.lowerFineTuneLine->setEnabled ( true );
        ui.lowerMinusButton->setEnabled ( true );
        ui.lowerPlusButton->setEnabled ( true );

        ui.higherFineTuneLine->setEnabled ( true );
        ui.higherMinusButton->setEnabled ( true );
        ui.higherPlusButton->setEnabled ( true );

        ui.trackBox->setEnabled ( false );
        ui.fitScaleBox->setEnabled ( false );
        ui.previousButton->setEnabled ( false );
        ui.nextButton->setEnabled ( false );

        break;
    case ( normal ) :
        ui.openIntervalListButton->setText ( "Close IntervalList" );
        ui.openIntervalListButton->setEnabled ( true );
        ui.importButton->setEnabled ( true );
        ui.exportButton->setEnabled ( true );
        ui.addIntervalButton->setEnabled ( true );
        ui.selectIntervalButton->setEnabled ( true );
        ui.delIntervalButton->setEnabled ( true );
        ui.lowerManualAdd->setEnabled ( true );
        ui.higherManualAdd->setEnabled ( true );
        ui.manualAddButton->setEnabled ( true );
        ui.manualDeleteButton->setEnabled ( true );

        ui.lowerFineTuneLine->setEnabled ( true );
        ui.lowerMinusButton->setEnabled ( true );
        ui.lowerPlusButton->setEnabled ( true );

        ui.higherFineTuneLine->setEnabled ( true );
        ui.higherMinusButton->setEnabled ( true );
        ui.higherPlusButton->setEnabled ( true );

        ui.trackBox->setEnabled ( true );
        ui.nextButton->setEnabled ( searchNextInterval ( currentInterval, dummy ) );
        ui.previousButton->setEnabled ( searchPrevInterval ( currentInterval, dummy ) );

        ui.lowerFineTuneLine->setText ( QString::number ( currentInterval.begin ) );
        ui.higherFineTuneLine->setText ( QString::number ( currentInterval.end ) );

        break;
    default:
        break;
    }
}

void IntervalEditor::threadDone()
{
    emit notifyListUpdate ( this );
}


bool IntervalEditor::addInterval ( Interval inter )
{
    if ( state == NoTree )
    {
        return false;
    }
    if ( hasInterval( inter ) )
    {
        return false;
    }

    IntervalListLoader_Suspend lock ( ill );

    // calculate path
    QString filePath = genBlockFileName( inter );

    if ( !QFile::exists ( filePath ) )
    {
        QDir treeDir ( treeRootDirName );
        QString dirPath = generatePath( inter, interval2height( inter ) + 1 );
        if ( !treeDir.mkpath ( dirPath ) )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Cannot create directory " + treeRootDirName + "/" + dirPath );
            myMessBox.exec();
            return false;
        }
        QFile blockFile ( filePath );
        if ( !blockFile.open ( QIODevice::WriteOnly ) )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Cannot open " + blockFile.fileName() + " " + blockFile.errorString() );
            myMessBox.exec();
            return false;
        }
        if ( blockFile.write ( ( char* ) &inter, sizeof ( Interval ) ) != sizeof ( Interval ) )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Cannot write to " + blockFile.fileName() + " " + blockFile.errorString() );
            myMessBox.exec();
            return false;
        }
        blockFile.close();
    }
    else
    {
        QFile blockFile ( filePath );
        if ( !blockFile.open ( QIODevice::ReadWrite ) )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Cannot open " + blockFile.fileName() + " " + blockFile.errorString() );
            myMessBox.exec();
            return false;
        }

        QByteArray buffer ( blockFile.size() + sizeof ( Interval ), '0' );
        Interval* bufPtr = ( Interval* ) buffer.data();
        if ( blockFile.read ( ( char* ) bufPtr, blockFile.size() ) != blockFile.size() )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Cannot read from " + blockFile.fileName() + " " + blockFile.errorString() );
            myMessBox.exec();
            return false;
        }

        // the for loop ends once i wraps around from 0 to max quint64
        quint64 limit = blockFile.size() / sizeof ( Interval ) - 1;
        for ( quint64 i = limit; i <= limit ; i-- )
        {
            if ( bufPtr[i] == inter )
            {
                return false;
            }
            if ( bufPtr[i] < inter )
            {
                bufPtr[i+1] = inter;
                break;
            }
            bufPtr[i+1] = bufPtr[i];
            if ( i == 0 )
            {
                bufPtr[0] = inter;
                break;
            }
        }

        qint64 oldBlockFileSize = blockFile.size();
        blockFile.seek ( 0 );
        if ( blockFile.write ( ( char* ) bufPtr, blockFile.size() + sizeof ( Interval ) ) != oldBlockFileSize + sizeof ( Interval ) )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Cannot write to " + blockFile.fileName() + " " + blockFile.errorString() );
            myMessBox.exec();
            return false;
        }
        blockFile.close();
    }

    // update the node files
    QStringList pathCompList = generatePathComponents( inter.begin, interval2height(inter) );
    partialNodeRebuild( pathCompList );


    if ( state != normal )
    {
        currentInterval = inter;
        state = normal;
    }
    updateGUI();
    trackUpdate();
    emit triggerRepaint ( this );
    return true;
}

bool IntervalEditor::hasInterval ( Interval inter )
{
    // calculate path

    QString path = genBlockFileName( inter );

    // check if file exists
    QFileInfo fileInfo ( path );
    if ( fileInfo.exists() )
    {
        // if exists:
        // - open
        QFile blockFile ( path );
        if ( blockFile.open ( QIODevice::ReadWrite ) )
        {
            // map data
            Interval* blockData = ( Interval* ) blockFile.map ( 0, blockFile.size() );
            bool found = false;
            if ( blockData == NULL )
            {
                return false;
            }

            for ( quint64 i = 0; i < blockFile.size() / sizeof ( Interval ); i++ )
            {
                if ( blockData[i] == inter )
                {
                    found = true;
                    break;
                }
            }
            blockFile.unmap ( ( uchar* ) blockData );
            blockFile.close();

            return found;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}


void IntervalEditor::partialNodeRebuild ( QStringList pathToBlockFile )
{
    pathToBlockFile.removeLast();
    // go down in path to blockfile
    for ( int i = pathToBlockFile.size(); i >= 0; i-- )
    {
        quint64 mask = nodeMask( TOTALHEIGHT - (i+1) );

        QString path = treeRootDirName;
        for ( int j = 0; j < i; j++ )
        {
            path += "/" + pathToBlockFile[j];
        }

        // generate node file

        // open corresponding block/node files in subdir and condense a new node
        QDir subDir( path );
        subDir.setNameFilters( QStringList( QString( "*.block" ) ) );
        QStringList blockFileList = subDir.entryList ( QDir::NoDotAndDotDot | QDir::Files, QDir::Name );
        subDir.setNameFilters( QStringList( QString( "*.node" ) ) );
        QStringList nodeFileList = subDir.entryList ( QDir::NoDotAndDotDot | QDir::Files, QDir::Name );


        QFile blockFile;
        QFile nodeFile;
        Interval* blockFileMap;
        Interval* nodeFileMap;
        bool blockFileOpen = blockFile.isOpen();
        bool nodeFileOpen = nodeFile.isOpen();
        qint64 blockFileIdx = 0;
        qint64 blockFileMaxIdx = -1;
        qint64 nodeFileIdx = 0;
        qint64 nodeFileMaxIdx = -1;
        int blockListIdx = 0;
        int nodeListIdx = 0;
        bool blockListEmpty = blockFileList.isEmpty();
        bool nodeListEmpty = nodeFileList.isEmpty();
        Interval* outBuffer = NULL;
        long outBufferLength = 1;
        long outBufferIdx = 0;


        Interval blockCacheValue;
        bool blockCacheValueValid = false;
        Interval nodeCacheValue;
        bool nodeCacheValueValid = false;

        // while some fileList contains elements
        while ( !blockListEmpty || !nodeListEmpty )
        {
            Interval value;

            // blockFile list: empty => skip | eof => close + next file | if empty => set blockListEmpty skip | closed => open | read
            if ( !blockCacheValueValid && !blockListEmpty )
            {
                // if eof
                if ( blockFileIdx == blockFileMaxIdx )
                {
                    blockFile.close();
                    blockFileOpen = false;
                    blockListIdx++;
                    if ( blockListIdx == blockFileList.size() )
                    {
                        blockListEmpty = true;
                        continue;
                    }
                }
                if ( !blockListEmpty )
                {
                    if ( !blockFileOpen )
                    {
                        // open blockFile
                        QString blockFileName = path + "/" + blockFileList[blockListIdx];
                        blockFile.setFileName( blockFileName );
                        if ( !blockFile.open( QIODevice::ReadOnly ) )
                        {
                            qDebug() << "Cannot open" << blockFile.fileName();
                            return;
                        }
                        blockFileMap = (Interval*) blockFile.map( 0, blockFile.size() );
                        if ( blockFileMap == NULL )
                        {
                            qDebug() << "Cannot map" << blockFile.fileName();
                            return;
                        }
                        blockFileMaxIdx = blockFile.size() / sizeof( Interval );
                        blockFileOpen = true;
                        blockFileIdx = 0;
                    }

                    // read value
                    blockCacheValue = blockFileMap[blockFileIdx++];
                    blockCacheValueValid = true;
                }
            }


            // nodeFile list: empty => skip | eof => close + next file | if empty => set blockListEmpty skip | closed => open | read
            if ( !nodeCacheValueValid && !nodeListEmpty )
            {
                // if eof
                if ( nodeFileIdx == nodeFileMaxIdx )
                {
                    nodeFile.close();
                    nodeFileOpen = false;
                    nodeListIdx++;
                    if ( nodeListIdx == nodeFileList.size() )
                    {
                        nodeListEmpty = true;
                        continue;
                    }
                }
                if ( !nodeListEmpty )
                {
                    if ( !nodeFileOpen )
                    {
                        // open nodeFile
                        QString nodeFileName = path + "/" + nodeFileList[nodeListIdx];
                        nodeFile.setFileName( nodeFileName );
                        if ( !nodeFile.open( QIODevice::ReadOnly ) )
                        {
                            qDebug() << "Cannot open" << nodeFile.fileName();
                            return;
                        }
                        nodeFileMap = (Interval*) nodeFile.map( 0, nodeFile.size() );
                        if ( nodeFileMap == NULL )
                        {
                            qDebug() << "Cannot map" << nodeFile.fileName();
                            return;
                        }
                        nodeFileMaxIdx = nodeFile.size() / sizeof( Interval );
                        nodeFileOpen = true;
                        nodeFileIdx = 0;
                    }

                    // read value
                    nodeCacheValue = nodeFileMap[nodeFileIdx++];
                    nodeCacheValueValid = true;
                }
            }


            // choose smaller value
            if ( ( blockCacheValueValid && !nodeCacheValueValid ) ||
                    ( ( blockCacheValueValid && nodeCacheValueValid ) && blockCacheValue < nodeCacheValue ) )
            {
                value = blockCacheValue;
                blockCacheValueValid = false;
            }
            else if ( ( !blockCacheValueValid && nodeCacheValueValid ) ||
                      ( ( blockCacheValueValid && nodeCacheValueValid ) && blockCacheValue > nodeCacheValue ) )
            {
                value = nodeCacheValue;
                nodeCacheValueValid = false;
            }

            // if array full => extend
            if ( outBufferIdx + 1 == outBufferLength )
            {
                outBufferLength *= 2;
                outBuffer = (Interval*) realloc( outBuffer, outBufferLength * sizeof( Interval ) );
            }

            // mask value
            value.begin = value.begin & mask;
            value.end = value.end & mask;

            // add to array
            if ( outBufferIdx == 0 || outBuffer[outBufferIdx-1] != value )
            {
                outBuffer[outBufferIdx++] = value;
            }
        }
        // end while

        //if ( nodeCache.isEmpty() )
        if ( outBufferIdx == 0 )
        {
            // delete old node file
            QFile::remove( path + ".node" );

            // check if directory is empty
            if ( subDir.count() == 0 ) // if yes, delete it
            {
                QDir rmDir( treeRootDirName );
                if ( !rmDir.rmdir( subDir.path() ) )
                {
                    qDebug() << "Error cannot delete empty directory" << subDir.path();
                    return;
                }
            }
            continue;
        }

        // Write out new node file
        QFile outNodeFile ( path + ".node" );
        if ( outNodeFile.open ( QIODevice::WriteOnly | QIODevice::Truncate ) )
        {
            outNodeFile.write ( ( char* ) outBuffer, outBufferIdx * sizeof ( Interval ) );
            outNodeFile.close();
        }
        free( outBuffer );
        // end loop
    }
}

bool IntervalEditor::findInterval ( Interval& inter, qint64 xBegin, qint64 xEnd )
{
    // recursive descent within xBegin and xEnd searching for an interval
    return findInterval_sub ( inter, xBegin, xEnd, treeRootDirName, 0, TOTALHEIGHT );
}

bool IntervalEditor::findInterval_sub ( Interval& inter, qint64 xBegin, qint64 xEnd, QString path, quint64 pathValue, int height )
{
    bool result = false;

    QDir currentDir( path );
    QStringList subDirList = currentDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
    QStringList blockFileList = currentDir.entryList( QStringList( QString( "*.block" ) ) , QDir::Files | QDir::NoDotAndDotDot, QDir::Name );
    int currentblockFileIdx = 0;

    for ( int i = 0; i < subDirList.size(); i++ )
    {
        QString testString = subDirList[i] + ".block";
        while ( currentblockFileIdx < blockFileList.size() && blockFileList[currentblockFileIdx] < testString )
        {
            if ( dirName2sliceMin( blockFileList[currentblockFileIdx].left( blockFileList[currentblockFileIdx].size() - 6 ), height-1, pathValue ) > xEnd )
            {
                return false;
            }
            if ( xBegin <= dirName2sliceMax( blockFileList[currentblockFileIdx].left( blockFileList[currentblockFileIdx].size() - 6 ), height-1, pathValue ) )
            {
                // search inside blockFile
                QFile blockFile ( path + "/" + blockFileList[currentblockFileIdx] );
                if ( blockFile.open ( QIODevice::ReadOnly ) )
                {
                    Interval* blockData = ( Interval* ) blockFile.map ( 0, blockFile.size() );
                    if ( blockData != NULL )
                    {
                        for ( quint64 j = 0; j < blockFile.size() / sizeof ( Interval ); j++ )
                        {
                            if ( blockData[j].begin > xEnd )
                            {
                                break;
                            }
                            if ( xBegin <= blockData[j].end )
                            {
                                inter = blockData[j];
                                return true;
                            }
                        }
                        blockFile.unmap ( ( uchar* ) blockData );
                    }
                    blockFile.close();
                }
            }
            currentblockFileIdx++;
        }

        // recurse
        quint64 sliceMin = dirName2sliceMin( subDirList[i], height-1, pathValue );
        if ( sliceMin > xEnd )
        {
            return false;
        }
        quint64 sliceMax = dirName2sliceMax( subDirList[i], height-1, pathValue );
        if ( xBegin <= sliceMax )
        {
            if ( findInterval_sub ( inter, xBegin, xEnd, path + "/" + subDirList[i],
                                    dirName2sliceMin( subDirList[i], height-1, pathValue ), height - 1 ) )
            {
                return true;
            }
        }
    }
    for ( int i = currentblockFileIdx; i < blockFileList.size(); i++ )
    {
        quint64 sliceMin = dirName2sliceMin( blockFileList[i].left( blockFileList[i].size() - 6 ), height-1, pathValue );
        if ( sliceMin > xEnd )
        {
            return false;
        }
        quint64 sliceMax = dirName2sliceMax( blockFileList[i].left( blockFileList[i].size() - 6 ), height-1, pathValue );
        if ( xBegin <= sliceMax )
        {
            // search inside blockFile
            QFile blockFile ( path + "/" + blockFileList[i] );
            if ( blockFile.open ( QIODevice::ReadOnly ) )
            {
                Interval* blockData = ( Interval* ) blockFile.map ( 0, blockFile.size() );
                if ( blockData != NULL )
                {
                    for ( quint64 j = 0; j < blockFile.size() / sizeof ( Interval ); j++ )
                    {
                        if ( blockData[j].begin > xEnd )
                        {
                            return false;
                        }
                        if ( xBegin <= blockData[j].end )
                        {
                            inter = blockData[j];
                            return true;
                        }
                    }
                    blockFile.unmap ( ( uchar* ) blockData );
                }
                blockFile.close();
            }
        }
    }

    return result;
}

bool IntervalEditor::delInterval ( Interval inter )
{
    if ( state != normal )
    {
        return false;
    }
    IntervalListLoader_Suspend lock ( ill );
    // generate path
    QString path = generatePath( inter, interval2height( inter ), ".block" );

    // open file
    QFile blockFile ( path );
    if ( blockFile.open ( QIODevice::ReadWrite ) )
    {
        Interval* blockData = ( Interval* ) blockFile.map ( 0, blockFile.size() );
        bool found = false;
        if ( blockData != NULL )
        {
            // read file
            for ( qint64 i = 0; i < blockFile.size() / sizeof ( Interval ); i++ )
            {
                if ( blockData[i] == inter )
                {
                    found = true;
                }
                if ( found && i < blockFile.size() / sizeof ( Interval ) - 1 )
                {
                    blockData[i] = blockData[i+1];
                }
            }
            blockFile.unmap ( ( uchar* ) blockData );
            if ( found )
            {
                blockFile.resize ( blockFile.size() - sizeof ( Interval ) );
            }
        }
        else
        {
            return false;
        }

        blockFile.close();

        if ( !found )
        {
            return false;
        }

        if ( blockFile.size() == 0 )
            blockFile.remove();

        // update node files on the way back
        partialNodeRebuild ( generatePathComponents( inter.begin, interval2height( inter ) ) );
    }
    else
    {
        return false;
    }
    if ( isEmpty() )
    {
        state = EmptyTree;
    }
    Interval newInter;
    if ( inter == currentInterval )
    {
        if ( searchNextInterval ( currentInterval, newInter ) )
        {
            currentInterval = newInter;
        }
        else if ( searchPrevInterval ( currentInterval, newInter ) )
        {
            currentInterval = newInter;
        }
    }
    updateGUI();
    trackUpdate();
    emit triggerRepaint ( this );
    return true;
}

bool IntervalEditor::isEmpty()
{
    QDir treeDir ( treeRootDirName );
    QStringList subDirList = treeDir.entryList ( QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDir::Name );
    return subDirList.isEmpty();
}

bool IntervalEditor::hasNextInterval ( Interval inter )
{
    //QVector<Interval> vec;
    Interval nextInter;
    //return searchNextInterval_sub ( inter, nextInter, true, treeRootDirName, vec, TOTALHEIGHT );
    return searchNextInterval_sub ( inter, nextInter, true, treeRootDirName, TOTALHEIGHT );
}

bool IntervalEditor::hasPrevInterval ( Interval inter )
{
    QVector<Interval> vec;
    Interval prevInter;
    return searchPrevInterval ( inter, prevInter );
}

Interval IntervalEditor::nextInterval ( Interval inter ) throw (bool)
{
    //QVector<Interval> vec;
    Interval nextInter;
    //if ( searchNextInterval_sub ( inter, nextInter, true, treeRootDirName, vec, TOTALHEIGHT ) )
    if ( searchNextInterval_sub ( inter, nextInter, true, treeRootDirName, TOTALHEIGHT ) )  
    {
        return nextInter;
    }
    else
    {
        throw false;
    }
}

Interval IntervalEditor::prevInterval ( Interval inter ) throw (bool )
{
    QVector<Interval> vec;
    Interval prevInter;
    if ( searchPrevInterval ( inter, prevInter ) )
    {
        return prevInter;
    }
    else
    {
        throw false;
    }
}


bool IntervalEditor::searchNextInterval ( Interval inter, Interval& nextInter )
{
    // 2 phases firstDescent true/false
    // in first descent we try to find the given interval
    // in the second phase we try to search the next interval
    //QVector<Interval> vec;
    //bool result =  searchNextInterval_sub ( inter, nextInter, true, treeRootDirName, vec, TOTALHEIGHT );
    bool result =  searchNextInterval_sub ( inter, nextInter, true, treeRootDirName, TOTALHEIGHT );
    return result;
}


// turn the vector thing upside down, first search deep then hand the results up, and *then* decide whether
// to use the interval from below or the top level block file
bool IntervalEditor::searchNextInterval_sub ( Interval inter, Interval& nextInter, bool firstDescent, QString path, int height )
{
  bool localDescent = firstDescent;
  QDir currentDir( path );
  QStringList subdirList = currentDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
  QStringList blockFileList = currentDir.entryList( QStringList(QString( "*.block" ) ),
                                                    QDir::Files | QDir::NoDotAndDotDot, QDir::Name );
  QString startDirName = event2dirName( inter.begin, height );
  
  int blockFileIdx = 0;
  // Iterate over all subdirectories
  for ( int i = 0; i < subdirList.size(); i++ ) {
    // firstDescent: Consume all blockfiles before next subdirectory
    // !firstDescent: Just consume next blockfiles
    while ( blockFileIdx < blockFileList.size() && blockFileList[blockFileIdx] < subdirList[i] + ".block" ) {
      if ( !localDescent || blockFileList[blockFileIdx] >= startDirName + ".block" ) {
        QVector<Interval> localVec;
        QFile blockFile( path + "/" + blockFileList[blockFileIdx] );
        if ( blockFile.open( QIODevice::ReadOnly ) ) {
          Interval* blockFileMap = ( Interval* ) blockFile.map ( 0, blockFile.size() );
          if ( blockFileMap != NULL ) {
            qint64 endIdx = blockFile.size() / sizeof( Interval );
            for ( qint64 j = 0; j < endIdx; j++ ) {
              if( blockFileMap[j] > inter ) {
                nextInter = blockFileMap[j];
                return true;
              }
            }
          }
          blockFile.close();
        }
      }
      blockFileIdx++;
    }
    
    // Read in the blockfile to the current subdir
    QVector<Interval> localVec;
    if ( ( blockFileIdx < blockFileList.size() ) &&
      ( blockFileList[blockFileIdx] == subdirList[i] + ".block" ) &&
      ( !localDescent || blockFileList[blockFileIdx] >= startDirName + ".block" ) ) 
    {
      QFile blockFile( path + "/" + blockFileList[blockFileIdx] );
      if ( blockFile.open( QIODevice::ReadOnly ) ) {
        Interval* blockFileMap = ( Interval* ) blockFile.map ( 0, blockFile.size() );
        if ( blockFileMap != NULL ) {
          qint64 endIdx = blockFile.size() / sizeof( Interval );
          for ( qint64 j = 0; j < endIdx; j++ ) {
            localVec.append( blockFileMap[j] );
          }
        }
        blockFile.close();
      }
    }
    
    // Descend into subdir
    if ( !localDescent || subdirList[i] >= startDirName ) {
      if ( subdirList[i] > startDirName ) {
        // we already passed the search start point => second descent
        localDescent = false;
      }
      
      if ( searchNextInterval_sub ( inter, nextInter, localDescent, path + "/" + subdirList[i], height-1 ) ) {
        // combine result from the depth of the tree with the current level block file result and
        // hand up the result
        localVec.append(nextInter);
        qSort( localVec );
        for ( qint64 j = 0; j < localVec.size(); j++ ) {
          if ( localVec[j] > inter ) {
            nextInter = localVec[j];
            return true;
          }
        }
      }
      else
      {
        // switch to second descent
        localDescent = false;
      }
    }
  }
  
  for ( int i = blockFileIdx; i < blockFileList.size(); i++ )
  {
    if ( !localDescent || blockFileList[i] >= startDirName + ".block" )
    {
      QVector<Interval> localVec;
      QFile blockFile( path + "/" + blockFileList[i] );
      if ( blockFile.open( QIODevice::ReadOnly ) )
      {
        Interval* blockFileMap = ( Interval* ) blockFile.map ( 0, blockFile.size() );
        if ( blockFileMap != NULL )
        {
          qint64 endIdx = blockFile.size() / sizeof( Interval );
          for ( qint64 j = 0; j < endIdx; j++ )
          {
            localVec.append( blockFileMap[j] );
          }
        }
        blockFile.close();
      }
      qSort( localVec );
      for ( qint64 j = 0; j < localVec.size(); j++ )
      {
        if ( localVec[j] > inter )
        {
          nextInter = localVec[j];
          return true;
        }
      }
    }
  }
  
  // if not found, return false
  return false;
}

bool IntervalEditor::searchPrevInterval ( Interval inter, Interval& prevInter )
{
    // 2 phases firstDescent true/false
    // in first descent we try to find the given interval
    // in the second phase we try to search the next interval
    QVector<Interval> vec = searchPrevInterval_sub ( inter, true, treeRootDirName, TOTALHEIGHT );
    if ( vec.isEmpty() )
    {
        return false;
    }
    else
    {
        qSort( vec );
        prevInter = vec.last();
        return true;
    }
}


QVector<Interval> IntervalEditor::searchPrevInterval_sub ( Interval inter, bool firstDescent, QString path, int height )
{
    QVector <Interval> result;
    bool localDescent = firstDescent;
    QDir currentDir( path );
    QStringList subdirList = currentDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
    QStringList blockFileList = currentDir.entryList( QStringList(QString( "*.block" ) ),
                                QDir::Files | QDir::NoDotAndDotDot, QDir::Name );
    QString endDirName = event2dirName( inter.begin, height );

    int blockFileIdx = blockFileList.size() - 1;
    for ( int i = subdirList.size() - 1; i >= 0; i-- ) {
        while ( blockFileIdx >= 0 && blockFileList[blockFileIdx] > subdirList[i] + ".block" ) {
            if ( !localDescent || blockFileList[blockFileIdx] <= endDirName + ".block" ) {
                QVector<Interval> localVec;
                QFile blockFile( path + "/" + blockFileList[blockFileIdx] );
                if ( blockFile.open( QIODevice::ReadOnly ) ) {
                    Interval* blockFileMap = ( Interval* ) blockFile.map ( 0, blockFile.size() );
                    if ( blockFileMap != NULL ) {
                        qint64 endIdx = blockFile.size() / sizeof( Interval );
                        for ( qint64 j = 0; j < endIdx; j++ ) {
                            localVec.append( blockFileMap[j] );
                        }
                    }
                    blockFile.close();
                }
                for ( qint64 j = localVec.size() - 1; j >= 0; j-- ) {
                    if ( localVec[j] < inter ) {
                        result.append( localVec[j] );
                        return result;
                    }
                }
            }
            blockFileIdx--;
        }

        if ( !localDescent || subdirList[i] <= endDirName ) {
            // if we can't descend down to the subdir we switch to second descent
            if ( subdirList[i] < endDirName ) {
                localDescent = false;
            }

            result = searchPrevInterval_sub ( inter, localDescent, path + "/" + subdirList[i], height-1 );

            if ( !result.isEmpty() ) {
                break;
            } else {
                // switch to second descent
                localDescent = false;
            }
        }
    }

    for ( int i = blockFileIdx; i >= 0; i-- ) {
        if ( !localDescent || blockFileList[i] <= endDirName + ".block" ) {
            QVector<Interval> localVec;
            QFile blockFile( path + "/" + blockFileList[i] );
            if ( blockFile.open( QIODevice::ReadOnly ) ) {
                Interval* blockFileMap = ( Interval* ) blockFile.map ( 0, blockFile.size() );
                if ( blockFileMap != NULL ) {
                    qint64 endIdx = blockFile.size() / sizeof( Interval );
                    for ( qint64 j = 0; j < endIdx; j++ ) {
                        localVec.append( blockFileMap[j] );
                    }
                }
                blockFile.close();
            }
            for ( qint64 j = localVec.size() - 1; j >= 0; j-- ) {
                if ( localVec[j] < inter ) {
                    result.append( localVec[j] );
                    return result;
                }
            }
        }
    }

    // if not found, return false
    return result;
}

QStringList IntervalEditor::orderBlockBeforeDir ( QStringList in )
{
    // delete node entries
    QStringList::iterator iter = in.begin();
    while ( iter != in.end() )
    {
        if ( iter->endsWith ( ".node" ) )
        {
            iter = in.erase ( iter );
        }
        else
        {
            iter++;
        }
    }
    // sort
    qSort ( in );

    // switch block file enties before corresponding directories
    if ( in.size() >= 2 )
    {
        for ( int i = 1; i < in.size(); i++ )
        {
            if ( in[i].endsWith ( ".block" ) )
            {
                QString tmp = in[i];
                tmp.chop ( 6 );
                if ( in[i-1] == tmp )
                {
                    tmp = in[i];
                    in[i] = in[i-1];
                    in[i-1] = tmp;
                }
            }
        }
    }
    return in;
}

QStringList IntervalEditor::orderDirBeforeBlock ( QStringList in )
{
    // delete node entries
    QStringList::iterator iter = in.begin();
    while ( iter != in.end() )
    {
        if ( iter->endsWith ( ".node" ) )
        {
            iter = in.erase ( iter );
        }
        else
        {
            iter++;
        }
    }
    // sort
    qSort ( in );

    // switch directories before block file enties
    if ( in.size() >= 2 )
    {
        for ( int i = 0; i < in.size() - 1; i++ )
        {
            if ( in[i].endsWith ( ".block" ) )
            {
                QString tmp = in[i];
                tmp.chop ( 6 );
                if ( in[i+1] == tmp )
                {
                    tmp = in[i];
                    in[i] = in[i+1];
                    in[i+1] = tmp;
                }
            }
        }
    }
    return in;
}


void IntervalEditor::nextIntervalSlot()
{
    Interval nextInter;
    if ( searchNextInterval ( currentInterval, nextInter ) )
    {
        currentInterval = nextInter;
        trackUpdate();
        updateGUI();
        emit triggerRepaint ( this );
    }
}

void IntervalEditor::prevIntervalSlot()
{
    Interval prevInter;
    if ( searchPrevInterval ( currentInterval, prevInter ) )
    {
        currentInterval = prevInter;
        trackUpdate();
        updateGUI();
        emit triggerRepaint ( this );
    }
}

void IntervalEditor::trackUpdate()
{
    // gets called from nextInterval, when ticking on or off a checkbox
    if ( ui.trackBox->checkState() != Qt::Checked )
        return;

    if ( ui.fitScaleBox->checkState() == Qt::Checked )
    {
        long double interWidth = currentInterval.end - currentInterval.begin;
        emit fitOn ( (long double) currentInterval.begin - (interWidth/2), (long double) currentInterval.end + (interWidth/2) );
    }
    else
    {
        long double center = ( ( long double ) currentInterval.begin + ( long double ) currentInterval.end ) / 2;
        emit centerOn ( center, DoubleNaN, DoubleNaN, DoubleNaN );
    }
    emit triggerRepaint ( this );
}

void IntervalEditor::toggleTrackBox ( int state )
{
    switch ( state )
    {
    case Qt::Unchecked:
        ui.fitScaleBox->setEnabled ( false );
        break;
    case Qt::Checked:
        ui.fitScaleBox->setEnabled ( true );
        trackUpdate();
        break;
    default:
        qDebug() << "void IntervalEditor::toggleTrackBox ( int state ): shouldn't reach default case.";
    }
}

void IntervalEditor::toggleFitBox ( int state )
{
    switch ( state )
    {
    case Qt::Unchecked:
        break;
    case Qt::Checked:
        trackUpdate();
        break;
    default:
        qDebug() << "void IntervalEditor::toggleFitBox ( int state ): shouldn't reach default case.";
    }
}


void IntervalEditor::validateManualLow ()
{
    QString text = ui.lowerManualAdd->text();
    // an empty string is ok
    if ( text.isEmpty() )
        return;

    // Try to convert it to quint64
    bool success;
    quint64 value = text.toULongLong ( &success );
    if ( success )
    {
        manualLow = text;
    }
    else
    {
        ui.lowerManualAdd->setText ( manualLow );
    }
}

void IntervalEditor::validateManualHigh ()
{
    QString text = ui.higherManualAdd->text();
    // an empty string is ok
    if ( text.isEmpty() )
        return;

    // Try to convert it to quint64
    bool success;
    quint64 value = text.toULongLong ( &success );
    if ( success )
    {
        manualHigh = text;
    }
    else
    {
        ui.higherManualAdd->setText ( manualHigh );
    }
}


void IntervalEditor::manualAddInterval()
{
    // check for empty fields
    if ( ui.lowerManualAdd->text().isEmpty() )
        ui.lowerManualAdd->setText ( "0" );

    if ( ui.higherManualAdd->text().isEmpty() )
        ui.higherManualAdd->setText ( "0" );

    // read the values from the input fields
    quint64 val1 = ui.lowerManualAdd->text().toULongLong();
    quint64 val2 = ui.higherManualAdd->text().toULongLong();
    Interval inter;
    inter.begin = qMin ( val1, val2 );
    inter.end = qMax ( val1, val2 );
    // add interval
    addInterval ( inter );
    currentInterval = inter;
    updateGUI();
    emit triggerRepaint ( this );
}

void IntervalEditor::manualDelInterval()
{
    delInterval ( currentInterval );
}

void IntervalEditor::lowerMinusHandler()
{
    // get value
    quint64 newBegin = ui.lowerFineTuneLine->text().toULongLong();
    // check that != 0
    if ( newBegin == 0 )
        return;
    // substract one
    newBegin--;
    Interval newCurrent = currentInterval;
    newCurrent.begin = newBegin;
    // del interval
    delInterval ( currentInterval );
    // wait until the event loop is ready again
    ill->eventLoopAlive();
    // add interval
    addInterval ( newCurrent );
    currentInterval = newCurrent;
    updateGUI();
    trackUpdate();
    emit triggerRepaint ( this );
}

void IntervalEditor::lowerPlusHandler()
{
    // get value
    quint64 newBegin = ui.lowerFineTuneLine->text().toULongLong();
    // check that != 0
    if ( newBegin == currentInterval.end )
        return;
    // substract one
    newBegin++;
    Interval newCurrent = currentInterval;
    newCurrent.begin = newBegin;
    // del interval
    delInterval ( currentInterval );
    // wait until the event loop is ready again
    ill->eventLoopAlive();
    // add interval
    addInterval ( newCurrent );
    currentInterval = newCurrent;
    updateGUI();
    trackUpdate();
    emit triggerRepaint ( this );
}

void IntervalEditor::upperMinusHandler()
{
    // get value
    quint64 newEnd = ui.higherFineTuneLine->text().toULongLong();
    // check that != 0
    if ( newEnd == currentInterval.begin )
        return;
    // substract one
    newEnd--;
    Interval newCurrent = currentInterval;
    newCurrent.end = newEnd;
    // del interval
    delInterval ( currentInterval );
    // wait until the event loop is ready again
    ill->eventLoopAlive();
    // add interval
    addInterval ( newCurrent );
    currentInterval = newCurrent;
    updateGUI();
    trackUpdate();
    emit triggerRepaint ( this );
}

void IntervalEditor::upperPlusHandler()
{
    // get value
    quint64 newEnd = ui.higherFineTuneLine->text().toULongLong();
    // check that != 0
    if ( newEnd == ( ( quint64 ) 0 - 1 ) )
        return;
    // substract one
    newEnd++;
    Interval newCurrent = currentInterval;
    newCurrent.end = newEnd;
    // del interval
    delInterval ( currentInterval );
    // wait until the event loop is ready again
    ill->eventLoopAlive();
    // add interval
    addInterval ( newCurrent );
    currentInterval = newCurrent;
    updateGUI();
    trackUpdate();
    emit triggerRepaint ( this );
}

void IntervalEditor::validateFineTuneLow()
{
    QString text = ui.lowerFineTuneLine->text();
    // an empty string is ok
    if ( text.isEmpty() )
        return;

    // Try to convert it to quint64
    bool success;
    quint64 value = text.toULongLong ( &success );
    if ( !success )
    {
        ui.lowerFineTuneLine->setText ( fineTuneLow );
    }
}


void IntervalEditor::validateFineTuneHigh()
{
    QString text = ui.higherFineTuneLine->text();
    // an empty string is ok
    if ( text.isEmpty() )
        return;

    // Try to convert it to quint64
    bool success;
    quint64 value = text.toULongLong ( &success );
    if ( !success )
    {
        ui.lowerFineTuneLine->setText ( fineTuneHigh );
    }
}


void IntervalEditor::handleFineTuneLow ( )
{
    QString text = ui.lowerFineTuneLine->text();
    // an empty string is ok
    if ( text.isEmpty() )
        return;

    // Try to convert it to quint64
    bool success;
    quint64 value = text.toULongLong ( &success );
    if ( success )
    {
        if ( value <= currentInterval.end )
        {
            fineTuneLow = text;
            Interval newCurrent = currentInterval;
            newCurrent.begin = value;
            delInterval ( currentInterval );
            ill->eventLoopAlive();
            addInterval ( newCurrent );
            currentInterval = newCurrent;
            updateGUI();
            trackUpdate();
            emit triggerRepaint ( this );
        }
    }
    else
    {
        ui.lowerFineTuneLine->setText ( fineTuneLow );
    }
}

void IntervalEditor::handleFineTuneHigh ( )
{
    QString text = ui.higherFineTuneLine->text();
    // an empty string is ok
    if ( text.isEmpty() )
        return;

    // Try to convert it to quint64
    bool success;
    quint64 value = text.toULongLong ( &success );
    if ( success )
    {
        if ( currentInterval.begin <= value )
        {
            fineTuneHigh = text;
            Interval newCurrent = currentInterval;
            newCurrent.end = value;
            delInterval ( currentInterval );
            ill->eventLoopAlive();
            addInterval ( newCurrent );
            currentInterval = newCurrent;
            updateGUI();
            trackUpdate();
            emit triggerRepaint ( this );
        }
    }
    else
    {
        ui.lowerFineTuneLine->setText ( fineTuneHigh );
    }
}

void IntervalEditor::showGUI()
{
    gui->show();
}

void IntervalEditor::hideGUI()
{
    gui->hide();
}

bool IntervalEditor::hasGUI()
{
    return true;
}

bool IntervalEditor::GUIvisible()
{
    return gui->isVisible();
}

void IntervalEditor::relayGUIupdate()
{
    emit GUIshowhide();
}

Interval IntervalEditor::getCurrentInterval()
{
    return currentInterval;
}

bool IntervalEditor::setCurrentInterval ( Interval inter )
{
    if ( !hasInterval ( inter ) )
    {
        return false;
    }
    else
    {
        currentInterval = inter;
        return true;
    }
}

qint64 IntervalEditor::getXmin()
{
    Interval inter;
    if ( findFirstInterval ( inter ) )
    {
        return inter.begin;
    }
    else
    {
        return FTSPlot::GL_Layer::getXmin();
    }
}

qint64 IntervalEditor::getXmax()
{
    Interval inter;
    if ( findLastInterval ( inter ) )
    {
        return inter.end;
    }
    else
    {
        return FTSPlot::GL_Layer::getXmax();
    }
}

QString IntervalEditor::generatePath(Interval inter, int height, QString suffix)
{
    QStringList components = generatePathComponents( inter.begin );
    QString result = treeRootDirName;
    for ( int i = 0; i < TOTALHEIGHT - height; i++ )
    {
        result += "/" + components[i];
    }
    result += suffix;
    return result;
}

QString IntervalEditor::genBlockFileName(Interval inter)
{
    return generatePath( inter, interval2height( inter ), ".block" );
}


int IntervalEditor::interval2height( Interval inter )
{
    int height = 0;
    quint64 diffMask = inter.begin ^ inter.end;
    diffMask >>= BLOCKFACTOR;
    while ( diffMask != 0 )
    {
        diffMask >>= BRANCHFACTOR;
        height++;
    }
    return height;
}


// kate: indent-mode cstyle; space-indent on; indent-width 0; 
