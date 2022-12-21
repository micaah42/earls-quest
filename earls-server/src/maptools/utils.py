from osgeo import gdal


def open_geo_data():
    return gdal.Open(r'E:\maps\DGM_Tirol_5m_epsg31254_2006_2019.tiff'), 5  # 5 metres per pixel in this dataset


def assert_in_bounds(geo_data, x: int, y: int, size: int):
    assert 0 <= x <= x + size < geo_data.RasterXSize, f'out of bounds: 0 <= {x} <= {x + size} < {geo_data.RasterXSize}'
    assert 0 <= y <= y + size < geo_data.RasterYSize, f'out of bounds: 0 <= {y} <= {y + size} < {geo_data.RasterYSize}'
