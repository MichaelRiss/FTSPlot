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


using namespace FTSPlot;

IntervalListLoader::IntervalListLoader ()
{
}

IntervalListLoader::~IntervalListLoader()
{
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

	BoxList.reset();
	BoxList.drawtype = GL_QUADS;
	LineList.reset();
	LineList.drawtype = GL_LINES;

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

							if ( xlow != xhigh )
							{
								BoxList.append( xlow );
								BoxList.append( ymin );
								BoxList.append( xhigh );
								BoxList.append( ymin );
								BoxList.append( xhigh );
								BoxList.append( ymax );
								BoxList.append( xlow );
								BoxList.append( ymax );
								LineList.append( xlow );
								LineList.append( ymin );
								LineList.append( xlow );
								LineList.append( ymax );
								LineList.append( xhigh );
								LineList.append( ymin );
								LineList.append( xhigh );
								LineList.append( ymax );
							}
							else
							{
								LineList.append( xlow );
								LineList.append( ymin );
								LineList.append( xlow );
								LineList.append( ymax );
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

		getRecursiveEvents ( XdataBegin, XdataEnd, treeDirName, 0, reqDispPower, reqXdataBegin, ymin, ymax, TOTALHEIGHT );
	}

	emit notifyListUpdate( &BoxList, &LineList );
}

void IntervalListLoader::getRecursiveEvents ( quint64 beginIdx, quint64 endIdx, QString path, quint64 pathValue, int reqDispPower, qint64 reqXdataBegin, double ymin, double ymax, int height )
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
											BoxList.append( xlow );
											BoxList.append( ymin );
											BoxList.append( xhigh );
											BoxList.append( ymin );
											BoxList.append( xhigh );
											BoxList.append( ymax );
											BoxList.append( xlow );
											BoxList.append( ymax );
											LineList.append( xlow );
											LineList.append( ymin );
											LineList.append( xlow );
											LineList.append( ymax );
											LineList.append( xhigh );
											LineList.append( ymin );
											LineList.append( xhigh );
											LineList.append( ymax );
										}
										else
										{
											LineList.append( xlow );
											LineList.append( ymin );
											LineList.append( xlow );
											LineList.append( ymax );
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
				getRecursiveEvents ( beginIdx, endIdx, path + "/" + subDirList[i], sliceValueBegin, reqDispPower, reqXdataBegin, ymin, ymax, height-1 );
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
										BoxList.append( xlow );
										BoxList.append( ymin );
										BoxList.append( xhigh );
										BoxList.append( ymin );
										BoxList.append( xhigh );
										BoxList.append( ymax );
										BoxList.append( xlow );
										BoxList.append( ymax );
										LineList.append( xlow );
										LineList.append( ymin );
										LineList.append( xlow );
										LineList.append( ymax );
										LineList.append( xhigh );
										LineList.append( ymin );
										LineList.append( xhigh );
										LineList.append( ymax );
									}
									else
									{
										LineList.append( xlow );
										LineList.append( ymin );
										LineList.append( xlow );
										LineList.append( ymax );
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
								BoxList.append( xlow );
								BoxList.append( ymin );
								BoxList.append( xhigh );
								BoxList.append( ymin );
								BoxList.append( xhigh );
								BoxList.append( ymax );
								BoxList.append( xlow );
								BoxList.append( ymax );
								LineList.append( xlow );
								LineList.append( ymin );
								LineList.append( xlow );
								LineList.append( ymax );
								LineList.append( xhigh );
								LineList.append( ymin );
								LineList.append( xhigh );
								LineList.append( ymax );							}
							else
							{
								LineList.append( xlow );
								LineList.append( ymin );
								LineList.append( xlow );
								LineList.append( ymax );
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

IntervalListLoader_Suspend::IntervalListLoader_Suspend ( QThread* thread )
{
	lock = thread;
	lock->quit();
	lock->wait();
}

IntervalListLoader_Suspend::~IntervalListLoader_Suspend()
{
	lock->start();
}
