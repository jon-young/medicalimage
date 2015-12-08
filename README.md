# medicalimage

## Introduction
The goal of this project is to reconstruct 3D liver models from MRI or CT imaging of patients with resectable hepatocellular carcinoma. In particular, the anatomic structures of interest include the blood vessels and tumor. 

Collected here are IPython notebooks and associated Python scripts to achieve this goal. The imaging data currently being analyzed is publicly available from [The Cancer Imaging Archive](http://www.cancerimagingarchive.net). 

## Prerequisites
Software dependencies include Python, SimpleITK, Matplotlib, NumPy, and SciPy.

## File descriptions
*Liver Interactive.ipynb* is an IPython notebook for slice-by-slice segmentation. 

*Liver Segmentation 3D.ipynb* is an IPython notebook attempting to perform segmentation in 3D, that is, on a series of MRI or CT slices simultaneously. 

*imgscroll.py* is a Python script for displaying image series with support for mouse scrolling through the series. 

## License
See [LICENSE](LICENSE)