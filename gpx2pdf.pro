#-------------------------------------------------
#
# Project created by QtCreator 2021-08-29T17:12:41
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gpx2pdf
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        gpx2pdf.cpp

HEADERS += \
        mainwindow.h \
        gpx2pdf.h

FORMS += \
        mainwindow.ui

# Path for libraries
unix:LIBS += -L/usr/local/lib

# Libs for PDF reading/writing
# GDAL version 3.0 or higher is required (for reading the geospatial data in GeoPDF)
#  - this depends on Proj version 6.0 or higher
#  - GDAL needs to be complied with PoDoFo support
# PoDoFo is required for reading/writing the PDF files
#  - older versions do not work with OpenSSL 1.1.1
#  - PoDoFo version 0.9.7 is ok but needs to be compied with PODOFO_HAVE_OPENSSL_1_1 defined

# The prebuilt binaries of PoDoFo do not work with OpenSSL 1.1.1
# Hence need to build podofo 0.9.7 with PODOFO_HAVE_OPENSSL_1_1 defined
LIBS += -lpodofo

# All the libs that PoFoDo depends on
#LIBS += -lz -lfreetype -lfontconfig -ljpeg -lunistring -lidn -lcrypto

# GDAL is used for reading the GeoPDF coordinate infomation
LIBS += -lgdal

# All the libs that GDAL depends on
#LIBS += -lpng -ltiff -ldl -lexpat -lpcre -lproj


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
