/**
  @file    mainwindow.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A Qt GUI window that allows the user to browse and select files.
  Then launches the gpx2pdf conversion.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // slots for all the buttons on the GUI
    void gpxBrowseButtonClicked(bool);
    void pdfInBrowseButtonClicked(bool);
    void pdfOutBrowseButtonClicked(bool);
    void startButtonClicked(bool);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
