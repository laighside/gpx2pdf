/**
  @file    main.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  Program for placing waypoints from a GPX file onto a GeoPDF map
 */

#include "mainwindow.h"
#include <QApplication>

#include <iostream>
#include <string>
#include "gpx2pdf.h"

int main(int argc, char *argv[])
{
    if (argc > 1) {
        // if there are args then run in the command line

        if (argc == 4) {
            std::string gpxFile = std::string(argv[1]);
            std::string pdfFileIn = std::string(argv[2]);
            std::string pdfFileOut = std::string(argv[3]);

            if (gpx2pdf::doConversion(gpxFile, pdfFileIn, pdfFileOut) == gpx2pdf::SUCCESS) {
                std::cout << "GPX waypoints successfully added to PDF file\n";
            }

        } else {
            std::cout << "Expected 3 arguments: gpx_file, pdf_file_in, pdf_file_out\n";
        }
        return 0;

    } else {
        // if there are no extra args then launch the GUI
        QApplication a(argc, argv);
        MainWindow w;
        w.show();
        return a.exec();
    }
}
