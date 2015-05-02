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
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <iostream>
#include "commonDefs.h"
#include "EventEditor.h"
#include "DeleteCursorData.xbm"

using namespace std;
using namespace FTSPlot;

EventEditor::EventEditor ( SimpleViewWidget* plotWidget ) : GL_Layer()
{
    gui = new HideNotifyWidget;
    ui.setupUi ( gui );
    manualPosValue = "0";
    ui.manualPosLine->setText ( manualPosValue );
    connect ( ui.openEventListButton, SIGNAL ( clicked() ), this, SLOT ( openEventList() ) );
    connect ( ui.importFlatFileButton, SIGNAL ( clicked() ), this, SLOT ( importFlatFileSlot() ) );
    connect ( ui.exportFlatFileButton, SIGNAL ( clicked() ), this, SLOT ( exportFlatFileSlot() ) );
    connect ( ui.addEventatButton, SIGNAL ( clicked() ), this, SLOT ( addEventManual() ) );
    connect ( ui.manualPosLine, SIGNAL ( textEdited ( QString ) ), this, SLOT ( validateManualPosLine ( QString ) ) );
    connect ( ui.nextEventButton, SIGNAL ( clicked() ), this, SLOT ( nextEventSlot() ) );
    connect ( ui.previousEventButton, SIGNAL ( clicked() ), this, SLOT ( prevEventSlot() ) );
    connect ( ui.deleteThisEventButton, SIGNAL ( clicked() ), this, SLOT ( deleteEvent() ) );
    connect ( ui.plusButton, SIGNAL ( clicked() ), this, SLOT ( plusEvent() ) );
    connect ( ui.minusButton, SIGNAL ( clicked ( bool ) ), this, SLOT ( minusEvent() ) );
    connect ( ui.fineEdit, SIGNAL ( editingFinished() ), this, SLOT ( handleFineTune() ) );
    connect ( ui.trackViewCheckBox, SIGNAL ( stateChanged ( int ) ), this, SLOT ( toggleTrackBox ( int ) ) );
    connect ( gui, SIGNAL ( GUIupdate() ), this, SLOT ( relayGUIupdate() ) );

    gui->setAttribute ( Qt::WA_QuitOnClose, false );
    gui->show();

    svw = plotWidget;
    eel = new EventEditorLoader ( svw->context() );
    workerThread = new QThread( this );
    eel->moveToThread( workerThread );


    connect ( eel, SIGNAL ( notifyListUpdate() ),
              this, SLOT ( threadDone() ) );
    connect ( this, SIGNAL ( requestDisplayList ( qint64,qint64,int,QString,double,double ) ),
              eel, SLOT ( genDisplayList ( qint64,qint64,int,QString,double,double ) ) );

    QBitmap bitmap = QBitmap::fromData ( QSize ( DeleteCursorData_width, DeleteCursorData_height ),
                                         DeleteCursorData_bits );
    QBitmap mask = QBitmap::fromData ( QSize ( DeleteCursorData_width, DeleteCursorData_height ),
                                       DeleteCursorData_bits );

    DeleteCursor = QCursor ( bitmap, mask );

    state = NoTree;
    updateGUI();
    currentEvent = 0;
    eventListDirName = "";
    flatFileName = "";
    XdataBegin = 0;
    displayPower = 0;
}

EventEditor::~EventEditor()
{
    // disconnect some signals to avoid stupid accidents
    disconnect ( eel, SIGNAL ( notifyListUpdate() ), this, SLOT ( threadDone() ) );
    // stop & delete thread
    delete ( eel );
    // delete gui widget
    delete ( gui );
}


void EventEditor::paintGL()
{
    if ( state == normal )
    {
        svw->makeCurrent();
        eel->paintGL();

        // repaint currentEvent
        double xcoord = ( currentEvent - XdataBegin ) >> displayPower;
        glColor4f ( 1-red, 1-green, 1-blue, 0.5 );
        glBegin ( GL_LINES );
        glVertex2d ( xcoord, ymin );
        glVertex2d ( xcoord, ymax );
        glEnd();
    }
}

void EventEditor::genDisplayList ( qint64 reqXdataBegin, qint64 reqXdataEnd, int reqDispPower, double reqYFrustMax, double reqYFrustMin )
{
    QString path = treeRootDirName;
    if ( state == NoTree )
    {
        path = "";
    }

    // get ymin and ymax
    ymin = reqYFrustMin;
    ymax = reqYFrustMax;
    this->reqXdataBegin = reqXdataBegin;
    this->reqDisplayPower = reqDispPower;

    emit requestDisplayList ( reqXdataBegin, reqXdataEnd, reqDispPower, path, ymin, ymax );
}

void EventEditor::toggleLists()
{
    eel->toggleLists();
    XdataBegin = reqXdataBegin;
    displayPower = reqDisplayPower;
}

double EventEditor::getMin()
{
    return GL_Layer::getMin();
}

double EventEditor::getMax()
{
    return GL_Layer::getMax();
}

bool EventEditor::isOverEvent()
{
    // Get screen coordinates
    int mouseX = svw->mapFromGlobal( QCursor::pos() ).x();
    // convert them +/- 1 to graph coordinates
    quint64 xlow;
    if ( round ( svw->Xscreen2graph ( mouseX - 1 ) ) < 0 )
    {
        xlow = 0;
    }
    else
    {
        xlow = ( quint64 ) round ( svw->Xscreen2graph ( mouseX - 1 ) );
    }
    quint64 xhigh;
    if ( round ( svw->Xscreen2graph ( mouseX + 1 ) ) < 0 )
    {
        xhigh = 0;
    }
    else
    {
        xhigh = ( quint64 ) round ( svw->Xscreen2graph ( mouseX + 1 ) );
    }

    // check this interval for events
    bool overEvent = hasEvent ( xlow, xhigh, treeRootDirName, 0, TOTALHEIGHT );
    return overEvent;
}


void EventEditor::mouseMoveEvent ( QMouseEvent* event )
{
    if ( event->modifiers() == Qt::ShiftModifier )
    {
        // if add mode
        if ( ui.AddEventButton->isChecked() )
        {
            if ( svw->cursor().shape() != Qt::CrossCursor )
            {
                svw->setCursor ( Qt::CrossCursor );
            }
        }
        // if select mode
        if ( ui.SelectEventButton->isChecked() )
        {
            bool overEvent = isOverEvent();

            if ( overEvent && svw->cursor().shape() != Qt::PointingHandCursor )
            {
                svw->setCursor ( Qt::PointingHandCursor );
            }

            if ( !overEvent && svw->cursor().shape() != Qt::CrossCursor )
            {
                svw->setCursor ( Qt::CrossCursor );
            }
        }

        // if delete mode
        if ( ui.DeleteEventButton->isChecked() )
        {
            bool overEvent = isOverEvent();

            if ( overEvent && svw->cursor().shape() != Qt::BitmapCursor )
            {
                svw->setCursor ( DeleteCursor );
            }

            if ( !overEvent && svw->cursor().shape() != Qt::CrossCursor )
            {
                svw->setCursor ( Qt::CrossCursor );
            }
        }
        event->accept();
    }
    else
    {
        // reset cursors
        if ( svw->cursor().shape() != Qt::ArrowCursor )
        {
            svw->setCursor ( Qt::ArrowCursor );
        }
        event->ignore();
    }

}

