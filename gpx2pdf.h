/**
  @file    gpx2pdf.h
  @author  Ben <admin@laighside.com>
  @version 1.0

  @section DESCRIPTION
  A class that reads waypoints from a GPX file and places them on a map from a GeoPDF file
  The status is outputed to std::cout as the data is processed
 */

#ifndef GPX2PDF_H
#define GPX2PDF_H

#include <string>
#include <vector>

#include <ogr_spatialref.h>

class gpx2pdf
{
public:

    /**
      Constructer for gpx2pdf class.

      @param gpxFile is the GPX file with the waypoints to put on the PDF file. Read permissions for this file are required.
      @param pdfFileIn is the GeoPDF file that contains the map to put the waypoints on. This file is not modified. Read permissions for this file are required.
      @param pdfFileOut is where the PDF file with the waypoints is written to. Write permissions for this file are required.
    */
    gpx2pdf(std::string gpxFile, std::string pdfFileIn, std::string pdfFileOut);

    /**
      Destructor for gpx2pdf class.
    */
    ~gpx2pdf();

    // Errors
    typedef enum
    {
        SUCCESS = 0,
        ERROR = 1,
        EMPTY_DATA = 2,
        INVALID_ARGUMENT = 3,
        FILE_ERROR = 4,
        PARSE_ERROR = 5
    } g2pErr;

    /**
      Do the conversion, and save to file if successful.

      A convenience function that performs all the conversions steps at once.
      Equivalent to calling loadGPX(), getGeospatialData(), savePdf() in order.

      @return SUCCESS if all steps are successful, and error code otherwise.
    */
    g2pErr doConversion();

    /**
      Convert from GPX to PDF, and save to file if successful.

      A convenience function that performs the conversion without an object.

      @param gpxFile is the GPX file with the waypoints to put on the PDF file. Read permissions for this file are required.
      @param pdfFileIn is the GeoPDF file that contains the map to put the waypoints on. This file is not modified. Read permissions for this file are required.
      @param pdfFileOut is where the PDF file with the waypoints is written to. Write permissions for this file are required.
      @return SUCCESS if all steps are successful, and error code otherwise.
    */
    static g2pErr doConversion(std::string gpxFile, std::string pdfFileIn, std::string pdfFileOut);

    /**
      Load the data from the GPX file.

      Reads the GPX file, parses the XML and extracts the waypoint coordinates.

      @return SUCCESS if the waypoints are loaded successfully, and error code otherwise.
    */
    g2pErr loadGpx();

    /**
      Get the geospatial data from the GeoPDF file.

      Reads the PDF file and extracts the geospatial data.

      @return SUCCESS if the geospatial data is found, and error code otherwise.
    */
    g2pErr getGeospatialData();

    /**
      Creates and saves the PDF file with waypoints.

      Reads the existing PDF file, overlays the waypoints on it then saves the result to a new PDF file.

      @return SUCCESS if the new file is created successfully, and error code otherwise.
    */
    g2pErr savePdf();

    /**
      Sets the page number to use for PDFs with more than one page.

      @param pageNumber is the number of page to use.
    */
    void setPageNumber(int pageNumber);

    /**
      Sets the password in the case that an encrypted PDF is used.

      @param password is the password for the PDF.
    */
    void setPdfPassword(std::string password);

    /**
      Sets whether to use the geocache name if it available in the GPX file.

      @param useGeocacheName is set to true to use the Geocache name if available.
    */
    void setUseGeocacheName(bool useGeocacheName);

    /**
      Sets whether to use the GSAK smart name if it available in the GPX file.

      @param useGsakSmartName is set to true to use the GSAK smart name if available.
    */
    void setUseGsakSmartName(bool useGsakSmartName);

    /**
      Sets the maximum length for the names printed on the map.

      @param maxNameLength is the maximum name length (set to -1 for no maximum).
    */
    void setMaxNameLength(int maxNameLength);

    /**
      Sets the font size for printing waypoint names on the map.

      @param nameFontSize is the font size.
    */
    void setNameFontSize(double nameFontSize);

private:
    /**
      An object to store a waypoint in.
    */
    struct waypoint {
        double lat;
        double lon;
        std::string name;
    };

    /**
      Converts lat/lon coordinates to pixel coordinates.

      A coordinate transform must be loaded for this to work.

      @param lat is the latitude (WGS84, decimal degrees).
      @param lon is the longitude (WGS84, decimal degrees).
      @param x is a pointer to a variable where the resulting x pixel coordinate will be placed.
      @param y is a pointer to a variable where the resulting y pixel coordinate will be placed.
      @return SUCCESS if the coordinate conversion was successful, and error code otherwise.
    */
    g2pErr convertCoordsToPixels(double lat, double lon, double *x, double *y);

    std::string gpxFile;               /*!< Stores the GPX file path */
    std::string pdfFileIn;             /*!< Stores the input PDF file path */
    std::string pdfFileOut;            /*!< Stores the output PDF file path */
    int pageNumber;                    /*!< Stores the page number */
    std::string pdfPassword;           /*!< Password for encrypted PDFs */
    bool useGeocacheName;              /*!< Use Geocache name instead of waypoint name if it is available */
    bool useGsakSmartName;             /*!< Use GSAK smart name instead of waypoint name if it is available */
    int maxNameLength;                 /*!< Max length of name to print on the map, any characters after this length are ignored (set to -1 for no limit) */
    double nameFontSize;               /*!< Font size to use when printing waypoint names */

    std::vector<waypoint> waypoints;   /*!< Vector to store the waypoints after reading them from file */

    OGRSpatialReference* pdfSRS;
    OGRSpatialReference WGS84;
    OGRCoordinateTransformation *coordTF;
    double adfGeoTransform[6];

    int xPixels;                       /*!< Width of the PDF page in pixels, comes from GDAL */
    int yPixels;                       /*!< Height of the PDF page in pixels, comes from GDAL */

};

#endif // GPX2PDF_H
