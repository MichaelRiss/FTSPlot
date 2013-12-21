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


#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include "O1PrepGUI.h"

using namespace O1Plot;

O1PrepGUI::O1PrepGUI()
{
    ui.setupUi(this);
    state = dth_missing;
    connect( ui.AddFileButton, SIGNAL(pressed()), this, SLOT(addFileHandler()) );
    connect( ui.RemoveFileButton, SIGNAL(pressed()), this, SLOT(RemoveFileHandler()) );
    connect( ui.StartStopButton, SIGNAL(pressed()), this, SLOT(StartStopHandlder()) );
    connect( ui.ResumeButton, SIGNAL(pressed()), this, SLOT(ResumeHandler()) );
    updateGUI();
}

void O1PrepGUI::setDataProcessor(O1Prep* dth)
{
    this->dth = dth;
    state = idle_noFile;
    updateGUI();
}


void O1PrepGUI::addFileHandler()
{
    // Spawn filedialog
    QStringList tmp = QFileDialog::getOpenFileNames( this,
                      tr("Open Resume File"), "", tr("Binary Files (*.bin);;All Files (*)"));
    for ( int i = 0; i < tmp.size(); i++ )
    {
        ui.listWidget->addItem(tmp.at(i));
    }
    // change state
    state = idle_File;
    updateGUI();
}

void O1PrepGUI::RemoveFileHandler()
{
    // Remove current item in listWidget
    QListWidgetItem* tmp = ui.listWidget->takeItem(ui.listWidget->currentRow());
    delete( tmp );
    if ( ui.listWidget->count() == 0 )
    {
        state = idle_noFile;
        updateGUI();
    }
}

void O1PrepGUI::StartStopHandlder()
{
    QMessageBox msgBox;
    QFile tmpFile;
    quint64 divideFactor;
    switch ( state )
    {
    case idle_File:
        // Set up total job file length, index, processed bytes
        abortion = false;
        totalFileIdx = 0;
        FileIdxAcc = 0;
        fileIdxs.clear();
        fileFactors.clear();
        for ( int i = 0; i < ui.listWidget->count(); i++ )
        {
            tmpFile.setFileName(ui.listWidget->item(i)->text());
            if ( !tmpFile.open(QIODevice::ReadOnly) )
            {
                msgBox.setText( QString( "Error: Cannot open file " ) +
                                ui.listWidget->item(i)->text() + QString( "!") );
                msgBox.exec();
            }
            quint64 tmpIdx = tmpFile.size();
            tmpFile.close();
            fileIdxs.append( tmpIdx );
            totalFileIdx += tmpIdx;
        }
        for ( int i = 0; i < ui.listWidget->count(); i++ )
        {
            fileFactors.append( (double) fileIdxs[i] / (double) totalFileIdx );
        }

        jobFileIdx = 0;
        if ( dth->setNewFile( ui.listWidget->item(jobFileIdx)->text(),
                              ui.ThinFactorBox->value(), FILESIZELIMIT ) )
        {
            // Set up total job file length, index, processed bytes
            // setup finshed() handler
            connect( this, SIGNAL(abortThread()), dth, SLOT(stopforResume()));
            connect( dth, SIGNAL(finished()), this, SLOT(handleThreadFinish()) );
            dth->setProgressGranularity( 2048 );
            ui.totalBar->setMinimum( 0 );
            ui.totalBar->setMaximum( 2048 );
            ui.fileLabel->setText( QString( "File: " ) + ui.listWidget->item(jobFileIdx)->text() );
            ui.FileBar->setMaximum( 0 );
            ui.FileBar->setMaximum( 2048 );
            ui.reductionBar->setMinimum( 0 );
            ui.reductionBar->setMaximum( 2048 );
            connect( dth, SIGNAL(progressUpdate(int,int,int)),
                     this, SLOT(handleProgressUpdate(int,int,int)) );
            // start first job
            dth->start();
            state = Running;
            updateGUI();
        }
        else
        {
            msgBox.setText("Error: O1Prep object is not ready!");
            msgBox.exec();
        }

        break;

    case Running:
        // send a stop signal
        // set a flag so that the handleThreadFinish routine knows that it has to
        // check for the resume information and write the resume file
        abortion = true;
        emit abortThread();

        break;

    default:
        qDebug("O1PrepGUI::StartStopHandlder(): invalid state!");
        break;
    }
}