void EventEditor::mousePressEvent ( QMouseEvent* event )
{

    if ( event->modifiers() == Qt::ShiftModifier )
    {

        if ( ui.AddEventButton->isChecked() )
        {
            long double xpos = svw->Xscreen2graph ( event->x() );
            if ( xpos < 0 )
            {
                addEvent ( 0 );
            }
            else
            {
                addEvent ( ( quint64 ) xpos );
            }
        }

        if ( ui.SelectEventButton->isChecked() )
        {
            // Get screen coordinates
            int screenx = event->x();
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
            if ( hasEvent( xlow, xhigh, treeRootDirName, 0, TOTALHEIGHT ) )
            {
                // good, we have an event in the interval

                // first try xlow directly
                bool found = false;
                quint64 nextEv;
                if ( hasEvent( xlow ) )
                {
                    nextEv = xlow;
                    found = true;
                }
                else // then in the interval
                {
                    try
                    {
                        nextEv = nextEvent( xlow );
                        if ( nextEv <= xhigh )
                        {
                            found = true;
                        }
                    }
                    catch (bool) {}
                }

                if ( found )
                {
                    // now set it
                    currentEvent = nextEv;
                    updateGUI();
                    updateCursor();
                }
            }
        }

        if ( ui.DeleteEventButton->isChecked() )
        {
            // Get screen coordinates
            int screenx = event->x();
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
            delEventInRange ( xlow, xhigh );


            if ( !hasEvent ( currentEvent ) )
            {
                try
                {
                    currentEvent = nextEvent( currentEvent );
                    updateGUI();
                    updateCursor();
                }
                catch ( bool err )
                {
                }

                // if not found search prev event
                try
                {
                    currentEvent = prevEvent( currentEvent );
                    updateGUI();
                    updateCursor();
                }
                catch ( bool err )
                {
                }

            }
        }

        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void EventEditor::wheelEvent ( QWheelEvent* event )
{
    if ( event->modifiers() == Qt::ShiftModifier )
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void EventEditor::keyPressEvent ( QKeyEvent* event )
{
    if ( event->key() == Qt::Key_Shift )
    {
        if ( isOverEvent() )
        {
            if ( ui.AddEventButton->isChecked() )
            {
                svw->setCursor( Qt::CrossCursor );
            }
            if ( ui.SelectEventButton->isChecked() )
            {
                svw->setCursor( Qt::PointingHandCursor );
            }
            if ( ui.DeleteEventButton->isChecked() )
            {
                svw->setCursor( DeleteCursor );
            }
        }
        else
        {
            svw->setCursor ( Qt::CrossCursor );
        }
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void EventEditor::keyReleaseEvent ( QKeyEvent* event )
{
    if ( event->key() == Qt::Key_Shift )
    {
        svw->setCursor ( Qt::ArrowCursor );
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void EventEditor::setColor ( QColor color )
{
    myColor = color;
    red = myColor.redF();
    green = myColor.greenF();
    blue = myColor.blueF();
    eel->setColor ( color );
}

QString EventEditor::getPath()
{
    return treeRootDirName;
}

bool EventEditor::findFirstEvent ( quint64* event, QString path, int height )
{
    if ( height == 0 ) // blockfile level
    {
        QFile blockFile ( path );
        if ( !blockFile.open ( QIODevice::ReadOnly ) )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Fatal: Cannot open " + blockFile.fileName() + ": " + blockFile.errorString() );
            myMessBox.exec();
            exit ( 1 );
        }
        blockFile.read ( ( char* ) event, sizeof ( quint64 ) );
        blockFile.close();
        return true;
    }
    else if ( height == 1 ) // last dir level
    {
        QDir currentDir ( path );
        currentDir.setNameFilters( QStringList( QString( "*.block" ) ) );
        QStringList blockFileList = currentDir.entryList ( QDir::Files | QDir::NoDotAndDotDot, QDir::Name );

        if ( blockFileList.isEmpty() )
        {
            return false;
        }
        return ( findFirstEvent ( event, path + "/" + blockFileList.first(), height -1 ) );
    }
    else // higher dirs
    {
        QDir currentDir ( path );
        QStringList subDirList = currentDir.entryList ( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );

        if ( subDirList.isEmpty() )
        {
            return false;
        }
        return ( findFirstEvent ( event, path + "/" + subDirList.first(), height -1 ) );
    }
}

bool EventEditor::findLastEvent ( quint64* event, QString path, int height )
{
    if ( height == 0 ) // block file level
    {
        QFile blockFile ( path );
        if ( !blockFile.open ( QIODevice::ReadOnly ) )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Fatal: Cannot open " + blockFile.fileName() + ": " + blockFile.errorString() );
            myMessBox.exec();
            exit ( 1 );
        }
        if ( !blockFile.seek ( blockFile.size() - sizeof ( quint64 ) ) )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Fatal: Cannot seek to end of " + blockFile.fileName() + ": " + blockFile.errorString() );
            myMessBox.exec();
            exit ( 1 );
        }
        blockFile.read ( ( char* ) event, sizeof ( quint64 ) );
        blockFile.close();
        return true;
    }
    else if ( height == 1 ) // last dir level
    {
        QDir currentDir ( path );
        currentDir.setNameFilters( QStringList( QString( "*.block" ) ) );
        QStringList blockFileList = currentDir.entryList ( QDir::Files | QDir::NoDotAndDotDot, QDir::Name );

        if ( blockFileList.isEmpty() )
        {
            return false;
        }
        return ( findLastEvent ( event, path + "/" + blockFileList.last(), height -1 ) );
    }
    else // higher dir levels
    {
        QDir currentDir ( path );
        QStringList subDirList = currentDir.entryList ( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );

        if ( subDirList.isEmpty() )
        {
            return false;
        }
        return ( findLastEvent ( event, path + "/" + subDirList.last(), height -1 ) );
    }
}

bool EventEditor::isFirstEvent ( quint64 ev )
{
    quint64 firstEvent;
    if ( findFirstEvent ( &firstEvent, treeRootDirName, TOTALHEIGHT ) )
    {
        return ( ev == firstEvent );
    }
    return false;
}

bool EventEditor::isLastEvent ( quint64 ev )
{
    quint64 lastElement;
    if ( findLastEvent ( &lastElement, treeRootDirName, TOTALHEIGHT ) )
    {
        return ( ev == lastElement );
    }
    return false;
}

bool EventEditor::openTreeDir ( QString EventListDirectory )
{
    if ( state != NoTree )
    {
        return false;
    }
    EventEditorLoader_Suspend lock( workerThread );
    eventListDirName = QDir ( EventListDirectory ).absolutePath();

    // either empty or having a "root" directory

    QDir eventListDir( eventListDirName );
    if ( eventListDir.count() == 2 ) // if empty ...
    {
        // ... create the "root" directory
        if ( !eventListDir.mkdir( "root" ) )
        {
            return false;
        }
    }

    // check for "root" dir
    QFileInfo treeRootInfo( eventListDirName + "/root" );
    if ( !treeRootInfo.isDir() )
    {
        QMessageBox MyMessBox;
        MyMessBox.setText ( "Root directory " + eventListDirName + "/root missing." );
        MyMessBox.exec();
        return false;
    }
    treeRootDirName = eventListDirName + "/root";

    QDir treeDir ( treeRootDirName );
    if ( treeDir.entryInfoList ( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name ).size() == 0 )
    {
        state = EmptyTree;
    }
    else
    {
        // search first entry
        quint64 firstEvent;
        if ( findFirstEvent ( &firstEvent, treeRootDirName, TOTALHEIGHT ) )
        {
            currentEvent = firstEvent;
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
    updateCursor();
    disconnect ( ui.openEventListButton, SIGNAL ( clicked() ),
                 this, SLOT ( openEventList() ) );
    connect ( ui.openEventListButton, SIGNAL ( clicked() ),
              this, SLOT ( closeEventList() ) );
    originalModuleName = svw->getModuleName( this );
    QFileInfo eventListBaseDir( eventListDirName );
    if( !svw->setModuleName( this, eventListBaseDir.baseName() ) )
    {
        qDebug() << "Cannot set module name.";
    }

    // if dataset is not empty signal to ViewWidget to update the displaylists
    if ( state == EmptyTree )
    {
        return true;;
    }
    emit ( triggerRepaint ( this ) );
    return true;
}


void EventEditor::openEventList()
{
    // load new data
    eventListDirName =
        QFileDialog::getExistingDirectory ( NULL, "EventList Tree Directory", eventListDirName, QFileDialog::ShowDirsOnly );

    if ( !eventListDirName.isEmpty() )
    {
        openTreeDir ( eventListDirName );
    }
}

bool EventEditor::closeTreeDir()
{
    if ( state == NoTree )
    {
        return false;
    }
    EventEditorLoader_Suspend lock( workerThread );
    closeEventList();
    return true;
}


void EventEditor::closeEventList()
{
    // set state to NoTree
    state = NoTree;
    updateGUI();
    updateCursor();

    disconnect ( ui.openEventListButton, SIGNAL ( clicked() ),
                 this, SLOT ( closeEventList() ) );
    connect ( ui.openEventListButton, SIGNAL ( clicked() ),
              this, SLOT ( openEventList() ) );
    if( !svw->setModuleName( this, originalModuleName ) )
    {
        qDebug() << "Cannot reset module name.";
    }
    
    emit triggerRepaint ( this );
}

bool EventEditor::importFlatFile ( QString flatFileName )
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

bool EventEditor::importFlatFileMain ( QString flatFileName ) throw ( QString )
{
    // delete directory tree
    if ( RemoveDirectory ( treeRootDirName ) )
    {
        throw "Cannot clean " + treeRootDirName;
        return false;
    }
    // delete "root.node" file
    QFile nodeFile( treeRootDirName + ".node" );
    if ( nodeFile.exists() )
    {
        if ( !nodeFile.remove() )
        {
            throw "Cannot delete " + treeRootDirName + ".node";
            return false;
        }
    }

    // open flat file
    QFile flatFile ( flatFileName );
    if ( !flatFile.open ( QIODevice::ReadOnly ) )
    {
        throw "Cannot open " +  flatFileName + ": " + flatFile.errorString();
        return false;
    }

    if ( flatFile.size() == 0 )
    {
        state = EmptyTree;
        return true;
    }

    // mmap the flat file
    quint64* flatDataField = ( quint64* ) flatFile.map ( 0, flatFile.size() );
    if ( flatDataField == NULL )
    {
        throw "Cannot mmap " + flatFileName + ".";
        return false;
    }

    // Why this complicated and not just statically defining it?
    // Depending on BLOCKFACTOR and TOTALHEIGHT cacheLine can become quite large,
    // larger than the stack limit => therefore we instantiate it on the heap
    typedef quint64 cacheLine[1<<BLOCKFACTOR];
    QScopedArrayPointer<cacheLine> nodeCache( new cacheLine[TOTALHEIGHT+1] );

    int nodeCacheFill[TOTALHEIGHT+1];

    for ( int i = 0; i < TOTALHEIGHT+1; i++ )
    {
        nodeCacheFill[i] = 0;
    }
    //quint64 dataBuffer[1<<BLOCKFACTOR];
    QScopedPointer<cacheLine> dataBuffer( new cacheLine[1] );
    quint64 dataBufferFill = 0;
    QFile blockFile;

    quint64 mask = height2mask( 0 );
    quint64 endIdx = flatFile.size() / sizeof ( quint64 );
    quint64 oldValue = 0;
    for ( quint64 idx = 0; idx < endIdx; idx++ )
    {
        //read value
        quint64 value = flatDataField[idx];
        if ( value < oldValue )
        {
            throw "Event values in Flatfile have to be ordered ascendingly! Exiting.";
            return false;
        }

        //add value
        //dataBuffer[dataBufferFill++] = value;
        (*dataBuffer)[dataBufferFill++] = value;
        nodeCache[0][nodeCacheFill[0]++] = value;
        //--
        //lookahead
        //endoffile or
        //(newvalue & mask) != (value & mask)
        quint64 newValue;
        if ( idx+1 == endIdx || (flatDataField[idx+1] & mask) != (value & mask) )
        {
            // create directories
            QDir treeDir ( treeRootDirName );
            if ( !treeDir.mkpath ( generatePath( value, 1 )) )
            {
                throw "Cannot open create directory path " + generatePath( value, 1 );
                return false;
            }

            // open file
            blockFile.setFileName ( generatePath(value, 0, ".block" ) );
            if ( !blockFile.open ( QIODevice::WriteOnly ) )
            {
                throw "Cannot open " +  blockFile.fileName() + ": " + blockFile.errorString();
                return false;
            }

            // write out dataBuffer, reset bufferfill, close file
            //if ( blockFile.write ( ( char* ) dataBuffer, dataBufferFill * sizeof ( quint64 ) ) != dataBufferFill * sizeof ( quint64 ) )
            if ( blockFile.write ( ( char* ) (*dataBuffer), dataBufferFill * sizeof ( quint64 ) ) != dataBufferFill * sizeof ( quint64 ) )
            {
                throw "Cannot write to " +  blockFile.fileName() + ": " + blockFile.errorString();
                return false;
            }
            blockFile.close();
            dataBufferFill = 0;

            //generate caches
            QStringList pathComponentsPrefix = generatePathComponents( value );
            QStringList newPathComponentsPrefix;
            if ( idx+1 != endIdx )
            {
                newPathComponentsPrefix = generatePathComponents( flatDataField[idx+1] );
            }
            else
            {
                newPathComponentsPrefix = generatePathComponents( value );
                newPathComponentsPrefix.append ("xx"); // this way we will also generate the root.node
            }

            for ( int height = 1; height < TOTALHEIGHT+1; height++ )
            {
                quint64 mask = nodeMask( height );

                for ( int j = 0; j < nodeCacheFill[height-1]; j++ )
                {
                    quint64 maskedValue = nodeCache[height-1][j] & mask;
                    if ( nodeCacheFill[height] == 0 || maskedValue != nodeCache[height][nodeCacheFill[height]-1] )
                    {
                        nodeCache[height][nodeCacheFill[height]++] = maskedValue;
                    }
                }
                nodeCacheFill[height-1] = 0;

                // get components
                // chop last elements
                // check them!
                pathComponentsPrefix.removeLast();
                newPathComponentsPrefix.removeLast();

                if ( pathComponentsPrefix != newPathComponentsPrefix )
                {
                    // generate cache file
                    if ( nodeCacheFill[height] != 0 )
                    {
                        // Write out new cache file
                        QFile nodeFile( generatePath( value, height, ".node" ) );
                        if ( nodeFile.open ( QIODevice::WriteOnly ) )
                        {
                            nodeFile.write ( ( char* ) &nodeCache[height][0], nodeCacheFill[height] * sizeof ( quint64 ) );
                            nodeFile.close();
                        }
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }

    flatFile.unmap ( ( uchar* ) flatDataField );
    flatFile.close();

    state = normal;

    updateGUI();

    emit ( triggerRepaint ( this ) );

    return true;
}


void EventEditor::importFlatFileSlot()
{
    flatFileName =
        QFileDialog::getOpenFileName ( NULL, "Choose Flat EventList file", flatFileName, "Flat EventList Files (*.evl);;All Files (*)", 0, NULL );
    if ( flatFileName.isEmpty() )
        return;

    bool result;
    try
    {
        result = importFlatFile ( flatFileName );
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

bool EventEditor::exportFlatFile ( QString flatFileName )
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


bool EventEditor::exportFlatFileMain ( QString flatFileName ) throw ( QString )
{
    // open flat file
    QFile flatFile ( flatFileName );
    if ( !flatFile.open ( QIODevice::WriteOnly ) )
    {
        throw "Cannot open " +  flatFile.fileName() + ": " + flatFile.errorString();
        return false;
    }
    // call recursive tree export
    bool result;
    result = recursiveTreeExport ( treeRootDirName, flatFile, TOTALHEIGHT );

    // close flat file
    flatFile.close();
    return result;
}


void EventEditor::exportFlatFileSlot()
{
    flatFileName =
        QFileDialog::getSaveFileName ( NULL, "Choose Flat EventList file", flatFileName, "Flat EventList Files (*.evl);;All Files (*)", 0, NULL );
    if ( flatFileName.isEmpty() )
        return;
    if ( !flatFileName.endsWith ( ".evl" ) )
        flatFileName.append ( ".evl" );

    bool result;
    try
    {
        result = exportFlatFile ( flatFileName );
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

bool EventEditor::recursiveTreeExport ( QString currentDirName, QFile& flatFile, int height ) throw ( QString )
{
    if ( height == 0 )
    {
        QString blockFileName = currentDirName;
        QFile blockFile ( blockFileName );
        if ( !blockFile.open ( QIODevice::ReadOnly ) )
        {
            throw "Cannot open " +  blockFile.fileName() + ": " + blockFile.errorString();
            return false;
        }
        qint64 blockFileSize = blockFile.size();

        uchar* blockFileMap = blockFile.map ( 0, blockFileSize );
        if ( blockFileMap == NULL )
        {
            throw "Cannot memory map " +  blockFile.fileName() + ": " + blockFile.errorString() + ".";
            return false;
        }
        if ( flatFile.write ( ( char* ) blockFileMap, blockFileSize ) != blockFileSize )
        {
            throw "Cannot write to flat file " +  flatFile.fileName() + ": " + flatFile.errorString() + ".";
            return false;
        }
        blockFile.unmap ( ( uchar* ) blockFileMap );
        blockFile.close();
    }
    else if ( height == 1 )
    {
        // get directory listing
        QDir currentDir ( currentDirName );
        currentDir.setNameFilters( QStringList( QString( "*.block" ) ) );

        // subdir List
        QStringList subDirList = currentDir.entryList ( QDir::NoDotAndDotDot | QDir::Files, QDir::Name );

        // error checking
        if ( subDirList.isEmpty() )
        {
            throw "Error: Malformed directory tree.";
            return false;
        }

        // still in tree => recursive descent

        for ( int i = 0; i < subDirList.size(); i++ )
        {
            if ( !recursiveTreeExport ( currentDirName + "/" + subDirList[i], flatFile, height - 1 ) )
            {
                return false;
            }
        }
    }
    else
    {
        // get directory listing
        QDir currentDir ( currentDirName );

        // subdir List
        QStringList subDirList = currentDir.entryList ( QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name );

        // error checking
        if ( subDirList.isEmpty() )
        {
            throw "Error: Malformed directory tree.";
            return false;
        }

        // still in tree => recursive descent

        for ( int i = 0; i < subDirList.size(); i++ )
        {
            if ( !recursiveTreeExport ( currentDirName + "/" + subDirList[i], flatFile, height - 1 ) )
            {
                return false;
            }
        }
    }

    return true;
}

void EventEditor::updateGUI()
{
    switch ( state )
    {
    case ( NoTree ) :
        ui.openEventListButton->setText ( "Open EventList" );
        ui.openEventListButton->setEnabled ( true );
        ui.importFlatFileButton->setEnabled ( false );
        ui.exportFlatFileButton->setEnabled ( false );
        ui.AddEventButton->setEnabled ( false );
        ui.deleteThisEventButton->setEnabled ( false );
        ui.SelectEventButton->setEnabled( false );
        ui.DeleteEventButton->setEnabled ( false );
        ui.addEventatButton->setEnabled ( false );
        ui.manualPosLine->setEnabled ( false );
        ui.fineEdit->setEnabled ( false );
        ui.plusButton->setEnabled ( false );
        ui.minusButton->setEnabled ( false );
        ui.previousEventButton->setEnabled ( false );
        ui.nextEventButton->setEnabled ( false );
        ui.trackViewCheckBox->setEnabled ( false );

        break;
    case ( EmptyTree ) :
        ui.openEventListButton->setText ( "Close EventList" );
        ui.openEventListButton->setEnabled ( true );
        ui.importFlatFileButton->setEnabled ( true );
        ui.exportFlatFileButton->setEnabled ( true );
        ui.AddEventButton->setEnabled ( true );
        ui.deleteThisEventButton->setEnabled ( false );
        ui.SelectEventButton->setEnabled( false );
        ui.DeleteEventButton->setEnabled ( false );
        ui.addEventatButton->setEnabled ( true );
        ui.manualPosLine->setEnabled ( true );
        ui.fineEdit->setEnabled ( false );
        ui.plusButton->setEnabled ( false );
        ui.minusButton->setEnabled ( false );
        ui.previousEventButton->setEnabled ( false );
        ui.nextEventButton->setEnabled ( false );
        ui.trackViewCheckBox->setEnabled ( false );
        break;
    case ( normal ) :
        ui.openEventListButton->setText ( "Close EventList" );
        ui.openEventListButton->setEnabled ( true );
        ui.importFlatFileButton->setEnabled ( true );
        ui.exportFlatFileButton->setEnabled ( true );
        ui.AddEventButton->setEnabled ( true );
        ui.deleteThisEventButton->setEnabled ( true );
        ui.SelectEventButton->setEnabled( true );
        ui.DeleteEventButton->setEnabled ( true );
        ui.addEventatButton->setEnabled ( true );
        ui.manualPosLine->setEnabled ( true );
        ui.fineEdit->setEnabled ( true );
        ui.plusButton->setEnabled ( true );
        ui.minusButton->setEnabled ( true );
        ui.trackViewCheckBox->setEnabled ( true );

        ui.fineEdit->setText ( QString::number ( currentEvent ) );

        ui.previousEventButton->setDisabled ( isFirstEvent ( currentEvent ) || isEmpty() );
        ui.nextEventButton->setDisabled ( isLastEvent ( currentEvent )  || isEmpty() );

        break;
    default:
        break;
    }
}

void EventEditor::updateCursor()
{
    if ( ui.trackViewCheckBox->isChecked() )
    {
        emit centerCoordsOn ( currentEvent, DoubleNaN, DoubleNaN, DoubleNaN );
    }
    emit ( triggerRepaint ( this ) );
}


bool EventEditor::RemoveDirectory ( QDir aDir )
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


QString EventEditor::generatePath(quint64 event, int height, QString suffix)
{
    QStringList components = generatePathComponents( event );
    QString result = treeRootDirName;
    for ( int i = 0; i < TOTALHEIGHT - height; i++ )
    {
        result += "/" + components[i];
    }
    result += suffix;
    return result;
}


void EventEditor::threadDone()
{
    emit notifyListUpdate ( this );
}

bool EventEditor::addEvent ( quint64 xpos )
{
    if ( state == NoTree )
    {
        return false;
    }
    // Shut down thread to avoid interference
    EventEditorLoader_Suspend lock ( workerThread );

    QString filePath = generatePath( xpos, 0, ".block" );

    if ( !QFile::exists ( filePath ) )
    {
        QDir treeDir ( treeRootDirName );
        QString dirPath = generatePath( xpos, 1 );
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
        if ( blockFile.write ( ( char* ) &xpos, sizeof ( quint64 ) ) != sizeof ( quint64 ) )
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

        QByteArray buffer ( blockFile.size() + sizeof ( quint64 ), '0' );
        quint64* bufPtr = ( quint64* ) buffer.data();
        if ( blockFile.read ( ( char* ) bufPtr, blockFile.size() ) != blockFile.size() )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Cannot read from " + blockFile.fileName() + " " + blockFile.errorString() );
            myMessBox.exec();
            return false;
        }

        for ( quint64 i = blockFile.size() / sizeof ( quint64 ) - 1; i >= 0 ; i-- )
        {
            if ( bufPtr[i] == xpos )
            {
                return false;
            }
            if ( bufPtr[i] < xpos )
            {
                bufPtr[i+1] = xpos;
                break;
            }
            bufPtr[i+1] = bufPtr[i];
            if ( i == 0 )
            {
                bufPtr[0] = xpos;
                break;
            }
        }

        qint64 oldBlockFileSize = blockFile.size();
        blockFile.seek ( 0 );
        if ( blockFile.write ( ( char* ) bufPtr, blockFile.size() + sizeof ( quint64 ) ) != oldBlockFileSize + sizeof ( quint64 ) )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Cannot write to " + blockFile.fileName() + " " + blockFile.errorString() );
            myMessBox.exec();
            return false;
        }
        blockFile.close();
    }

    // update the node files
    QStringList pathToBlockFile = generatePathComponents( xpos );

    // iterate over the directories
    for ( int height = 1; height <= TOTALHEIGHT; height++ )
    {
        QString path = treeRootDirName;
        for ( int j = 0; j < TOTALHEIGHT - height; j++ )
        {
            path += "/" + pathToBlockFile[j];
        }

        // mask xpos accordingly
        quint64 mask = nodeMask( height );
        quint64 maskedXpos = xpos & mask;

        // search it
        bool found = false;
        QFile nodeFile;
        nodeFile.setFileName( path + ".node" );
        if ( nodeFile.open ( QIODevice::ReadOnly ) )
        {
            quint64* nodeData = ( quint64* ) nodeFile.map ( 0, nodeFile.size() );
            if ( nodeData != NULL )
            {
                for ( qint64 j = 0; j < nodeFile.size() / sizeof ( quint64 ); j++ )
                {
                    if ( nodeData[j] == maskedXpos )
                    {
                        found = true;
                        break;
                    }
                }
            }
        }
        nodeFile.close();

        if ( found ) // if does exist => break and finish
        {
            break;
        }
        else // if doesn't exist yet => write new
        {
            // add it
            if ( nodeFile.open ( QIODevice::ReadWrite ) )
            {
                // seek
                if ( !nodeFile.seek( nodeFile.size() ) )
                {
                    qDebug() << "Cannot seek to the end of" << nodeFile.fileName();
                    return false;
                }

                // add a long long
                quint64 dummy = maskedXpos; // for the case of zero size node files
                if ( nodeFile.write( (char*) &dummy, sizeof( quint64 ) ) != sizeof( quint64 ) )
                {
                    qDebug() << "Cannot write to" << nodeFile.fileName();
                    return false;
                }

                // reopen to update file size structures
                nodeFile.close();
                if ( nodeFile.open( QIODevice::ReadWrite ) )
                {
                    // mmap
                    quint64* nodeData = ( quint64* ) nodeFile.map ( 0, nodeFile.size() );
                    if ( nodeData != NULL )
                    {
                        // back iterate and insert new xpos
                        for ( qint64 i = (nodeFile.size() / sizeof(quint64))-2; i >= 0; i-- )
                        {
                            if( nodeData[i] > maskedXpos ){
                                nodeData[i+1] = nodeData[i];
                                nodeData[i] = maskedXpos;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }

                    nodeFile.close();
                }
            }
        }
    }

    if ( state != normal )
    {
        currentEvent = xpos;
        state = normal;
    }
    updateGUI();
    updateCursor();
    emit ( triggerRepaint ( this ) );
    return true;
}

bool EventEditor::isEmpty()
{
    QDir treeDir ( treeRootDirName );
    QStringList subDirList = treeDir.entryList ( QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDir::Name );
    return subDirList.isEmpty();
}

bool EventEditor::delEvent ( quint64 event )
{
    return delEventInRange ( event, event );
}


bool EventEditor::delEventInRange ( quint64 xBegin, quint64 xEnd )
{
    if ( state != normal )
    {
        return false;
    }

    EventEditorLoader_Suspend lock ( workerThread );
    quint64 deletedEvent;
    if ( delEvent_sub ( xBegin, xEnd, &deletedEvent, treeRootDirName, 0, TOTALHEIGHT ) )
    {
        if ( deletedEvent == currentEvent )
        {
            try
            {
                currentEvent = nextEvent( currentEvent );
            }
            catch ( bool err ) {}

            try
            {
                currentEvent = prevEvent( currentEvent );
            }
            catch ( bool err ) {}

            updateGUI();
            updateCursor();
        }

        if ( isEmpty() )
        {
            state = EmptyTree;
            updateGUI();
        }
    }
    else
    {
        return false;
    }
    emit triggerRepaint ( this );
    return true;
}

bool EventEditor::delEvent_sub ( quint64 xBegin, quint64 xEnd, quint64* deletedEvent, QString path, quint64 pathValue, int height )
{
    QDir currentDir( path );
    bool nodeUpdateNeeded = false;
    if ( height == 0 ) // filename level
    {
        QString blockFileName = path;
        QFile blockFile ( blockFileName );
        if ( !blockFile.open ( QIODevice::ReadWrite ) )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Cannot open " +  blockFile.fileName() + ": " + blockFile.errorString() );
            myMessBox.exec();
            return false;
        }

        QByteArray blockData ( blockFile.size() + sizeof ( quint64 ), '0' );
        if ( blockFile.read ( blockData.data(), blockFile.size() ) != blockFile.size() )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Cannot read from " + blockFile.fileName() + " " + blockFile.errorString() );
            myMessBox.exec();
            return false;
        }

        quint64* blockPtr = ( quint64* ) blockData.data();
        bool found = false;
        for ( quint64 i = 0; i < blockFile.size() / sizeof ( quint64 ); i++ )
        {
            quint64 value = blockPtr[i];
            if ( xBegin <= value && value <= xEnd )
            {
                found = true;
                *deletedEvent = value;
            }
            if ( found )
            {
                blockPtr[i] = blockPtr[i+1];
            }
        }

        if ( found )
        {
            blockFile.seek ( 0 );
            qint64 oldBlockFileSize = blockFile.size();
            if ( !blockFile.resize ( 0 ) )
            {
                QMessageBox myMessBox;
                myMessBox.setText ( "Cannot resize " + blockFile.fileName() + " " + blockFile.errorString() );
                myMessBox.exec();
                return false;
            }
            if ( blockFile.write ( blockData.data(), oldBlockFileSize - sizeof ( quint64 ) ) != oldBlockFileSize - sizeof ( quint64 ) )
            {
                QMessageBox myMessBox;
                myMessBox.setText ( "Cannot write to " + blockFile.fileName() + " " + blockFile.errorString() );
                myMessBox.exec();
                return false;
            }
        }

        if ( blockFile.size() == 0 )
        {
            blockFile.remove();
            return true;
        }
        blockFile.close();

        return true;
    }
    else if ( height == 1 ) // directory level over block files
    {
        // get directory listing
        currentDir.setNameFilters( QStringList( QString( "*.block" ) ) );
        QStringList blockFileList = currentDir.entryList ( QDir::NoDotAndDotDot | QDir::Files, QDir::Name );

        // still in tree => recursive descent

        for ( int i = 0; i < blockFileList.size(); i++ )
        {
            QString coreName = blockFileList[i];
            coreName.chop( 6 );

            quint64 sliceValueBegin = dirName2sliceMin( coreName, height-1, pathValue );
            quint64 sliceValueEnd = dirName2sliceMax( coreName, height-1, pathValue );

            if ( xEnd < sliceValueBegin )
            {
                break;
            }
            if ( xBegin <= sliceValueEnd )
            {
                if ( delEvent_sub ( xBegin, xEnd, deletedEvent, path + "/" + blockFileList[i], sliceValueBegin, height - 1 ) )
                {
                    nodeUpdateNeeded = true;
                    break;
                }
            }
        }


        blockFileList = currentDir.entryList ( QDir::NoDotAndDotDot | QDir::Files, QDir::Name );
        if ( blockFileList.isEmpty() ) // last block file has been deleted
        {
            // delete directory
            QDir deleteDir( treeRootDirName );
            if ( !deleteDir.rmdir( path ) )
            {
                qDebug() << "Cannot delete" << path;
                return false;
            }
            // delete node file
            QFile nodeFile( path + ".node" );
            if ( !nodeFile.remove() )
            {
                qDebug() << "Cannot delete" << nodeFile.fileName();
                return false;
            }
            return true;
        }
    }
    else // higher directory levels
    {
        // get directory listing
        QStringList subDirList = currentDir.entryList ( QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name );

        // still in tree => recursive descent

        for ( int i = 0; i < subDirList.size(); i++ )
        {
            quint64 sliceValueBegin = dirName2sliceMin( subDirList[i], height-1, pathValue );
            quint64 sliceValueEnd = dirName2sliceMax( subDirList[i], height-1, pathValue );

            if ( xEnd < sliceValueBegin )
            {
                break;
            }
            if ( xBegin <= sliceValueEnd )
            {
                if ( delEvent_sub ( xBegin, xEnd, deletedEvent, path + "/" + subDirList[i], sliceValueBegin, height - 1 ) )
                {
                    nodeUpdateNeeded = true;
                    break;
                }
            }
        }

        subDirList = currentDir.entryList ( QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name );
        if ( subDirList.isEmpty() ) // last block file has been deleted
        {
            // delete directory
            QDir deleteDir( treeRootDirName );
            if ( !deleteDir.rmdir( path ) )
            {
                qDebug() << "Cannot delete" << path;
                return false;
            }
            // delete node file
            QFile nodeFile( path + ".node" );
            if ( !nodeFile.remove() )
            {
                qDebug() << "Cannot delete" << nodeFile.fileName();
                return false;
            }
            return true;
        }
    }

    if ( nodeUpdateNeeded )
    {
        // update node files

        // check this interval for another event (hasEvent)
        quint64 mask = height2mask( height );
        quint64 beginInterval = *deletedEvent & mask;
        quint64 endInterval = *deletedEvent | ~mask;

        if ( hasEvent( beginInterval, endInterval, treeRootDirName, 0, TOTALHEIGHT ) )// if found => break
        {
            return true;
        }

        // if !found => regenerate

        QFile::remove( path + ".node" );

        if ( height == 1 )
        {
            // get file list
            currentDir.setNameFilters( QStringList( QString( "*.block" ) ) );
            QStringList fileList = currentDir.entryList ( QDir::NoDotAndDotDot | QDir::Files, QDir::Name );
            if ( !fileList.isEmpty() )
            {
                QFile blockFile;
                QVector<quint64> vec( BLOCKFACTOR );
                foreach ( QString blockFileName, fileList )
                {
                    blockFile.setFileName ( path + "/" + blockFileName );
                    if ( blockFile.open ( QIODevice::ReadOnly ) )
                    {
                        quint64* blockData = ( quint64* ) blockFile.map ( 0, blockFile.size() );
                        if ( blockData != NULL )
                        {
                            quint64 mask = nodeMask( height );

                            for ( int j = 0; j < blockFile.size() / sizeof ( quint64 ); j++ )
                            {
                                quint64 maskedValue = blockData[j] & mask;
                                if ( vec.isEmpty() || maskedValue != vec.last() )
                                {
                                    vec.append( maskedValue );
                                }
                            }

                            blockFile.unmap ( ( uchar* ) blockData );
                        }
                        blockFile.close();
                    }
                }

                if ( !vec.isEmpty() )
                {
                    // Write out new cache file
                    QFile nodeFile ( path + ".node" );
                    if ( nodeFile.open ( QIODevice::WriteOnly ) )
                    {
                        nodeFile.write ( ( char* ) vec.data(), vec.size() * sizeof ( quint64 ) );
                        nodeFile.close();
                    }
                }
            }
        }
        if ( height > 1 )
        {
            // get dir list
            QStringList subDirList = currentDir.entryList ( QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name );
            QVector<quint64> vec( BLOCKFACTOR );
            QFile nodeFile;
            for ( int i = 0; i < subDirList.size(); i++ )
            {
                nodeFile.setFileName( path + "/" + subDirList[i] + ".node" );

                if ( nodeFile.open ( QIODevice::ReadOnly ) )
                {
                    quint64* nodeData = ( quint64* ) nodeFile.map ( 0, nodeFile.size() );
                    if ( nodeData != NULL )
                    {
                        quint64 mask = nodeMask( height );

                        for ( int j = 0; j < nodeFile.size() / sizeof ( quint64 ); j++ )
                        {
                            quint64 maskedValue = nodeData[j] & mask;
                            if ( vec.isEmpty() || maskedValue != vec.last() )
                            {
                                vec.append( maskedValue );
                            }
                        }
                        nodeFile.unmap ( ( uchar* ) nodeData );
                    }
                    nodeFile.close();
                }
            }

            if ( !vec.isEmpty() )
            {
                // Write out new cache file
                QFile nodeFile ( path + ".node" );
                if ( nodeFile.open ( QIODevice::WriteOnly ) )
                {
                    nodeFile.write ( ( char* ) vec.data(), vec.size() * sizeof ( quint64 ) );
                    nodeFile.close();
                }
            }
        }

        QStringList subDirList = currentDir.entryList ( QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDir::Name );
        // check for empty directory
        if ( subDirList.isEmpty() && height != TOTALHEIGHT )
        {
            currentDir.rmdir ( path );
        }
        return true;
    }
    return false;
}


quint64 EventEditor::getCurrentEvent()
{
    return currentEvent;
}

bool EventEditor::setCurrentEvent ( quint64 xpos )
{
    if ( !hasEvent ( xpos ) )
    {
        return false;
    }
    currentEvent = xpos;
    updateCursor();
    return true;
}


bool EventEditor::hasEvent ( quint64 xpos )
{
    return hasEvent ( xpos, xpos, treeRootDirName, 0, TOTALHEIGHT );
}

bool EventEditor::hasEvent ( quint64 xBegin, quint64 xEnd, QString path, quint64 pathValue, int height )
{
    if ( height == 0 ) // block file level
    {
        QString blockFileName = path;
        QFile blockFile ( blockFileName );
        if ( !blockFile.open ( QIODevice::ReadOnly ) )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Cannot open " +  blockFile.fileName() + ": " + blockFile.errorString() );
            myMessBox.exec();
            return false;
        }

        quint64* blockFileMap = ( quint64* ) blockFile.map ( 0, blockFile.size() );
        if ( blockFileMap == NULL )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Cannot memory map " +  blockFile.fileName() + ": " + blockFile.errorString() + "." );
            myMessBox.exec();
            return false;
        }
        bool found = false;
        for ( quint64 i = 0; i < blockFile.size() / sizeof ( quint64 ); i++ )
        {
            quint64 value = blockFileMap[i];
            if ( xBegin <= value && value <= xEnd )
            {
                found = true;
                break;
            }
        }
        blockFile.unmap ( ( uchar* ) blockFileMap );
        blockFile.close();
        return found;
    }
    else if ( height == 1 ) // directory level above block files
    {
        // get directory listing
        QDir currentDir ( path );
        currentDir.setNameFilters( QStringList( QString( "*.block" ) ) );
        QStringList blockFileList = currentDir.entryList ( QDir::NoDotAndDotDot | QDir::Files, QDir::Name );

        // still in tree => recursive descent

        for ( int i = 0; i < blockFileList.size(); i++ )
        {
            QString coreName = blockFileList[i];
            coreName.chop( 6 );

            quint64 sliceValueBegin = dirName2sliceMin( coreName, height-1, pathValue );
            quint64 sliceValueEnd = dirName2sliceMax( coreName, height-1, pathValue );

            if ( xEnd < sliceValueBegin )
            {
                break;
            }
            if ( xBegin <= sliceValueEnd && hasEvent ( xBegin, xEnd, path + "/" + blockFileList[i], sliceValueBegin, height - 1 ) )
            {
                return true;
            }
        }
    }
    else // directories above
    {
        // get directory listing
        QDir currentDir ( path );
        QStringList subDirList = currentDir.entryList ( QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name );

        // error checking
        if ( subDirList.isEmpty() && height != TOTALHEIGHT )
        {
            QMessageBox myMessBox;
            myMessBox.setText ( "Error: Malformed directory tree." );
            myMessBox.exec();
            return false;
        }

        // still in tree => recursive descent

        for ( int i = 0; i < subDirList.size(); i++ )
        {
            quint64 sliceValueBegin = dirName2sliceMin( subDirList[i], height-1, pathValue );
            quint64 sliceValueEnd = dirName2sliceMax( subDirList[i], height-1, pathValue );

            if ( xEnd < sliceValueBegin )
            {
                break;
            }
            if ( xBegin <= sliceValueEnd && hasEvent ( xBegin, xEnd, path + "/" + subDirList[i], sliceValueBegin, height - 1 ) )
            {
                return true;
            }
        }
    }
    return false;
}

void EventEditor::addEventManual()
{
    addEvent ( ui.manualPosLine->text().toULongLong() );
}

void EventEditor::validateManualPosLine ( QString newValue )
{
    // Try to convert it to quint64
    bool success;
    quint64 value = newValue.toULongLong ( &success );
    if ( success )
    {
        manualPosValue = newValue;
    }
    else
    {
        ui.manualPosLine->setText ( manualPosValue );
    }
}

bool EventEditor::hasNextEvent ( quint64 event )
{
    quint64 nextEvent;
    if ( searchNextEvent ( true, event, &nextEvent, treeRootDirName, TOTALHEIGHT ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool EventEditor::hasPrevEvent ( quint64 event )
{
    quint64 prevEvent;
    if ( searchPrevEvent ( true, event, &prevEvent, treeRootDirName, TOTALHEIGHT ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

quint64 EventEditor::nextEvent ( quint64 event ) throw ( bool )
{
    quint64 nextEvent;
    if ( searchNextEvent ( true, event, &nextEvent, treeRootDirName, TOTALHEIGHT ) )
    {
        return nextEvent;
    }
    else
    {
        throw false;
    }
}

quint64 EventEditor::prevEvent ( quint64 event ) throw ( bool )
{
    quint64 prevEvent;
    if ( searchPrevEvent ( true, event, &prevEvent, treeRootDirName, TOTALHEIGHT ) )
    {
        return prevEvent;
    }
    else
    {
        throw false;
    }
}


void EventEditor::nextEventSlot()
{
    try
    {
        currentEvent = nextEvent( currentEvent );
        updateGUI();
        updateCursor();
        emit triggerRepaint ( this );
    }
    catch ( bool err )
    {
        qDebug() << "Cannot switch to next event.";
    }

}

void EventEditor::prevEventSlot()
{
    try
    {
        currentEvent = prevEvent( currentEvent );
        updateGUI();
        updateCursor();
        emit triggerRepaint ( this );
    }
    catch ( bool err )
    {
        qDebug() << "Cannot switch to prev event.";
    }

}

void EventEditor::toggleTrackBox ( int state )
{
    switch ( state )
    {
    case Qt::Unchecked:
        // do nothing
        break;
    case Qt::Checked:
        updateCursor();
        break;
    default:
        qDebug() << "void EventEditor::toggleTrackBox ( int state ): shouldn't reach default case.";
    }
}


bool EventEditor::searchNextEvent ( bool firstDescent, quint64 event, quint64* nextEvent, QString path, int height )
{
    if ( firstDescent )
    {
        if ( height == 0 )
        {
            // blocklevel
            QFile blockFile ( path );
            if ( !blockFile.open ( QIODevice::ReadOnly ) )
            {
                return false;
            }
            // map file
            quint64* blockData = ( quint64* ) blockFile.map ( 0, blockFile.size() );
            if ( blockData == NULL )
            {
                qDebug() << "Cannot map" << path;
                return false;
            }

            // Search event and then the next element
            bool foundNewEvent = false;
            for ( int i = 0; i < blockFile.size() / sizeof ( quint64 ); i++ )
            {
                quint64 value = blockData[i];
                if ( value == event )
                {
                    if ( i+1 < blockFile.size() / sizeof ( quint64 ) )
                    {
                        *nextEvent = blockData[i+1];
                        foundNewEvent = true;
                        break;
                    }
                    else
                    {
                        foundNewEvent = false;
                        break;
                    }
                }
                if ( value >= event )
                {
                    *nextEvent = value;
                    foundNewEvent = true;
                    break;
                }
            }

            // unmap and close file
            blockFile.unmap ( ( uchar* ) blockData );
            blockFile.close();
            return foundNewEvent;
        }
        else if ( height == 1 ) // directory level above block files
        {
            QString blockFileName = generatePathComponents( event )[TOTALHEIGHT-height] + ".block";
            if ( searchNextEvent ( true, event, nextEvent, path + "/" + blockFileName, height-1 ) )
            {
                return true;
            }
            else
            {
                // need to search in another branch
                QDir currentDir ( path );
                currentDir.setNameFilters( QStringList( QString( "*.block" ) ) );
                QStringList blockFileList;
                blockFileList = currentDir.entryList ( QDir::Files | QDir::NoDotAndDotDot, QDir::Name );

                for ( int i = 0; i < blockFileList.size(); i++ )
                {
                    if ( blockFileName < blockFileList[i] )
                    {
                        return searchNextEvent ( false, event, nextEvent, path + "/" + blockFileList[i], height-1 );
                    }
                }
                return false;
            }
        }
        else // directory levels higher up
        {
            QString subDirName = generatePathComponents( event )[TOTALHEIGHT-height];

            if ( searchNextEvent ( true, event, nextEvent, path + "/" + subDirName, height-1 ) )
            {
                return true;
            }
            else
            {
                // need to search in another branch
                QDir currentDir ( path );
                QStringList subDirList;
                subDirList = currentDir.entryList ( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );

                for ( int i = 0; i < subDirList.size(); i++ )
                {
                    if ( subDirName < subDirList[i] )
                    {
                        return searchNextEvent ( false, event, nextEvent, path + "/" + subDirList[i], height-1 );
                    }
                }
                return false;
            }
        }
    }
    else // second descent
    {
        if ( height == 0 ) // File level
        {
            // blocklevel
            QFile blockFile ( path );
            if ( !blockFile.open ( QIODevice::ReadOnly ) )
            {
                QMessageBox myMessBox;
                myMessBox.setText ( "Cannot open " +  blockFile.fileName() + ": " + blockFile.errorString() );
                myMessBox.exec();
                return false;
            }
            if ( blockFile.read ( ( char* ) nextEvent, sizeof ( quint64 ) ) != sizeof ( quint64 ) )
            {
                QMessageBox myMessBox;
                myMessBox.setText ( "Cannot open read from " +  blockFile.fileName() + ": " + blockFile.errorString() );
                myMessBox.exec();
                return false;
            }
            blockFile.close();
            return true;
        }
        else if ( height == 1 ) // last dir level
        {
            QDir currentDir ( path );
            currentDir.setNameFilters( QStringList( QString( "*.block" ) ) );
            QStringList blockFileList = currentDir.entryList ( QDir::Files | QDir::NoDotAndDotDot, QDir::Name );
            return searchNextEvent ( false, event, nextEvent, path + "/" + blockFileList.first(), height-1 );
        }
        else // higher dir levels
        {
            QDir currentDir ( path );
            QStringList subDirList = currentDir.entryList ( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
            return searchNextEvent ( false, event, nextEvent, path + "/" + subDirList.first(), height-1 );
        }
    }
}


bool EventEditor::searchPrevEvent ( bool firstDescent, quint64 event, quint64* prevEvent, QString path, int height )
{
    if ( firstDescent )
    {
        if ( height == 0 ) // blocklevel
        {
            // blocklevel
            QFile blockFile ( path );
            if ( !blockFile.open ( QIODevice::ReadOnly ) )
            {
                return false;
            }
            // map file
            quint64* blockData = ( quint64* ) blockFile.map ( 0, blockFile.size() );
            if ( blockData == NULL )
            {
                qDebug() << "Cannot map" << path;
                return false;
            }

            // Search event and then the next element
            bool foundNewEvent = false;
            for ( quint64 i = ( blockFile.size() / sizeof ( quint64 ) ) - 1; i >= 0; i-- )
            {
                quint64 value = blockData[i];
                if ( value == event )
                {
                    if ( i != 0 )
                    {
                        *prevEvent = blockData[i-1];
                        foundNewEvent = true;
                    }
                    else
                    {
                        foundNewEvent = false;
                    }
                    break;
                }
                if ( value <= event )
                {
                    *prevEvent = value;
                    foundNewEvent = true;
                    break;
                }
            }

            // unmap and close file
            blockFile.unmap ( ( uchar* ) blockData );
            blockFile.close();
            return foundNewEvent;
        }
        else if ( height == 1 ) // last dir level
        {
            QString blockFileName = generatePathComponents( event )[TOTALHEIGHT-height] + ".block";
            if ( searchPrevEvent ( true, event, prevEvent, path + "/" + blockFileName, height-1 ) )
            {
                return true;
            }
            else
            {
                // need to search in another branch
                QDir currentDir ( path );

                QStringList blockFileList;
                blockFileList = currentDir.entryList ( QDir::Files | QDir::NoDotAndDotDot, QDir::Name );

                for ( int i = blockFileList.size() - 1; i >= 0; i-- )
                {
                    if ( blockFileName > blockFileList[i] )
                    {
                        return searchPrevEvent ( false, event, prevEvent, path + "/" + blockFileList[i], height-1 );
                    }
                }
                return false;
            }
        }
        else // higher directories
        {
            QString subDirName = generatePathComponents( event )[TOTALHEIGHT-height];
            if ( searchPrevEvent ( true, event, prevEvent, path + "/" + subDirName, height-1 ) )
            {
                return true;
            }
            else
            {
                // need to search in another branch
                QDir currentDir ( path );

                QStringList subDirList;
                subDirList = currentDir.entryList ( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );


                for ( int i = subDirList.size() - 1; i >= 0; i-- )
                {
                    if ( subDirName > subDirList[i] )
                    {
                        return searchPrevEvent ( false, event, prevEvent, path + "/" + subDirList[i], height-1 );
                    }
                }
                return false;
            }
        }
    }
    else // second descent
    {
        if ( height == 0 ) // blockFile level
        {
            QFile blockFile ( path );
            if ( !blockFile.open ( QIODevice::ReadOnly ) )
            {
                QMessageBox myMessBox;
                myMessBox.setText ( "Cannot open " +  blockFile.fileName() + ": " + blockFile.errorString() );
                myMessBox.exec();
                return false;
            }
            if ( !blockFile.seek ( blockFile.size() - sizeof ( quint64 ) ) )
            {
                QMessageBox myMessBox;
                myMessBox.setText ( "Cannot seek to end of " +  blockFile.fileName() + ": " + blockFile.errorString() );
                myMessBox.exec();
                return false;
            }
            if ( blockFile.read ( ( char* ) prevEvent, sizeof ( quint64 ) ) != sizeof ( quint64 ) )
            {
                QMessageBox myMessBox;
                myMessBox.setText ( "Cannot open read from " +  blockFile.fileName() + ": " + blockFile.errorString() );
                myMessBox.exec();
                return false;
            }
            blockFile.close();
            return true;
        }
        else if ( height == 1 ) // last dir level
        {
            QDir currentDir ( path );
            currentDir.setNameFilters( QStringList( QString( "*.block" ) ) );
            QStringList blockFileName = currentDir.entryList ( QDir::Files | QDir::NoDotAndDotDot, QDir::Name );
            return searchPrevEvent ( false, event, prevEvent, path + "/" + blockFileName.last(), height-1 );
        }
        else // higher dir levels
        {
            QDir currentDir ( path );
            QStringList subDirList = currentDir.entryList ( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
            return searchPrevEvent ( false, event, prevEvent, path + "/" + subDirList.last(), height-1 );
        }
    }
}

void EventEditor::deleteEvent()
{
    quint64 event = currentEvent;
    delEventInRange ( event, event );
}

void EventEditor::plusEvent()
{
    quint64 newCurrentEvent = currentEvent+1;
    delEventInRange ( currentEvent, currentEvent );
    //eel->eventLoopAlive();

    addEvent ( newCurrentEvent );

    currentEvent = newCurrentEvent;
    updateGUI();
    updateCursor();
    emit ( triggerRepaint ( this ) );
}

void EventEditor::minusEvent()
{
    quint64 newCurrentEvent = currentEvent-1;
    delEventInRange ( currentEvent, currentEvent );
    //eel->eventLoopAlive();

    addEvent ( newCurrentEvent );

    currentEvent = newCurrentEvent;
    updateGUI();
    updateCursor();
    emit ( triggerRepaint ( this ) );
}

void EventEditor::handleFineTune()
{
    QString text = ui.fineEdit->text();
    // an empty string is ok
    if ( text.isEmpty() )
        return;

    // Try to convert it to quint64
    bool success;
    quint64 value = text.toULongLong ( &success );
    if ( success )
    {
        fineEditValue = text;
        delEvent ( currentEvent );
        //eel->eventLoopAlive();
        addEvent ( value );
        currentEvent = value;
        updateGUI();
        updateCursor();
        emit triggerRepaint ( this );
    }
    else
    {
        ui.fineEdit->setText ( fineEditValue );
    }
}

void EventEditor::showGUI()
{
    gui->show();
}

void EventEditor::hideGUI()
{
    gui->hide();
}

bool EventEditor::hasGUI()
{
    return true;
}

bool EventEditor::GUIvisible()
{
    return gui->isVisible();
}

void EventEditor::relayGUIupdate()
{
    emit GUIshowhide();
}

qint64 EventEditor::getXmin()
{
    quint64 event;
    if ( findFirstEvent ( &event, treeRootDirName, TOTALHEIGHT ) )
    {
        return event;
    }
    else
    {
        return FTSPlot::GL_Layer::getXmin();
    }
}

qint64 EventEditor::getXmax()
{
    quint64 event;
    if ( findLastEvent ( &event, treeRootDirName, TOTALHEIGHT ) )
    {
        return event;
    }
    else
    {
        return FTSPlot::GL_Layer::getXmax();
    }
}

void EventEditor::partialCacheRebuild( QStringList pathToBlockFile )
{
    for ( int height = 1; height <= TOTALHEIGHT; height++ )
    {
        QString path = treeRootDirName;
        for ( int j = 0; j < TOTALHEIGHT - height; j++ )
        {
            path += "/" + pathToBlockFile[j];
        }

        if ( height == 1 )
        {
            // get file list
            QDir currentDir ( path );
            currentDir.setNameFilters( QStringList( QString ( "*.block" ) ) );
            QStringList fileList = currentDir.entryList ( QDir::NoDotAndDotDot | QDir::Files, QDir::Name );
            if ( fileList.isEmpty() )
            {
                continue;
            }
            QFile blockFile;
            QVector<quint64> vec;
            foreach ( QString blockFileName, fileList )
            {
                blockFile.setFileName ( path + "/" + blockFileName );
                if ( blockFile.open ( QIODevice::ReadOnly ) )
                {
                    quint64* blockData = ( quint64* ) blockFile.map ( 0, blockFile.size() );
                    if ( blockData != NULL )
                    {
                        quint64 mask = nodeMask( height );

                        for ( int j = 0; j < blockFile.size() / sizeof ( quint64 ); j++ )
                        {
                            quint64 maskedValue = blockData[j] & mask;
                            if ( vec.isEmpty() || maskedValue != vec.last() )
                            {
                                vec.append( maskedValue );
                            }
                        }
                    }
                    blockFile.close();
                }
            }

            QFile::remove( path + ".node" );

            if ( !vec.isEmpty() )
            {
                // Write out new cache file
                QFile nodeFile ( path + ".node" );
                if ( nodeFile.open ( QIODevice::WriteOnly ) )
                {
                    nodeFile.write ( ( char* ) vec.data(), vec.size() * sizeof ( quint64 ) );
                    nodeFile.close();
                }
            }
        }
        else
        {
            // get dir list
            QDir currentDir ( path );
            currentDir.setNameFilters( QStringList( QString( "*.node" ) ) );
            QStringList nodeFileList = currentDir.entryList ( QDir::NoDotAndDotDot | QDir::Files, QDir::Name );
            if ( nodeFileList.isEmpty() )
            {
                continue;
            }
            QVector<quint64> vec;
            QFile nodeFile;
            for ( int i = 0; i < nodeFileList.size(); i++ )
            {
                nodeFile.setFileName( path + "/" + nodeFileList[i] );

                if ( nodeFile.open ( QIODevice::ReadOnly ) )
                {
                    quint64* nodeData = ( quint64* ) nodeFile.map ( 0, nodeFile.size() );
                    if ( nodeData != NULL )
                    {
                        quint64 mask = nodeMask( height );

                        for ( int j = 0; j < nodeFile.size() / sizeof ( quint64 ); j++ )
                        {
                            quint64 maskedValue = nodeData[j] & mask;
                            if ( vec.isEmpty() || maskedValue != vec.last() )
                            {
                                vec.append( maskedValue );
                            }
                        }

                        nodeFile.unmap ( ( uchar* ) nodeData );
                    }
                    nodeFile.close();
                }
            }

            QFile::remove( path + ".node" );

            if ( !vec.isEmpty() )
            {
                // Write out new cache file
                QFile nodeFile ( path + ".node" );
                if ( nodeFile.open ( QIODevice::WriteOnly ) )
                {
                    nodeFile.write ( ( char* ) vec.data(), vec.size() * sizeof ( quint64 ) );
                    nodeFile.close();
                }
            }
        }
    }
}

inline quint64 EventEditor::value2BlockBegin(quint64 value, int height)
{
    quint64 mask = height2mask( height );
    return value & mask;
}

inline quint64 EventEditor::value2BlockEnd(quint64 value, int height)
{
    quint64 mask = ~height2mask( height );
    return value | mask; ;
}

// kate: indent-mode cstyle; space-indent on; indent-width 0; 
