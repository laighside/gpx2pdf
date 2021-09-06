# gpx2pdf
A simple and easy to use tool for plotting waypoints on GeoPDF maps. Featuring both a command line interface and a GUI. Two input files are required, a GPX file with the waypoints to plot and a GeoPDF file with the map.

### Usage
To use the GUI version, run the application with no arguments
```
./gpx2pdf
```
To use the command line version, three arguments are required
```
./gpx2pdf gpx_file pdf_file_in pdf_file_out
```

### Building
Dependencies:
* The Qt Libraries are used for the GUI and the XML parser.
* [GDAL](https://gdal.org/) is used for reading the geospatial data in GeoPDF files, and performing the datum conversions on waypoints.
* [PoDoFo](http://podofo.sourceforge.net/) is used for reading/writing PDF files.
