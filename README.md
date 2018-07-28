# AircraftsFinder
This project is used to recognize flying aircraft from Landsat data. This is the first time that aircrafts can be identified from an image with resolution no more than 30 m.

![alt text](https://github.com/xialang2012/AircraftsFinder/blob/master/global%20aircrafts.jpg)


## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

To maximum reducation dependences, we re-write some digital image pocessing methods in the project, e.g. Sobel filter, Laplance filter, which often get from OpenCV. Hence, the only dependence you need is GDAL and it is used to read GeoTIFF data and create vector file. What's more, the version of GDAL should be large than 2.0. You can install it under Ubuntu 16.04 like this. 

```
wget http://download.osgeo.org/gdal/2.1.0/gdal-2.1.0.tar.gz
tar -xvzf gdal-2.1.0.tar.gz
cd gdal-2.1.0/
./configure --prefix=/usr/
make
sudo make install
```

### Building

Install build-essential

```
sudo apt-get install build-essential
```

Install cmake

```
sudo apt install cmake
```
Building
```
cmake .
make
```

## Running the tests

For example, if you want to detect aircrafts from Mar. 31, 2013 to Apr. 31, 2013:

```
./AircraftFinder -outPath  ./data  -dateBegin 2013.03.31 -dateEnd 2013.04.31  -sceneFile ./ancData/sceneList/scene_list_pre
```
where -sceneFile is the file which describ all avaiable scenes of Landsat8. You can obtain this file from https://landsat-pds.s3.amazonaws.com/c1/L8/scene_list.gz. Besides, you need the vapor data to support the 2.1 micrometer low vapor test (optional for the non-AWS version).

## Example
This is the global aircrafts detected between 15-30 June 2016 from Landsat OLI images.

![alt text](https://github.com/xialang2012/AircraftsFinder/blob/master/global%20aircrafts.jpg)

Besides, we are producing the global aircrafts data on Amazon EC2, and you can view it online now: http://ec2-54-201-29-225.us-west-2.compute.amazonaws.com

## Algorithm
The detailed algorithm and theory can be found from our paper, https://www.sciencedirect.com/science/article/pii/S0924271618301345

## Acknowledgments

* We thank for USGS and AWS to provide data and calculation resource.
