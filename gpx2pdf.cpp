/**
  @file    gpx2pdf.cpp
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A class that reads waypoints from a GPX file and places them on a map from a GeoPDF file
  The status is outputed to std::cout as the data is processed
 */

#include "gpx2pdf.h"

#include <iostream>

#include <QFile>
#include <QtXml/QDomDocument>

#include <podofo/podofo.h>
#include <gdal.h>
#include <gdal_priv.h>
#include <ogr_core.h>
#include <ogr_spatialref.h>

#define GPX2PDF_VERSION  "1.0"

gpx2pdf::gpx2pdf(std::string gpxFile, std::string pdfFileIn, std::string pdfFileOut) {
    this->gpxFile = gpxFile;
    this->pdfFileIn = pdfFileIn;
    this->pdfFileOut = pdfFileOut;
    this->pageNumber = 1;
    this->useGeocacheName = true;
    this->useGsakSmartName = true;
    this->maxNameLength = 10;
    this->nameFontSize = 8.0;
    this->pdfSRS = nullptr;
    this->WGS84.SetWellKnownGeogCS("WGS84");
    this->coordTF = nullptr;
    this->xPixels = 0;
    this->yPixels = 0;
}

gpx2pdf::~gpx2pdf() {
    if (this->coordTF)
        OCTDestroyCoordinateTransformation(this->coordTF);
    if (this->pdfSRS)
        OGRSpatialReference::DestroySpatialReference(this->pdfSRS);
}

gpx2pdf::g2pErr gpx2pdf::doConversion() {
    gpx2pdf::g2pErr result = gpx2pdf::SUCCESS;
    std::cout << "gpx2pdf version " GPX2PDF_VERSION "\n";

    result = this->loadGpx();
    if (result != gpx2pdf::SUCCESS)
        return result;

    result = this->getGeospatialData();
    if (result != gpx2pdf::SUCCESS)
        return result;

    result = this->savePdf();
    if (result != gpx2pdf::SUCCESS)
        return result;

    return result;
}

gpx2pdf::g2pErr gpx2pdf::doConversion(std::string gpxFile, std::string pdfFileIn, std::string pdfFileOut) {
    gpx2pdf instance(gpxFile, pdfFileIn, pdfFileOut);
    return instance.doConversion();
}

gpx2pdf::g2pErr gpx2pdf::loadGpx() {
    QDomDocument xmlDoc;
    std::cout << "Reading GPX file: " << this->gpxFile << "\n";

    QFile file(QString::fromStdString(this->gpxFile));
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Unable to open GPX file for reading: " << this->gpxFile << "\n";
        return gpx2pdf::FILE_ERROR;
    }
    if (!xmlDoc.setContent(&file)) {
        file.close();
        std::cout << "Unable to parse GPX file - GPX file is not valid\n";
        return gpx2pdf::PARSE_ERROR;
    }
    file.close();

    QDomNodeList xmlWaypoints = xmlDoc.elementsByTagName("wpt");
    for (int i = 0; i < xmlWaypoints.length(); i++) {
        QDomNode xmlWaypoint = xmlWaypoints.at(i);

        if (!xmlWaypoint.attributes().contains("lat") || !xmlWaypoint.attributes().contains("lon"))
            continue;

        QDomNode nameNode = xmlWaypoint.namedItem("name");
        if (nameNode.isNull() || !nameNode.isElement())
            continue;

        QString nameStr = nameNode.toElement().text();

        double lat = xmlWaypoint.attributes().namedItem("lat").nodeValue().toDouble();
        double lon = xmlWaypoint.attributes().namedItem("lon").nodeValue().toDouble();

        if (this->useGeocacheName) {
            QDomNode cacheNameNode = xmlWaypoint.namedItem("groundspeak:cache").namedItem("groundspeak:name");
            if (!cacheNameNode.isNull() && cacheNameNode.isElement())
                nameStr = cacheNameNode.toElement().text();
        }

        if (this->useGsakSmartName) {
            QDomNode gsakNameNode = xmlWaypoint.namedItem("gsak:wptExtension").namedItem("gsak:SmartName");
            if (!gsakNameNode.isNull() && gsakNameNode.isElement())
                nameStr = gsakNameNode.toElement().text();
        }

        if (this->maxNameLength >= 0)
            nameStr = nameStr.left(this->maxNameLength);

        this->waypoints.push_back({lat, lon, nameStr.toStdString()});
    }

    std::cout << this->waypoints.size() << " waypoint(s) read\n";
    if (this->waypoints.size() < 1)
        return gpx2pdf::EMPTY_DATA;

    return gpx2pdf::SUCCESS;
}

