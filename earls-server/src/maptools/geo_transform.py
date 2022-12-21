import numpy as np
from osgeo import gdal, osr


def pixel2polar(i: int, j: int, x_metres: float, y_metres: float, ds: gdal.Dataset):
    geo_matrix = ds.GetGeoTransform()
    x_geo, y_geo = (j * geo_matrix[1]) + geo_matrix[0], (i * geo_matrix[5]) + geo_matrix[3]
    x_geo, y_geo = x_geo + x_metres, y_geo + y_metres

    src = osr.SpatialReference()
    src.ImportFromWkt(ds.GetProjection())

    dst = osr.SpatialReference()
    dst.SetWellKnownGeogCS('WGS84')

    ct = osr.CoordinateTransformation(src, dst)
    (latitude, longitude, height) = ct.TransformPoint(y_geo, x_geo)
    return latitude, longitude


def polar2pixel(latitude: float, longitude: float, x_metres: float, y_metres: float, ds: gdal.Dataset):
    src = osr.SpatialReference()
    src.SetWellKnownGeogCS('WGS84')

    dst = osr.SpatialReference()
    dst.ImportFromWkt(ds.GetProjection())

    ct = osr.CoordinateTransformation(src, dst)
    y_geo, x_geo, _ = ct.TransformPoint(latitude, longitude)
    x_geo, y_geo = x_geo + x_metres, y_geo + y_metres

    geo_matrix = ds.GetGeoTransform()
    inv_geo_matrix = gdal.InvGeoTransform(geo_matrix)

    i = int(inv_geo_matrix[0] + inv_geo_matrix[1] * x_geo + inv_geo_matrix[2] * y_geo)
    j = int(inv_geo_matrix[3] + inv_geo_matrix[4] * x_geo + inv_geo_matrix[5] * y_geo)
    return i, j


class Converter:
    def __init__(self, geo_dataset: gdal.Dataset):
        self.src = osr.SpatialReference()
        self.src.SetWellKnownGeogCS('WGS84')

        self.dst = osr.SpatialReference()
        self.dst.ImportFromWkt(geo_dataset.GetProjection())

        self.polar2pixel_transform = osr.CoordinateTransformation(self.src, self.dst)
        self.pixel2polar_transform = osr.CoordinateTransformation(self.dst, self.src)

        self.geo_matrix = geo_dataset.GetGeoTransform()
        self.inv_geo_matrix = gdal.InvGeoTransform(self.geo_matrix)

    def polar2pixel(self, latitude: float, longitude: float, x_metres: float = 0, y_metres: float = 0):
        # project into localized geo projection in metres
        y_geo, x_geo, height = self.polar2pixel_transform.TransformPoint(latitude, longitude)

        # apply a shift in metres
        x_geo, y_geo = x_geo + x_metres, y_geo + y_metres

        # convert metres in projection into matrix pixel positions
        i = int(self.inv_geo_matrix[0] + self.inv_geo_matrix[1] * x_geo + self.inv_geo_matrix[2] * y_geo)
        j = int(self.inv_geo_matrix[3] + self.inv_geo_matrix[4] * x_geo + self.inv_geo_matrix[5] * y_geo)
        return i, j

    def pixel2polar(self, i: int, j: int, x_metres: float = 0, y_metres: float = 0):
        # convert pixels in image to geo projection metres
        x_geo, y_geo = (j * self.geo_matrix[1]) + self.geo_matrix[0], (i * self.geo_matrix[5]) + self.geo_matrix[3]

        # apply a shift in metres
        x_geo, y_geo = x_geo + x_metres, y_geo + y_metres

        # convert localized geo projection into international latitude, longitude
        latitude, longitude, height = self.pixel2polar_transform.TransformPoint(y_geo, x_geo)
        return latitude, longitude
