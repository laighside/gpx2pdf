/**
  @file    mainwindow.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A Qt GUI window that allows the user to browse and select files.
  Then launches the gpx2pdf conversion.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <sstream>
#include <iostream>
#include <QFileDialog>

#include "gpx2pdf.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // connect all the buttons to their slots
    connect(this->ui->gpxBrowseButton, SIGNAL(clicked(bool)), this, SLOT(gpxBrowseButtonClicked(bool)));
    connect(this->ui->pdfInBrowseButton, SIGNAL(clicked(bool)), this, SLOT(pdfInBrowseButtonClicked(bool)));
    connect(this->ui->pdfOutBrowseButton, SIGNAL(clicked(bool)), this, SLOT(pdfOutBrowseButtonClicked(bool)));
    connect(this->ui->startButton, SIGNAL(clicked(bool)), this, SLOT(startButtonClicked(bool)));

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::gpxBrowseButtonClicked(bool) {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open GPX File"), "", tr("GPX Files (*.gpx)"));
    if (fileName.size())
        this->ui->gpxFileLineEdit->setText(fileName);
}

void MainWindow::pdfInBrowseButtonClicked(bool) {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open PDF File"), "", tr("GeoPDF Files (*.pdf)"));
    if (fileName.size())
        this->ui->pdfFileInLineEdit->setText(fileName);
}

void MainWindow::pdfOutBrowseButtonClicked(bool) {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save PDF File"), "", tr("GeoPDF Files (*.pdf)"));
    if (fileName.size())
        this->ui->pdfFileOutLineEdit->setText(fileName);
}

void MainWindow::startButtonClicked(bool) {
    // redirect std::cout to place it in a text box on the GUI
    // This could be done better so that it updates as the conversion happens, and not just once at the end
    std::stringstream buffer;
    std::streambuf * old = std::cout.rdbuf(buffer.rdbuf());

    gpx2pdf converter(this->ui->gpxFileLineEdit->text().toStdString(), this->ui->pdfFileInLineEdit->text().toStdString(), this->ui->pdfFileOutLineEdit->text().toStdString());
    converter.setUseGeocacheName(this->ui->geocacheNameCheckBox->isChecked());
    converter.setUseGsakSmartName(this->ui->smartNameCheckBox->isChecked());
    converter.setMaxNameLength(this->ui->nameLengthSpinBox->value());
    converter.setNameFontSize(this->ui->fontSizeSpinBox->value());
    if (converter.doConversion() == gpx2pdf::SUCCESS) {
        std::cout << "GPX waypoints successfully added to PDF file\n";
    }

    // put std::cout back how it was
    std::cout.rdbuf(old);
    this->ui->statusTextEdit->setPlainText(QString::fromStdString(buffer.str()));
}
