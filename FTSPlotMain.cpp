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


#include <QTextStream>
#include <QCoreApplication>
#include "mainwindow.h"
#include "FTSPrep.h"
#include <iostream>
#include <QtDebug>

#ifdef Q_WS_X11

#include <X11/Xlib.h>

#endif


using namespace std;

bool isnt_config_file(char *filename) {
    unsigned length = strlen(filename);
    return strcmp(filename + (length - 4), ".cfg");
}

void preprocess(int argc, char *argv[]) {
    FTSPrep ftsPrep;

    for (unsigned i = 1; i < argc; i++) {
        if (isnt_config_file(argv[i])) {
            QString str(argv[i]);
            ftsPrep.setNewFile(str);
        }
    }
    ftsPrep.run();

}

int main(int argc, char *argv[]) {
#ifdef Q_WS_X11
    XInitThreads();
#endif

    QApplication app(argc, argv);


    MainWindow *mainwidget = new MainWindow();

    if (argc) {
        preprocess(argc, argv);
        unsigned i;
        for (i = 0; i < argc; i++) {
            QString str(argv[i]);
            if (isnt_config_file(argv[i])) {
                str.append(".cfg");
            }
            mainwidget->MySimpleViewWidget->addTimeSeries(str);
        }
    }

    mainwidget->show();
    return app.exec();
}

