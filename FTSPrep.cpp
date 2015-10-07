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


#include <QMutexLocker>
#include <QDebug>
#include <QtGlobal>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "FTSPrep.h"

using namespace FTSPlot;

FTSPrep::FTSPrep()
{
    state = ready;
    granularity = 2048;
}

FTSPrep::~FTSPrep()
{}

void FTSPrep::setProgressGranularity(quint64 gran )
{
    granularity = gran;
}


bool FTSPrep::setNewFile ( QString fileName, quint64 thinfactor, quint64 FileSizeLimit )
{
    QMutexLocker myLocker ( &threadlock );
    if ( state == busy )
    {
        return false;
    }
    this->inFileName = fileName;
    this->thinFactor = thinfactor;
    this->FileSizeLimit = FileSizeLimit;
    resume = false;
    return true;
}

bool FTSPrep::setresumeFile ( QString fileName, quint64 thinfactor, quint64 FileSizeLimit,
                             quint64 level, quint64 position )
{
    QMutexLocker myLocker ( &threadlock );
    if ( state == busy )
    {
        return false;
    }
    this->inFileName = fileName;
    this->thinFactor = thinfactor;
    this->FileSizeLimit = FileSizeLimit;
    resume = true;
    this->level = level;
    this->position = position;
    return true;
}

ResumeData FTSPrep::getResumeData()
{
    ResumeData value;
    value.ResumethinFactor = ResumethinFactor;
    value.ResumePosition = ResumePosition;
    return value;
}


void FTSPrep::stopforResume()
{
    abort = true;
}