void O1PrepGUI::ResumeHandler()
{
    // FileDialog for ResumeFileButton
    QString resumeFileName = QFileDialog::getOpenFileName( this,
                             tr("Open Resume File"), "", tr("Resume Files (*.resume)"));
    QFile resumeFile( resumeFileName );
    if ( !resumeFile.open( QIODevice::ReadOnly ) )
    {
        QMessageBox msgBox;
        msgBox.setText( QString( "Cannot open resume file " ) + resumeFileName );
        msgBox.exec();
        return;
    }
    QTextStream resumeText( &resumeFile );
    // read current file, enter into listWidget, save level, position
    QString thinFactorString;
    QString fileName;
    QString levelString;
    QString positionString;
    resumeText >> thinFactorString;
    resumeText >> fileName >> levelString >> positionString;
    quint64 level = levelString.toULongLong();
    quint64 position = positionString.toULongLong();
    ui.ThinFactorBox->setValue( thinFactorString.toULongLong() );
    ui.listWidget->clear();
    ui.listWidget->addItem( fileName );
    // read rest of file listWidget
    while ( !resumeText.atEnd() )
    {
        resumeText >> fileName;
        if ( !fileName.isEmpty() )
        {
            ui.listWidget->addItem( fileName );
        }
    }
    resumeFile.close();
    
    // Set up total job file length, index, processed bytes
    abortion = false;
    totalFileIdx = 0;
    FileIdxAcc = 0;
    fileIdxs.clear();
    fileFactors.clear();
    QFile tmpFile;
    for ( int i = 0; i < ui.listWidget->count(); i++ )
    {
        tmpFile.setFileName(ui.listWidget->item(i)->text());
        if ( !tmpFile.open(QIODevice::ReadOnly) )
        {
            QMessageBox msgBox;
            msgBox.setText( QString( "ResumeHandler error: Cannot open file " ) +
                            ui.listWidget->item(i)->text() + QString( "!") );
            msgBox.exec();
            return;
        }
        quint64 tmpIdx = tmpFile.size();
        tmpFile.close();
        fileIdxs.append( tmpIdx );
        totalFileIdx += tmpIdx;
    }
    for ( int i = 0; i < ui.listWidget->count(); i++ )
    {
        fileFactors.append( (double) fileIdxs[i] / (double) totalFileIdx );
    }

    
    // call setupresumeFile
    jobFileIdx = 0;
    if ( !dth->setresumeFile( ui.listWidget->item(0)->text(), ui.ThinFactorBox->value(),
                              FILESIZELIMIT, level, position ) )
    {
        QMessageBox msgBox;
        msgBox.setText( "O1Prep is not ready." );
        msgBox.exec();
        return;
    }
    // connect signals
    connect( this, SIGNAL(abortThread()), dth, SLOT(stopforResume()));
    connect( dth, SIGNAL(finished()), this, SLOT(handleThreadFinish()) );
    dth->setProgressGranularity( 2048 );
    ui.totalBar->setMinimum( 0 );
    ui.totalBar->setMaximum( 2048 );
    ui.fileLabel->setText( QString( "File: " ) + ui.listWidget->item(jobFileIdx)->text() );
    ui.FileBar->setMaximum( 0 );
    ui.FileBar->setMaximum( 2048 );
    ui.reductionBar->setMinimum( 0 );
    ui.reductionBar->setMaximum( 2048 );
    connect( dth, SIGNAL(progressUpdate(int,int,int)),
             this, SLOT(handleProgressUpdate(int,int,int)) );
    
    // start first job
    dth->start();
    state = Running;
    updateGUI();

}


