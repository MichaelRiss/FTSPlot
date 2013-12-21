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


#include <QDebug>
#include "ModifyColorTest.h"

void ModifyColorTest::initTestCase()
{
    tsplot = new SimpleViewWidget ( NULL );
    QVERIFY ( tsplot != NULL );
    tsplot->show();

    QDir testDir ( "testData" );
    QVERIFY ( testDir.exists() == true );

    QFile testFile ( "testData/timeSeriesDataFile.bin.cfg" );
    QVERIFY ( testFile.exists() == true );
    tsModule = tsplot->addTimeSeries ( "testData/timeSeriesDataFile.bin.cfg" );
    QVERIFY ( tsModule != NULL );

    testDir.setPath ( "testData/EventEditorTest" );
    QVERIFY ( testDir.exists() == true );
    eeModule = tsplot->addEventEditorModule ( "testData/EventEditorTest" );
    QVERIFY ( eeModule != NULL );


    testDir.setPath ( "testData/IntervalEditorTest" );
    QVERIFY ( testDir.exists() == true );
    ieModule = tsplot->addIntervalEditorModule ( "testData/IntervalEditorTest" );
    QVERIFY ( ieModule != NULL );
}

void ModifyColorTest::cleanupTestCase()
{
    bool result;
    result = tsplot->deleteModule ( ieModule );
    QVERIFY ( result == true );

    result = tsplot->deleteModule ( eeModule );
    QVERIFY ( result == true );

    result = tsplot->deleteModule ( tsModule );
    QVERIFY ( result == true );

    delete tsplot;
}

void ModifyColorTest::colorModificationTest()
{
    QColor tsColor = tsplot->getModuleColor( tsModule );
    QColor eeColor = tsplot->getModuleColor( eeModule );
    QColor ieColor = tsplot->getModuleColor( ieModule );
    
    QColor newColor;
    while( ( newColor == tsColor ) || ( newColor == eeColor ) || ( newColor == ieColor ) )
    {
      newColor.setRed( (int) ( (double) qrand() * (double) 255 / (double) RAND_MAX ) );
      newColor.setGreen( (int) ( (double) qrand() * (double) 255 / (double) RAND_MAX ) );
      newColor.setBlue( (int) ( (double) qrand() * (double) 255 / (double) RAND_MAX ) );
    }
    
    QVERIFY( tsplot->setModuleColor( tsModule, newColor ) );
    
    QVERIFY( tsplot->getModuleColor( tsModule ) == newColor );
    QVERIFY( tsplot->getModuleColor( eeModule ) == eeColor );
    QVERIFY( tsplot->getModuleColor( ieModule ) == ieColor );
    
    tsColor = newColor;
    
    while( ( newColor == tsColor ) || ( newColor == eeColor ) || ( newColor == ieColor ) )
    {
      newColor.setRed( (int) ( (double) qrand() * (double) 255 / (double) RAND_MAX ) );
      newColor.setGreen( (int) ( (double) qrand() * (double) 255 / (double) RAND_MAX ) );
      newColor.setBlue( (int) ( (double) qrand() * (double) 255 / (double) RAND_MAX ) );
    }
    
    QVERIFY( tsplot->setModuleColor( eeModule, newColor ) );
    
    QVERIFY( tsplot->getModuleColor( tsModule ) == tsColor );
    QVERIFY( tsplot->getModuleColor( eeModule ) == newColor );
    QVERIFY( tsplot->getModuleColor( ieModule ) == ieColor );
    
    eeColor = newColor;
    
    while( ( newColor == tsColor ) || ( newColor == eeColor ) || ( newColor == ieColor ) )
    {
      newColor.setRed( (int) ( (double) qrand() * (double) 255 / (double) RAND_MAX ) );
      newColor.setGreen( (int) ( (double) qrand() * (double) 255 / (double) RAND_MAX ) );
      newColor.setBlue( (int) ( (double) qrand() * (double) 255 / (double) RAND_MAX ) );
    }
    
    QVERIFY( tsplot->setModuleColor( ieModule, newColor ) );
    
    QVERIFY( tsplot->getModuleColor( tsModule ) == tsColor );
    QVERIFY( tsplot->getModuleColor( eeModule ) == eeColor );
    QVERIFY( tsplot->getModuleColor( ieModule ) == newColor );
}


#include "ModifyColorTest.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 0; 