void FTSPrep::run()
{
    threadlock.lock();
    state = busy;
    threadlock.unlock();

    QString oldfileName;
    QString thinFileName;
    oldfileName = inFileName;
    bool rawFile;
    quint64 idx;
    quint64 lidx;
    quint64 idxlimit;
    quint64 currThinFactor;
    quint64 seekOffset;
    quint64 oldfileLength;
    double* origMap;
    double locmin;
    double locmax;
    quint64 fileProgress;
    int oldfileProgress = -1;
    quint64 tmpcounter = 0;
    quint64 origIdxLength;
    quint64 totalIdxLength;
    quint64 totalidx = 0;
    quint64 totalProgress;


    // setup parameters
    abort = false;
    if ( resume )
    {
        // generate correct level
        currThinFactor = level / thinFactor;
        // level - 1 and then check if it's already the raw file
        if ( currThinFactor == 1 )
        {
            rawFile = true;
            totalidx = position;
        }
        else
        {
            rawFile = false;
            oldfileName.append( "." );
            oldfileName.append( QString::number( currThinFactor ) );
        }

        // set position
        totalidx = 0;
        if ( rawFile )
        {
            totalidx = position;
        }
        else
        {
            // open original file
            QFile origFile( inFileName );
            if ( !origFile.open ( QIODevice::ReadOnly ) )
            {
                emit QDTexception( QString ("Cannot open file ") +
                                   inFileName + QString( "." ) );
                return;
            }

            // get file size and translate to idx position
            quint64 totalLevelIdx = origFile.size() / sizeof(double);
            origFile.close();
            totalidx = totalLevelIdx;

            // while (...) reduce idx length and add
            quint64 tmplevel = 1;
            while ( (tmplevel *= thinFactor) < level / thinFactor )
            {
                totalLevelIdx /= thinFactor;
                totalidx += totalLevelIdx;
            }

            // in the end: add position
            totalidx += position;
        }
        idx = position;
        lidx = position % thinFactor;
        // should be 0
        Q_ASSERT( lidx == 0 );
        if ( rawFile )
        {
            seekOffset = idx / thinFactor * 8 * 2;
        }
        else
        {
            seekOffset = idx / thinFactor * 8;
        }
    }
    else
    {
        // just take the given filename
        oldfileName = inFileName;
        // set level to raw
        rawFile = true;
        currThinFactor = 1;
        // set position to zero
        idx = 0;
        lidx = 0;
        seekOffset = 0;
    }


    QFile origFile;
    QFile thinFile;
    // open original file to get file length
    origFile.setFileName( inFileName );
    if ( !origFile.open ( QIODevice::ReadOnly ) )
    {
        emit QDTexception( QString ("Cannot open file ") +
                           inFileName + QString( "." ) );
        return;
    }
    origIdxLength = origFile.size() / sizeof(double);
    origFile.close();
    totalIdxLength = origIdxLength;

    int i = 0;
    while ( origIdxLength * sizeof(double) > FileSizeLimit )
    {
        origIdxLength /= thinFactor;
        totalIdxLength += origIdxLength;
    }
    // open old file to get file length
    origFile.setFileName( oldfileName );
    if ( !origFile.open ( QIODevice::ReadOnly ) )
    {
        emit QDTexception( QString ("Cannot open file ") +
                           oldfileName + QString( "." ) );
        return;
    }
    oldfileLength = origFile.size();
    origFile.close();

    while ( oldfileLength > FileSizeLimit )
    {
        // mmap former file
        origFile.setFileName ( oldfileName );
        if ( !origFile.open ( QIODevice::ReadOnly ) )
        {
            emit QDTexception( QString ("Cannot open file ") +
                               oldfileName + QString( "." ) );
            return;
        }
        origMap = ( double* ) origFile.map ( 0, origFile.size(), QFile::NoOptions );
        if ( origMap == NULL )
        {
            emit QDTexception( QString( "Cannot map file ") +
                               oldfileName + QString( "." ) );
            return;
        }
        // open new file with .currentthinFactor suffix
        currThinFactor *= thinFactor;
        thinFileName.clear();
        thinFileName.append ( inFileName );
        thinFileName.append ( "." );
        thinFileName.append ( QString::number ( currThinFactor ) );
        thinFile.setFileName ( thinFileName );
        if ( !thinFile.open ( QIODevice::ReadWrite )  )
        {
            emit QDTexception( QString ("Cannot open file ") +
                               thinFileName + QString( "." ) );
            return;
        }
        // Seek to correct position
        if (!thinFile.seek( seekOffset ))
        {
            emit QDTexception(
                QString( "Cannot seek in file ") +
                thinFileName + QString( "." ) );
            return;
        }
        // Set seekOffset to 0 after the first time
        seekOffset = 0;

        // traverse former file, thin, write new file
        locmin = INFINITY;
        locmax = -INFINITY;
        idxlimit = ( quint64 ) origFile.size() / sizeof ( double );

        do
        {
            if ( lidx == thinFactor )
            {
                lidx = 0;
                // write out min and max
                qint64 result;
                result = thinFile.write ( ( const char* ) &locmin, sizeof ( double ) );
                if ( result != sizeof ( double ) )
                {
                    emit QDTexception(
                        QString ("Cannot write to file ") +
                        thinFileName + QString( "." ) );
                    return;
                }
                result = thinFile.write ( ( const char* ) &locmax, sizeof ( double ) );
                if ( result != sizeof ( double ) )
                {
                    emit QDTexception(
                        QString ("Cannot write to file ") +
                        thinFileName + QString( "." ) );
                    return;
                }
                // check for abort
                ResumethinFactor = currThinFactor;
                ResumePosition = idx;
                if ( abort )
                {
                    // set resume data
                    // close files
                    thinFile.close();
                    origFile.close();
                    // set right state
                    state = ready;
                    // return
                    return;
                }

                // emit progress signal
                // calculate new progress value
                fileProgress = idx * granularity / idxlimit;
                totalProgress = totalidx * granularity / totalIdxLength;
                // compare with old, if different => send

                if ( fileProgress != oldfileProgress )
                {
                    emit progressUpdate( totalProgress,
                                         currThinFactor, fileProgress );
                    oldfileProgress = fileProgress;
                }

                locmin = INFINITY;
                locmax = -INFINITY;
            }
            if ( rawFile )
            {
                if ( locmin > origMap[idx] )
                    locmin = origMap[idx];
                if ( locmax < origMap[idx] )
                    locmax = origMap[idx];
            }
            else
            {
                if ( locmin > origMap[idx] )
                    locmin = origMap[idx];
                if ( locmax < origMap[idx+1] )
                    locmax = origMap[idx+1];
                idx++;
            }
            idx++;
            lidx++;
            totalidx++;
        }
        while ( idx < idxlimit );

        // Write out the rest
        if ( lidx != 1 )
        {
            qint64 result;
            result = thinFile.write ( ( const char* ) &locmin, sizeof ( double ) );
            if ( result != sizeof ( double ) )
            {
                emit QDTexception(
                    QString ("Cannot write to file ") +
                    thinFileName + QString( "." ) );
                return;
            }
            result = thinFile.write ( ( const char* ) &locmax, sizeof ( double ) );
            if ( result != sizeof ( double ) )
            {
                emit QDTexception(
                    QString ("Cannot write to file ") +
                    thinFileName + QString( "." ) );
                return;
            }
        }

        // unmap and close all files
        oldfileLength = thinFile.size();
        oldfileName = thinFileName;
        rawFile = false;
        thinFile.close();
        origFile.unmap ( ( uchar* ) origMap );
        origFile.close();

        // reset indexes
        idx = 0;
        lidx = 0;
    }


    // write config file
    // raw
    QString configfileName = inFileName;
    configfileName.append( ".cfg" );
    QFile configfile( configfileName );
    if ( !configfile.open ( QIODevice::WriteOnly)  )
    {
        emit QDTexception( QString ("Cannot open file ") +
                           configfileName + QString( "." ) );
        return;
    }
    QFileInfo fInfo( inFileName );
    QString configFileText;
    configFileText.append( "raw " );
    configFileText.append( fInfo.fileName() );
    configFileText.append( "\n" );
    for ( quint64 i = thinFactor; i <= currThinFactor; i *= thinFactor )
    {
        configFileText.append( QString::number(i) );
        configFileText.append( " " );
        configFileText.append( fInfo.fileName() + QString (".")
                               + QString::number(i) );
        configFileText.append( "\n" );
    }
    QTextStream configStream( &configfile );
    configStream << configFileText;

    threadlock.lock();
    state = ready;
    threadlock.unlock();
}

