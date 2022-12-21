import argparse
import datetime
import glob
import hashlib
import logging
import os

import osgeo.gdal
import pandas as pd

from maptools import geo_transform


def scan_directory(directory: str, dry: False):
    collection = []
    files = glob.glob(os.path.join(directory, '**'), recursive=True)
    files = filter(lambda x: x.endswith('.tif') or x.endswith('.tiff'), files)
    print(files)

    for file_path in files:
        geo_data = osgeo.gdal.Open(file_path)
        if geo_data is None:
            logging.warning(f'failed to open: {file_path} as geo dataset')
            continue
        else:
            print(f'- {file_path:<39}:')
            print(f'  - {geo_data.GetProjection()}')
            print(f'  - {geo_data.GetGeoTransform()}')

        converter = geo_transform.Converter(geo_data)
        north, west = converter.pixel2polar(0, 0)
        south, east = converter.pixel2polar(geo_data.RasterXSize, geo_data.RasterYSize)
        collection.append({
            'north': north, 'west': west, 'south': south, 'east': east,
            'res_x': converter.geo_matrix[1], 'res_y': -converter.geo_matrix[-1],
            'file_path': file_path
        })

    collection = pd.DataFrame(collection)
    markdown = collection.to_markdown()
    print('\n', ''.center(markdown.index('\n') - 2, '='))
    print(markdown)
    print(''.center(markdown.index('\n'), '-'))

    if not dry:
        os.makedirs('resources', exist_ok=True)
        meta_file_path = os.path.join('resources', f'meta_data_{datetime.datetime.now():%Y%m%d%H%M}')
        collection.to_csv(meta_file_path, index=False)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--dry', action='store_true')
    parser.add_argument('directory', type=str)
    args = parser.parse_args()
    scan_directory(**vars(args))


if __name__ == '__main__':
    main()