gpx2pdf::g2pErr gpx2pdf::savePdf() {
    PoDoFo::PdfError::EnableDebug(false);
    PoDoFo::PdfError::EnableLogging(false);

    PoDoFo::PdfMemDocument* docPodofo = new PoDoFo::PdfMemDocument();
    PoDoFo::PdfPage* pagePodofo = nullptr;

    try {
        docPodofo->Load(this->pdfFileIn.c_str());
    } catch(PoDoFo::PdfError& pdfError) {
        if (pdfError.GetError() == PoDoFo::ePdfError_InvalidPassword) {
            if (this->pdfPassword.size()) {
                try {
                    docPodofo->SetPassword(this->pdfPassword);
                } catch(PoDoFo::PdfError& pdfError2) {
                    if (pdfError2.GetError() == PoDoFo::ePdfError_InvalidPassword) {
                        std::cout << "Invalid password\n";
                    } else {
                        std::cout << "Invalid PDF: " << pdfError2.what() << "\n";
                    }
                    delete docPodofo;
                    return gpx2pdf::ERROR;
                } catch(...) {
                    std::cout << "Invalid PDF\n";
                    delete docPodofo;
                    return gpx2pdf::ERROR;
                }
            } else {
                std::cout << "PDF file is encrypted\n";
                delete docPodofo;
                return gpx2pdf::FILE_ERROR;
            }
        } else {
            std::cout << "PDF Error: " << pdfError.what() << "\n";
            delete docPodofo;
            return gpx2pdf::ERROR;
        }
    } catch(...) {
        std::cout << "Invalid PDF\n";
        delete docPodofo;
        return gpx2pdf::ERROR;
    }

    int nPages = docPodofo->GetPageCount();
    if (this->pageNumber < 1 || this->pageNumber > nPages) {
        std::cout << "Invalid page number: " << this->pageNumber << " (PDF file has " << nPages << " pages)\n";
        delete docPodofo;
        return gpx2pdf::INVALID_ARGUMENT;
    }

    try {
        pagePodofo = docPodofo->GetPage(this->pageNumber - 1);
    } catch(PoDoFo::PdfError& pdfError) {
        std::cout << "PDF Error: " << pdfError.what() << "\n";
        delete docPodofo;
        return gpx2pdf::ERROR;
    } catch(...) {
        std::cout << "Invalid PDF\n";
        delete docPodofo;
        return gpx2pdf::ERROR;
    }

    if (!pagePodofo) {
        std::cout << "Invalid PDF Page\n";
        delete docPodofo;
        return gpx2pdf::INVALID_ARGUMENT;
    }

    try {
        PoDoFo::PdfPainter painter;
        PoDoFo::PdfFont* pFont;

        painter.SetPage(pagePodofo);

        pFont = docPodofo->CreateFont("Helvetica");

        if (!pFont) {
            std::cout << "Error creating font\n";
            delete docPodofo;
            return gpx2pdf::ERROR;
        }

        pFont->SetFontSize(this->nameFontSize);
        painter.SetFont(pFont);

        const PoDoFo::PdfFontMetrics* fontMetrics = pFont->GetFontMetrics();
        if (!fontMetrics) {
            std::cout << "Error creating font metrics\n";
            delete docPodofo;
            return gpx2pdf::ERROR;
        }



        // set stroke pen to black and 1 unit width
        painter.SetStrokingColor(PoDoFo::PdfColor(0.0, 0.0, 0.0));
        painter.SetStrokeWidth(1.0);

        double pageHeight = pagePodofo->GetPageSize().GetHeight();
        double pageWidth = pagePodofo->GetPageSize().GetWidth();

        int waypointCount = 0;
        bool convertError = false;
        for (unsigned int i = 0; i < this->waypoints.size(); i++) {

            double x = 0, y = 0;
            if (convertCoordsToPixels(this->waypoints.at(i).lat, this->waypoints.at(i).lon, &x, &y) == g2pErr::SUCCESS) {

                // Convert the pixels to PDF units
                // This should be done using the DPI value but GDAL doesn't expose that value in their API
                // So we calculate DPI by doing page size / number of pixels
                x = x * pageWidth / static_cast<double>(this->xPixels);
                y = y * pageHeight / static_cast<double>(this->yPixels);

                // if the waypoint is on the PDF page
                if (x >= 0 && x <= pageWidth && y >= 0 && y <= pageHeight) {

                    // get width of the name rectangle
                    double textWidth = fontMetrics->StringWidth(this->waypoints.at(i).name.c_str());

                    // draw yellow rectangle
                    painter.SetColor(PoDoFo::PdfColor(1.0, 1.0, 0.0));
                    painter.Rectangle(x - textWidth / 2 - 2, pageHeight - y + 6, textWidth + 4, this->nameFontSize + 3);
                    painter.FillAndStroke();

                    // draw line below rectangle
                    painter.DrawLine(x, pageHeight - y + 6, x, pageHeight - y);

                    // draw circle on waypoint
                    painter.SetColor(PoDoFo::PdfColor(1.0, 1.0, 1.0));
                    painter.Circle(x, pageHeight - y, 3);
                    painter.FillAndStroke();

                    // draw cross in middle of the circle
                    painter.DrawLine(x, pageHeight - y + 2, x, pageHeight - y - 2);
                    painter.DrawLine(x + 2, pageHeight - y, x - 2, pageHeight - y);

                    // draw the name within the rectangle
                    painter.SetColor(PoDoFo::PdfColor(0.0, 0.0, 0.0));
                    painter.DrawMultiLineText(x - textWidth / 2 - 2, pageHeight - y + 6, textWidth + 4, this->nameFontSize + 2, PoDoFo::PdfString(this->waypoints.at(i).name), PoDoFo::ePdfAlignment_Center, PoDoFo::ePdfVerticalAlignment_Center);

                    waypointCount++;
                }
            } else {
                convertError = true;
            }

        }

        std::cout << waypointCount << " waypoint(s) added to PDF file\n";

        painter.FinishPage();

        if (convertError)
            std::cout << "Error converting waypoint coordinates.\n";

        if (waypointCount <= 0) {
            std::cout << "No waypoints are within the page limits. Output file not written.\n";
            delete docPodofo;
            return gpx2pdf::INVALID_ARGUMENT;
        }

    } catch(PoDoFo::PdfError& pdfError) {
        std::cout << "Error printing to PDF: " << pdfError.what() << "\n";
        delete docPodofo;
        return gpx2pdf::ERROR;
    } catch(...) {
        std::cout << "Error printing to PDF\n";
        delete docPodofo;
        return gpx2pdf::ERROR;
    }

    // Write the finished PDF to file
    try {
        docPodofo->Write(this->pdfFileOut.c_str());
    }catch(PoDoFo::PdfError& pdfError){
        std::cout << "Error writting PDF file: " << pdfError.what() << "\n";
        delete docPodofo;
        return gpx2pdf::ERROR;
    }

    delete docPodofo;
    return gpx2pdf::SUCCESS;
}