void O1PrepGUI::handleThreadFinish()
{
    QMessageBox msgBox;
    //   - cleanup old job
    if ( abortion )
    {
        // ask if we should resume
        msgBox.setText("Data Thinning aborted.");
        msgBox.setInformativeText("Do you want to save the progress into a resume file?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No );
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        
        if( ret == QMessageBox::Yes )
        {
            QFile ResumeFile;
            bool FileTestResult;
            do {
                // get filename
                QString tmp = QFileDialog::getSaveFileName( this,
                      tr("Save Resume File"), "", tr("Resume Files (*.resume);;All Files (*)"));
                      
                // add resume suffix if not existing
                if( !tmp.endsWith( ".resume", Qt::CaseInsensitive ))
                {
                    tmp.append( ".resume" );
                }
            
                // check if file can be created
                ResumeFile.setFileName( tmp );
                if( FileTestResult = !ResumeFile.open(QIODevice::WriteOnly) )
                {
                    msgBox.setText( QString( "Error: Cannot open ResumeFile for writing " ) + tmp );
                    msgBox.exec();
                }
            }
            while( FileTestResult );
            
            // save resume information
            ResumeData resData = dth->getResumeData();
            QString resumeText;
            resumeText.append( ui.ThinFactorBox->text() );
            resumeText.append( "\n" );
            resumeText.append( ui.listWidget->item(jobFileIdx)->text() );
            resumeText.append( " " );
            resumeText.append( QString::number( resData.ResumethinFactor ) );
            resumeText.append( " " );
            resumeText.append( QString::number( resData.ResumePosition ) );
            resumeText.append( "\n" );
            for ( int i = jobFileIdx+1; i < ui.listWidget->count(); i++ )
            {
                resumeText.append( ui.listWidget->item(i)->text() );
                resumeText.append( "\n" );
            }
            QTextStream resumeStream( &ResumeFile );
            resumeStream << resumeText;
            ResumeFile.close();
            
        }
        
        
        //   - if finished disconnect signals, change state, updateGUI()
        disconnect(dth, SIGNAL(progressUpdate(int,int,int)),
                   this, SLOT(handleProgressUpdate(int,int,int)));
        disconnect(dth, SIGNAL(finished()), this, SLOT(handleThreadFinish()));
        ui.listWidget->clear();
        ui.fileLabel->setText( "File:" );
        ui.reductionLevelLabel->setText( "ReductionLevel:" );
        state = idle_noFile;
        updateGUI();
        // quit
        return;
    }
    //   - update variables
    FileIdxAcc += 2048 * fileFactors[jobFileIdx];
    if ( ++jobFileIdx < ui.listWidget->count() )
    {
        //   - if not finished start next job
        if ( dth->setNewFile( ui.listWidget->item(jobFileIdx)->text(),
                              ui.ThinFactorBox->value(), FILESIZELIMIT ) )
        {
            ui.fileLabel->setText( QString( "File: " ) + ui.listWidget->item(jobFileIdx)->text() );
            dth->start();
        }
        else
        {
            msgBox.setText("Error: O1Prep object is not ready!");
            msgBox.exec();
        }
    }
    else
    {
        //   - if finished disconnect signals, change state, updateGUI()
        disconnect(dth, SIGNAL(progressUpdate(int,int,int)),
                   this, SLOT(handleProgressUpdate(int,int,int)));
        disconnect(dth, SIGNAL(finished()), this, SLOT(handleThreadFinish()));
        ui.fileLabel->setText( "File: " );
        ui.reductionLevelLabel->setText( "Reductionlevel: " );
        ui.listWidget->clear();
        state = idle_noFile;
        updateGUI();
    }
}

void O1PrepGUI::handleProgressUpdate(int totalProgress, int level, int fileProgress)
{
    quint64 tmp = FileIdxAcc + (quint64) ((double) totalProgress * fileFactors[jobFileIdx]);
    ui.totalBar->setValue( tmp );
    ui.FileBar->setValue( totalProgress);
    ui.reductionLevelLabel->setText( QString( "ReductionLevel: " ) + QString::number( level ) );
    ui.reductionBar->setValue( fileProgress );
}

void O1PrepGUI::updateGUI()
{
    switch ( state ) {
    case dth_missing:
        ui.ThinFactorBox->setEnabled(false);
        ui.listWidget->setEnabled(false);
        ui.AddFileButton->setEnabled(false);
        ui.RemoveFileButton->setEnabled(false);
        ui.StartStopButton->setText("Start");
        ui.StartStopButton->setEnabled(false);
        ui.ResumeButton->setEnabled(false);
        ui.totalBar->reset();
        ui.FileBar->reset();
        ui.reductionBar->reset();
        break;

    case idle_noFile:
        ui.ThinFactorBox->setEnabled(true);
        ui.listWidget->setEnabled(true);
        ui.AddFileButton->setEnabled(true);
        ui.RemoveFileButton->setEnabled(false);
        ui.StartStopButton->setText("Start");
        ui.StartStopButton->setEnabled(false);
        ui.ResumeButton->setEnabled(true);
        ui.totalBar->reset();
        ui.FileBar->reset();
        ui.reductionBar->reset();
        break;

    case idle_File:
        ui.ThinFactorBox->setEnabled(true);
        ui.listWidget->setEnabled(true);
        ui.AddFileButton->setEnabled(true);
        ui.RemoveFileButton->setEnabled(true);
        ui.StartStopButton->setText("Start");
        ui.StartStopButton->setEnabled(true);
        ui.ResumeButton->setEnabled(true);
        ui.totalBar->reset();
        ui.FileBar->reset();
        ui.reductionBar->reset();
        break;

    case Running:
        ui.ThinFactorBox->setEnabled(false);
        ui.listWidget->setEnabled(false);
        ui.AddFileButton->setEnabled(false);
        ui.RemoveFileButton->setEnabled(false);
        ui.StartStopButton->setText("Stop");
        ui.StartStopButton->setEnabled(true);
        ui.ResumeButton->setEnabled(false);
        break;

    default:
        qDebug("Invalid GUI state!");
        break;
    }
}


//#include "O1PrepGUI.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 0; 
