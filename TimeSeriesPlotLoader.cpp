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


#include "TimeSeriesPlotLoader.h"
#include "commonDefs.h"

#if defined(BENCHMARK) && ( defined(COUNT_TILES) || defined(LINUX_DISABLE_FILESYSTEMCACHE) )
#include "benchmarks/benchmarkHelper.h"
#endif

#define MAX(a,b) a<b?b:a
#define MIN(a,b) a<b?a:b

using namespace std;
using namespace FTSPlot;

bool FTSPlot::isPow2( int a )
{
	return !(a & (a-1));
}

int FTSPlot::intlog2( int a )
{
	int ret = -1;
	while (a != 0) {
		a >>= 1;
		ret++;
	}
	return ret;
}


TimeSeriesPlotLoader::TimeSeriesPlotLoader ( QOpenGLContext* glcontext )
: initialized( false ),
  dispList( 0 ), begin(0), end(0), reqBegin(0), reqEnd(0), power(0),
  reqDispList(0), NoSamples(0), mySurface( glcontext->surface() )
{
	//this->glwindow = glwindow;
	//myGLwidget = new QGLWidget ( glwindow->format(), NULL, glwindow );
	//myGLContext = new QGLContext( glwindow->format() );
	//qDebug() << "Transmitted context pointer: " << glcontext;

	myGLContext = new QOpenGLContext( this );
	myGLContext->setFormat( glcontext->format() );
	myGLContext->setScreen( glcontext->screen() );
	myGLContext->setShareContext( glcontext->shareContext() );
	if( !myGLContext->create() ){
		qDebug() << "Cannot create new context.";
		exit(1);
	}

//	if( !myGLContext->create( glcontext ) ){
//		cout << "Failed to create new context." << endl;
//	}
	if ( !QOpenGLContext::areSharing( glcontext, myGLContext ) ){
		qDebug() << "GLPlotLoader: Context is not shared. Exiting!";
		exit ( 1 );
	}
	if( !myGLContext->isValid() ){
		qDebug() << "TimeSeriesPlotLoader: Context not valid. Exiting!";
		exit(1);
	}
//	if( myGLContext->format() != glcontext->format() )
//	{
//		cout << "GLPlotLoader: Cannot get the same GL context format as main widget. Exiting!" << endl;
//		exit ( 1 );
//	}

	myGLContext->makeCurrent( mySurface );

	//    if( !myGLContext->isValid() )
	//    {
	//    	cerr << "Invalid GL context!";
	//    }

	reqPower = 0;
	filesready = false;
	//newJob = false;
	//stopThread = false;
	recordMin = DoubleInfinity;
	recordMax = -DoubleInfinity;

#if defined(BENCHMARK) && defined(LINUX_DISABLE_FILESYSTEMCACHE)
	benchmarkHelper::filesptr = &files;
#endif

	// start thread
	//start();
}

TimeSeriesPlotLoader::~TimeSeriesPlotLoader()
{
	//stop();
	uchar* lastPtr = NULL;
	for ( int i = 0; i < files.size(); i++ )
	{
		if ( files[i].mmap != lastPtr )
		{
			lastPtr = (uchar*) files[i].mmap;
			if ( !files[i].qfile->unmap ( ( uchar* ) files[i].mmap ) )
			{
				cout << "Error: GLPlotLoader cannot unmap files" << endl;
				exit ( 1 );
			}
			files[i].qfile->close();
			delete ( files[i].qfile );
		}
	}
	//myGLContext->doneCurrent();
	delete( myGLContext );
}