gpx2pdf::g2pErr gpx2pdf::getGeospatialData() {
    std::cout << "Extracting Geospatial Data from PDF file: " << this->pdfFileIn << "\n";

    GDALAllRegister();

    // Load PDF file with GDAL
    std::string optionStr = "USER_PWD=" + this->pdfPassword;
    const char* options[2] = {optionStr.c_str(), nullptr};
    GDALDataset *pdfDataset = static_cast<GDALDataset*>(GDALDataset::Open(this->pdfFileIn.c_str(), GA_ReadOnly, nullptr, this->pdfPassword.size() ? options : nullptr));
    if (!pdfDataset) {
        std::cout << "Unable to open PDF file for reading: " << this->pdfFileIn << "\n";
        return gpx2pdf::FILE_ERROR;
    }

    // Get a copy of the adfGeoTransform variable
    if (pdfDataset->GetGeoTransform(this->adfGeoTransform) == CE_None) {

        std::cout << std::fixed;
        std::cout << "Geospatial data found: Origin = (" << adfGeoTransform[0] << ", " << adfGeoTransform[3] << "), Pixel Size = (" << adfGeoTransform[1] << ", " << adfGeoTransform[5] << ")\n";

        // Get page size in pixels
        this->xPixels = pdfDataset->GetRasterXSize();
        this->yPixels = pdfDataset->GetRasterYSize();

        if (pdfDataset->GetSpatialRef()) {
            this->pdfSRS = pdfDataset->GetSpatialRef()->Clone();
            if (this->pdfSRS)
                this->coordTF = OGRCreateCoordinateTransformation(&this->WGS84, this->pdfSRS);
        } else {
            std::cout << "Error: null return from GetSpatialRef\n";
            GDALClose(pdfDataset);
            return gpx2pdf::ERROR;
        }

    } else {
        // adfGeoTransform is not set, so likely not a GeoPDF
        std::cout << "Geospatial data not found, are you sure this is a GeoPDF?\n";
        GDALClose(pdfDataset);
        return gpx2pdf::PARSE_ERROR;
    }

    GDALClose(pdfDataset);
    return gpx2pdf::SUCCESS;
}

