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


#ifndef O1PREP_H
#define O1PREP_H

#include <QThread>
#include <QMutex>
#include <QFile>

#ifndef INFINITY
#include <limits>
#define INFINITY std::numeric_limits<double>::infinity()
#endif

namespace O1Plot
{

typedef struct {
    quint64 ResumethinFactor;
    quint64 ResumePosition;
} ResumeData;

class O1Prep : public QThread
{
    Q_OBJECT
public:
    O1Prep();
    virtual ~O1Prep();

    bool setNewFile( QString fileName, quint64 thinfactor = 64, quint64 FileSizeLimit = 10485760 );
    bool setresumeFile( QString fileName, quint64 thinfactor, quint64 FileSizeLimit, quint64 level,
                        quint64 position );
    void setProgressGranularity( quint64 gran = 2048 );
    ResumeData getResumeData();

public slots:
    void stopforResume();

signals:
    void QDTexception( QString msg );
    void progressUpdate( int totalProgress, int level, int levelProgress );

protected:
    void run();

private:
    enum workstate {ready, busy};
    workstate state;
    bool abort;
    quint64 ResumethinFactor;
    quint64 ResumePosition;
    QMutex threadlock;
    QString inFileName;
    quint64 thinFactor;
    quint64 FileSizeLimit;
    bool resume;
    quint64 level;
    quint64 position;
    quint64 granularity;
};

}

#endif // O1PREP_H
// kate: indent-mode cstyle; space-indent on; indent-width 0; 
