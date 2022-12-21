import io
import json
import logging
import math
import os.path

import numpy as np
import osgeo.gdal
import pandas as pd
from PIL import Image
from flask import Flask, request, jsonify, render_template

from maptools import utils, geo_transform


class CustomJsonEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, (np.ndarray, np.number)):
            return obj.tolist()
        elif isinstance(obj, (complex, np.complex)):
            return [obj.real, obj.imag]
        elif isinstance(obj, set):
            return list(obj)
        elif isinstance(obj, bytes):  # pragma: py3
            return obj.decode()
        return json.JSONEncoder.default(self, obj)


# init app
app = Flask(__name__)
app.json_encoder = CustomJsonEncoder


# -------------------------------------------------------------------------------------------------------------------- #
@app.route("/")
def hello_world():
    return render_template('index.html')


@app.route('/echo')
def echo():
    args = request.get_json()
    logging.info(args)
    return args


@app.route('/height')
def height():
    x = request.args.get('x', type=float)
    y = request.args.get('y', type=float)
    latitude = request.args.get('lat', type=float)
    longitude = request.args.get('long', type=float)
    size = request.args.get('size', type=int)
    pixel_size = request.args.get('pixel_size', type=int)  # target image metres per pixel

    # find a suitable dataset
    available = pd.read_csv(os.path.join('resources', max(os.listdir('resources'))))

    # create a mask of datasets containing the given latitude/longitude
    latitude_mask = (available['south'] < latitude) & (latitude < available['north'])
    longitude_mask = (available['west'] < longitude) & (longitude < available['east'])
    suitable = available[latitude_mask & longitude_mask]

    # sort by closest to the wanted resolution
    res_diff = suitable[['res_x', 'res_y']].mean(axis=1)
    suitable = suitable.iloc[res_diff.argmin()]

    geo_data = osgeo.gdal.Open(suitable['file_path'])
    metres_per_pixel = geo_data.GetGeoTransform()[1]

    # get the image section to read
    i, j = geo_transform.polar2pixel(latitude, longitude, x, y, geo_data)
    read_size = math.ceil((pixel_size * size) / metres_per_pixel)  # size * pixel_size == read_size * metres_per_pixel
    utils.assert_in_bounds(geo_data, i, j, read_size)

    # to image
    array = geo_data.ReadAsArray(i, j, read_size, read_size)
    array = array.astype(np.uint32)
    image = Image.fromarray(array, 'I')
    image = image.resize((size, size))

    # serialize
    tiff = io.BytesIO()
    image.save(tiff, 'png')
    tiff.seek(0)

    return tiff.read()


if __name__ == '__main__':
    app.run()