void gpx2pdf::setPageNumber(int pageNumber) {
    this->pageNumber = pageNumber;
}

void gpx2pdf::setPdfPassword(std::string password) {
    this->pdfPassword = password;
}

void gpx2pdf::setUseGeocacheName(bool useGeocacheName) {
    this->useGeocacheName = useGeocacheName;
}

void gpx2pdf::setUseGsakSmartName(bool useGsakSmartName) {
    this->useGsakSmartName = useGsakSmartName;
}

void gpx2pdf::setMaxNameLength(int maxNameLength) {
    this->maxNameLength = maxNameLength;
}

void gpx2pdf::setNameFontSize(double nameFontSize) {
    this->nameFontSize = nameFontSize;
}

gpx2pdf::g2pErr gpx2pdf::convertCoordsToPixels(double lat, double lon, double *x, double *y) {
    // if there is no coordinate transformation loaded, then the conversion can not be done
    if (!this->coordTF)
        return gpx2pdf::ERROR;

    double x_i = lat;
    double y_i = lon;

    // convert to the format/datum that the GeoPDF uses
    if (!this->coordTF->Transform(1, &x_i, &y_i))
        return gpx2pdf::INVALID_ARGUMENT;

    // Convert from coordiates (UTM usually) to pixels on the PDF page
    // Inverse of this (from GDAL docs)
    // Xp = adfGeoTransform[0] + P*adfGeoTransform[1] + L*adfGeoTransform[2];
    // Yp = adfGeoTransform[3] + P*adfGeoTransform[4] + L*adfGeoTransform[5];

    double determinant = 1 / (this->adfGeoTransform[1] * this->adfGeoTransform[5] - this->adfGeoTransform[2] * this->adfGeoTransform[4]);

    *x = (((x_i - this->adfGeoTransform[0]) * this->adfGeoTransform[5]) + ((y_i - this->adfGeoTransform[3]) * -1 * adfGeoTransform[4])) * determinant;
    *y = (((x_i - this->adfGeoTransform[0]) * -1 * this->adfGeoTransform[2]) + ((y_i - this->adfGeoTransform[3]) * adfGeoTransform[1])) * determinant;

    return gpx2pdf::SUCCESS;
}
