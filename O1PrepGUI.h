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


#ifndef O1PREPGUI_H
#define O1PREPGUI_H

#include <QWidget>
#include "O1Prep.h"
#include "ui_O1PrepGUI.h"

#define FILESIZELIMIT 10485760

namespace O1Plot
{

class O1PrepGUI : public QWidget
{
    Q_OBJECT

public:
    O1PrepGUI();
    void setDataProcessor( O1Prep* dth );

public slots:
    void addFileHandler();
    void RemoveFileHandler();
    void StartStopHandlder();
    void ResumeHandler();
    void handleThreadFinish();
    void handleProgressUpdate( int totalProgress, int level, int fileProgress );

signals:
    void abortThread();

private:
    bool abortion;
    quint64 FileIdxAcc;
    quint64 totalFileIdx;
    QVector<quint64> fileIdxs;
    QVector<double> fileFactors;
    int jobFileIdx;
    O1Prep* dth;
    Ui::UIWidget ui;
    enum GUIstate {dth_missing, idle_noFile, idle_File, Running};
    GUIstate state;
    void updateGUI();
};

}

#endif // O1PREPGUI_H
// kate: indent-mode cstyle; space-indent on; indent-width 0; 