bool TimeSeriesPlotLoader::openFile ( QString filename )
{
	if ( filesready )
	{
		cout << "GLPlot: have already some open files!" << endl;
		return false;
	}
	// parse configfile
	QFile cfgfile ( filename );
	if ( !cfgfile.open ( QFile::ReadOnly ) )
	{
		qDebug() << "Cannot open " << filename << endl;
		return false;
	}
	QFileInfo cfgInfo ( cfgfile );
	QDir dataDirectory = cfgInfo.absoluteDir();

	QTextStream cfgstream ( &cfgfile );

	// raw part
	mmapFileInfo entry;
	QString rawKeyWord;
	QString rawFilename;
	cfgstream >> rawKeyWord >> rawFilename;
	cfgstream.skipWhiteSpace();
	entry.pow = 0;

	entry.qfile = new QFile ( dataDirectory.filePath ( rawFilename ) );
	if ( !entry.qfile->open ( QIODevice::ReadOnly ) )
	{
		qDebug() << "Error opening " << dataDirectory.filePath ( rawFilename );
		return false;
	}
	// get file length

	entry.size = entry.qfile->size();
	NoSamples = entry.size / sizeof ( double );


	entry.mmap = entry.qfile->map ( 0, entry.size );
	if ( entry.mmap == 0 )
	{
		cout << "Error mapping memory." << endl;
		cout << "raw file" << endl;
		return false;
	}
	files.append ( entry );

	// aggl parts into a vector ( QT - style )
	while ( !cfgstream.atEnd() )
	{
		QString powerLevel;
		QString levelFilename;
		cfgstream >> powerLevel >> levelFilename;

		int power = powerLevel.toInt();
		//if ( pow ( 2,round ( log2 ( power ) ) ) != power )
		if (!isPow2( power ))
		{
			qDebug() << "Hierarchy level" << power << "isn't a power of 2.";
			qDebug() << "Exiting.";
			return false;
		}

		mmapFileInfo entry;
		//entry.pow = round ( log2 ( power ) );
		entry.pow = intlog2( power );

		entry.qfile = new QFile ( dataDirectory.filePath ( levelFilename ) );
		if ( !entry.qfile->open ( QIODevice::ReadOnly ) )
		{
			qDebug() << "Error opening " << dataDirectory.filePath ( levelFilename );
			return false;
		}

		// get file length
		entry.size = entry.qfile->size();
		entry.mmap = entry.qfile->map ( 0, entry.size );
		if ( entry.mmap == 0 )
		{
			qDebug() << "Error mapping memory.";
			return false;
		}

		// fill up the "in-between" slots
		if ( files.size() < entry.pow )
		{
			mmapFileInfo last = files.last();
			int origVectorSize = files.size();
			for ( int i = 0; i < entry.pow - origVectorSize; i++ )
			{
				files.append ( last );
			}
		}
		files.append ( entry );
		cfgstream.skipWhiteSpace();
	}

	// Find min and max values
	recordMin = DoubleInfinity;
	recordMax = -DoubleInfinity;
	mmapFileInfo topFI = files.last();
	for ( unsigned int i = 0; i < topFI.size / sizeof ( double ); i += 2 )
	{
		if ( ( ( double* ) topFI.mmap ) [i] < recordMin )
		{
			recordMin = ( ( double* ) topFI.mmap ) [i];
		}
		if ( ( ( double* ) topFI.mmap ) [i+1] > recordMax )
		{
			recordMax = ( ( double* ) topFI.mmap ) [i+1];
		}
	}

	filesready = true;

	return true;
}

//bool TimeSeriesPlotLoader::initGL(){
//
//}

void TimeSeriesPlotLoader::genDisplayList ( qint64 Xbegin, qint64 Xend, int reqPower, GLuint displaylist )
{
	if ( !filesready )
	{
		cout << "GLPlotLoader: no files open, cannot generate display list!"
				<< endl;
		return;
	}
	//	if( !initialized )
	//	{
	//		if( !initGL() )
	//		{
	//			cerr << "Cannot init GL Context, not rendering display list!" << endl;
	//			return;
	//		}
	//	}

	this->reqBegin = Xbegin;
	this->reqEnd = Xend;
	this->reqPower = reqPower;
	this->reqDispList = displaylist;
	//newJob = true;
	//done = false;


	//---------


	//done = false;
	//stopThread = false;

	if( !myGLContext->isValid() ){
		myGLContext->makeCurrent( mySurface );
		if( !myGLContext->isValid() )
		{
			cerr << "Invalid GL context!";
			return;
		}
	}

	// We also have to initialize this OpenGL Context to use alpha blending for example
	// TODO: maybe move to constructor?
	glClearColor ( 1.0, 1.0, 1.0, 1.0 );
	glShadeModel ( GL_FLAT );

	glEnable ( GL_LINE_SMOOTH );
	glEnable ( GL_BLEND );
	glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glHint ( GL_LINE_SMOOTH_HINT, GL_DONT_CARE );
	glLineWidth ( 1.0 );


	// waitCond
	//		ctrlLock.lock();
	//		if ( !newJob && !stopThread )
	//		{
	//			ctrlWait.wait ( &ctrlLock );
	//		}
	// check stopThread and exit
	//		if ( stopThread )
	//		{
	//			break;
	//		}
	// check impossible condition !newJob && !stopThread
	//		if ( !stopThread && !newJob )
	//		{
	//			cout << "PaintThread: !stopThread && !newJob => invalid condition!" << endl;
	//			exit ( 1 );
	//		}

#ifdef COUNT_TILES
	qint64 vertexCount = 0;
	qint64 lineCount = 0;
#endif // COUNT_TILES

	begin = reqBegin;
	end = reqEnd;

	power = reqPower;
	dispList = reqDispList;
	//		done = false;
	//		newJob = false;
	//		ctrlLock.unlock();

	//	if ( !QGLContext::currentContext() )
	//	{
	//		cout << "No valid GLcontext!" << endl;
	//	}

	// Finding the right dataset
	mmapFileInfo work;
	if ( power >= files.size() )
	{
		work = files.last();
	}
	else
	{
		work = files[power];
	}

#define AGGL_THR 2

	if ( work.pow == 0 )
	{
		if ( power - work.pow < AGGL_THR )
		{
			// Draw lines
			glNewList ( dispList, GL_COMPILE );
			glBegin ( GL_LINE_STRIP );
			for ( qint64 i = ( MAX ( begin, 0 ) ); i < ( MIN ( end, NoSamples ) ); i++ )
			{
				double xcoord = ( double ) ( ( i - begin ) >>power );
				double ycoord = ( ( double* ) work.mmap ) [i];
				glVertex2d ( xcoord, ycoord );

#ifdef COUNT_TILES
				vertexCount++;
				lineCount++;
#endif // COUNT_TILES

			}
			glEnd();
			glEndList();
		}
		else
		{
			// Draw bars
			glNewList ( dispList, GL_COMPILE );
			glBegin ( GL_LINES );
			double oldmin = DoubleNaN;
			double oldmax = DoubleNaN;
			qint64 loopbegin = MAX ( ( begin >> power ) << power, 0 );
			qint64 loopend = MIN ( ( end >> power ) << power, NoSamples );
			qint64 jbegin = MAX ( begin - loopbegin, 0 );
			for ( qint64 i = loopbegin; i < loopend; i = i + ( ( qint64 ) 1<<power ) )
			{
				double minvalue = DoubleInfinity;
				double maxvalue = -DoubleInfinity;
				for ( qint64 j = ( i==loopbegin ) ?jbegin:0; ( j < ( ( qint64 ) 1<<power ) ) && ( i+j < loopend ); j++ )
				{
					if ( ( ( double* ) work.mmap ) [i+j] < minvalue )
					{
						minvalue = ( ( double* ) work.mmap ) [i+j];
					}
					if ( ( ( double* ) work.mmap ) [i+j] > maxvalue )
					{
						maxvalue = ( ( double* ) work.mmap ) [i+j];
					}
				}

				// Comparison with NaN always yields false, so the initial
				// comparison should be handled correctly
				if ( minvalue >= oldmax || maxvalue <= oldmin )
				{
					glVertex2d ( ( i-1 + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power,
							( oldmin + oldmax ) / 2.0 );
					glVertex2d ( ( i + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power,
							( minvalue + maxvalue ) / 2.0 );

#ifdef COUNT_TILES
					vertexCount += 2;
					lineCount++;
#endif // COUNT_TILES

				}
				glVertex2d ( ( i + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power,
						minvalue );
				glVertex2d ( ( i + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power,
						maxvalue );

#ifdef COUNT_TILES
				vertexCount += 2;
				lineCount++;
#endif // COUNT_TILES

				oldmin = minvalue;
				oldmax = maxvalue;
			}
			glEnd();
			glEndList();
		}
	}
	else
	{
		// Draw bars
		glNewList ( dispList, GL_COMPILE );
		glBegin ( GL_LINES );

		double oldmin = DoubleNaN;
		double oldmax = DoubleNaN;

		qint64 loopbegin = MAX ( ( begin >> power ) << ( power-work.pow ), 0 );
		qint64 loopend = MIN ( ( end >> power ) << ( power-work.pow ), NoSamples >> work.pow );
		qint64 jbegin = MAX ( ( begin >> work.pow ) - loopbegin, 0 );
		for ( qint64 i = loopbegin; i < loopend; i += ( ( qint64 ) 1<< ( power-work.pow ) ) )
		{
			double minvalue = DoubleInfinity;
			double maxvalue = -DoubleInfinity;
			for ( qint64 j = ( i==loopbegin ) ?jbegin:0;
					( j < ( ( qint64 ) 1<< ( power-work.pow ) ) ) &&
							( ( i+j ) < loopend );
					j++ )
			{
				if ( ( ( double* ) work.mmap ) [ ( i+j ) <<1] < minvalue )
				{
					minvalue = ( ( double* ) work.mmap ) [ ( i+j ) <<1];
				}
				if ( ( ( double* ) work.mmap ) [ ( ( i+j ) <<1 ) +1] > maxvalue )
				{
					maxvalue = ( ( double* ) work.mmap ) [ ( ( i+j ) <<1 ) +1];
				}
			}

			// Comparison with NaN always yields false, so the initial
			// comparison should be handled correctly

			if ( minvalue >= oldmax || maxvalue <= oldmin )
			{
				glVertex2d ( ( ( i<<work.pow )-1 + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power, ( oldmin + oldmax ) / 2.0 );
				glVertex2d ( ( ( i<<work.pow ) + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power, ( minvalue + maxvalue ) / 2.0 );

#ifdef COUNT_TILES
				vertexCount += 2;
				lineCount++;
#endif // COUNT_TILES

			}

			glVertex2d ( ( ( i<<work.pow ) + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power, minvalue );
			glVertex2d ( ( ( i<<work.pow ) + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power, maxvalue );

#ifdef COUNT_TILES
			vertexCount += 2;
			lineCount++;
#endif // COUNT_TILES

			oldmin = minvalue;
			oldmax = maxvalue;
		}

		glEnd();
		glEndList();
	}

	GLenum err = glGetError();
	if ( err != GL_NO_ERROR )
	{
		cout << "GLPlotLoader: " << gluErrorString ( err ) << endl;
	}

#if defined(COUNT_TILES) && !defined(BENCHMARK)
	qDebug() << "TimeSeriesPlotLoader: generated" << vertexCount << "vertexes and" << lineCount << "lines.";
#endif // COUNT_TILES

#if defined(BENCHMARK) && defined(COUNT_TILES)
	benchmarkHelper::vertexCount = vertexCount;
	benchmarkHelper::lineCount = lineCount;
	benchmarkHelper::quadCount = 0;
#endif // BENCHCOUNT

	//done = true;

	emit notifyListUpdate();
}



double TimeSeriesPlotLoader::getMin()
{
	return recordMin;
}

double TimeSeriesPlotLoader::getMax()
{
	return recordMax;
}

//void TimeSeriesPlotLoader::run()
//{
//	done = false;
//	//stopThread = false;
//
//	myGLContext->makeCurrent();
//	if( !myGLContext->isValid() )
//	{
//		cerr << "Invalid GL context!";
//	}
//
//	// We also have to initialize this OpenGL Context to use alpha blending for example
//	glClearColor ( 1.0, 1.0, 1.0, 1.0 );
//	glShadeModel ( GL_FLAT );
//
//	glEnable ( GL_LINE_SMOOTH );
//	glEnable ( GL_BLEND );
//	glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
//	glHint ( GL_LINE_SMOOTH_HINT, GL_DONT_CARE );
//	glLineWidth ( 1.0 );
//
//	while ( 1 )
//	{
//		// waitCond
//		ctrlLock.lock();
//		if ( !newJob && !stopThread )
//		{
//			ctrlWait.wait ( &ctrlLock );
//		}
//		// check stopThread and exit
//		if ( stopThread )
//		{
//			break;
//		}
//		// check impossible condition !newJob && !stopThread
//		if ( !stopThread && !newJob )
//		{
//			cout << "PaintThread: !stopThread && !newJob => invalid condition!" << endl;
//			exit ( 1 );
//		}
//
//#ifdef COUNT_TILES
//		qint64 vertexCount = 0;
//		qint64 lineCount = 0;
//#endif // COUNT_TILES
//
//		begin = reqBegin;
//		end = reqEnd;
//
//		power = reqPower;
//		dispList = reqDispList;
//		done = false;
//		newJob = false;
//		ctrlLock.unlock();
//
//		if ( !QGLContext::currentContext() )
//		{
//			cout << "No valid GLcontext!" << endl;
//		}
//
//		// Finding the right dataset
//		mmapFileInfo work;
//		if ( power >= files.size() )
//		{
//			work = files.last();
//		}
//		else
//		{
//			work = files[power];
//		}
//
//#define AGGL_THR 2
//
//		if ( work.pow == 0 )
//		{
//			if ( power - work.pow < AGGL_THR )
//			{
//				// Draw lines
//				glNewList ( dispList, GL_COMPILE );
//				glBegin ( GL_LINE_STRIP );
//				for ( qint64 i = ( MAX ( begin, 0 ) ); i < ( MIN ( end, NoSamples ) ); i++ )
//				{
//					double xcoord = ( double ) ( ( i - begin ) >>power );
//					double ycoord = ( ( double* ) work.mmap ) [i];
//					glVertex2d ( xcoord, ycoord );
//
//#ifdef COUNT_TILES
//					vertexCount++;
//					lineCount++;
//#endif // COUNT_TILES
//
//				}
//				glEnd();
//				glEndList();
//			}
//			else
//			{
//				// Draw bars
//				glNewList ( dispList, GL_COMPILE );
//				glBegin ( GL_LINES );
//				double oldmin = DoubleNaN;
//				double oldmax = DoubleNaN;
//				qint64 loopbegin = MAX ( ( begin >> power ) << power, 0 );
//				qint64 loopend = MIN ( ( end >> power ) << power, NoSamples );
//				qint64 jbegin = MAX ( begin - loopbegin, 0 );
//				for ( qint64 i = loopbegin; i < loopend; i = i + ( ( qint64 ) 1<<power ) )
//				{
//					double minvalue = DoubleInfinity;
//					double maxvalue = -DoubleInfinity;
//					for ( qint64 j = ( i==loopbegin ) ?jbegin:0; ( j < ( ( qint64 ) 1<<power ) ) && ( i+j < loopend ); j++ )
//					{
//						if ( ( ( double* ) work.mmap ) [i+j] < minvalue )
//						{
//							minvalue = ( ( double* ) work.mmap ) [i+j];
//						}
//						if ( ( ( double* ) work.mmap ) [i+j] > maxvalue )
//						{
//							maxvalue = ( ( double* ) work.mmap ) [i+j];
//						}
//					}
//
//					// Comparison with NaN always yields false, so the initial
//					// comparison should be handled correctly
//					if ( minvalue >= oldmax || maxvalue <= oldmin )
//					{
//						glVertex2d ( ( i-1 + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power,
//								( oldmin + oldmax ) / 2.0 );
//						glVertex2d ( ( i + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power,
//								( minvalue + maxvalue ) / 2.0 );
//
//#ifdef COUNT_TILES
//						vertexCount += 2;
//						lineCount++;
//#endif // COUNT_TILES
//
//					}
//					glVertex2d ( ( i + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power,
//							minvalue );
//					glVertex2d ( ( i + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power,
//							maxvalue );
//
//#ifdef COUNT_TILES
//					vertexCount += 2;
//					lineCount++;
//#endif // COUNT_TILES
//
//					oldmin = minvalue;
//					oldmax = maxvalue;
//				}
//				glEnd();
//				glEndList();
//			}
//		}
//		else
//		{
//			// Draw bars
//			glNewList ( dispList, GL_COMPILE );
//			glBegin ( GL_LINES );
//
//			double oldmin = DoubleNaN;
//			double oldmax = DoubleNaN;
//
//			qint64 loopbegin = MAX ( ( begin >> power ) << ( power-work.pow ), 0 );
//			qint64 loopend = MIN ( ( end >> power ) << ( power-work.pow ), NoSamples >> work.pow );
//			qint64 jbegin = MAX ( ( begin >> work.pow ) - loopbegin, 0 );
//			for ( qint64 i = loopbegin; i < loopend; i += ( ( qint64 ) 1<< ( power-work.pow ) ) )
//			{
//				double minvalue = DoubleInfinity;
//				double maxvalue = -DoubleInfinity;
//				for ( qint64 j = ( i==loopbegin ) ?jbegin:0;
//						( j < ( ( qint64 ) 1<< ( power-work.pow ) ) ) &&
//								( ( i+j ) < loopend );
//						j++ )
//				{
//					if ( ( ( double* ) work.mmap ) [ ( i+j ) <<1] < minvalue )
//					{
//						minvalue = ( ( double* ) work.mmap ) [ ( i+j ) <<1];
//					}
//					if ( ( ( double* ) work.mmap ) [ ( ( i+j ) <<1 ) +1] > maxvalue )
//					{
//						maxvalue = ( ( double* ) work.mmap ) [ ( ( i+j ) <<1 ) +1];
//					}
//				}
//
//				// Comparison with NaN always yields false, so the initial
//				// comparison should be handled correctly
//
//				if ( minvalue >= oldmax || maxvalue <= oldmin )
//				{
//					glVertex2d ( ( ( i<<work.pow )-1 + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power, ( oldmin + oldmax ) / 2.0 );
//					glVertex2d ( ( ( i<<work.pow ) + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power, ( minvalue + maxvalue ) / 2.0 );
//
//#ifdef COUNT_TILES
//					vertexCount += 2;
//					lineCount++;
//#endif // COUNT_TILES
//
//				}
//
//				glVertex2d ( ( ( i<<work.pow ) + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power, minvalue );
//				glVertex2d ( ( ( i<<work.pow ) + ( ( qint64 ) 1<< ( power-1 ) ) - begin ) >> power, maxvalue );
//
//#ifdef COUNT_TILES
//				vertexCount += 2;
//				lineCount++;
//#endif // COUNT_TILES
//
//				oldmin = minvalue;
//				oldmax = maxvalue;
//			}
//
//			glEnd();
//			glEndList();
//		}
//
//		GLenum err = glGetError();
//		if ( err != GL_NO_ERROR )
//		{
//			cout << "GLPlotLoader: " << gluErrorString ( err ) << endl;
//		}
//
//#if defined(COUNT_TILES) && !defined(BENCHMARK)
//		qDebug() << "TimeSeriesPlotLoader: generated" << vertexCount << "vertexes and" << lineCount << "lines.";
//#endif // COUNT_TILES
//
//#if defined(BENCHMARK) && defined(COUNT_TILES)
//		benchmarkHelper::vertexCount = vertexCount;
//		benchmarkHelper::lineCount = lineCount;
//		benchmarkHelper::quadCount = 0;
//#endif // BENCHCOUNT
//
//		done = true;
//
//		emit notifyListUpdate();
//	}
//}

//void TimeSeriesPlotLoader::stop()
//{
//	stopThread = true;
//	ctrlWait.wakeAll();
//	wait();
//}

qint64 TimeSeriesPlotLoader::getXMin()
{
	return 0;
}

qint64 TimeSeriesPlotLoader::getXMax()
{
	return NoSamples;
}

// kate: indent-mode cstyle; space-indent on; indent-width 0; 
