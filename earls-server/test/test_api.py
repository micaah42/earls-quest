import io

import numpy as np
import requests
from PIL import Image

HOST = 'http://localhost:5000'


def test_alive():
    test_json = {'number': 2112, 'string': 'rush', 'array': ['rush', 2112]}
    resp = requests.get(f'{HOST}/echo', json=test_json)
    assert resp.status_code == 200
    assert test_json == resp.json()


def test_height_api():
    params = {
        'lat': 47.267222,
        'long': 11.392778,
        'x': 0.,
        'y': 0,
        'size': 64,
        'pixel_size': 10
    }

    resp = requests.get('http://localhost:5000/height', params=params)
    assert resp.status_code == 200

    params = {
        'lat': 48.728809822244294,
        'long': 13.887979103084659,
        'x': 0.,
        'y': 0,
        'size': 32,
        'pixel_size': 10
    }

    resp = requests.get('http://localhost:5000/height', params=params)

    # parse
    png = io.BytesIO(resp.content)
    img = Image.open(png)
    assert img.size == (params['size'], params['size']), f'{img.size=}'

    # check content
    array = np.asarray(img)


def test_transforms():
    from maptools import geo_transform, utils
    ds, mpp = utils.open_geo_data()

    innsbruck = (47.267222, 11.392778)
    i, j = geo_transform.polar2pixel(*innsbruck, 0, 0, ds)  # coordinates off innsbruck, should be in map
    print(f'{(i, j)=}')
    utils.assert_in_bounds(ds, i, j, 1)

    recovered_innsbruck = geo_transform.pixel2polar(j, i, 0, 0, ds)
    distance = np.linalg.norm(np.asarray(recovered_innsbruck) - np.asarray(innsbruck))
    print(f'{innsbruck=}, {recovered_innsbruck=}, ~{distance:.10f} degree error')


def test_converter_transforms():
    from maptools import geo_transform, utils
    ds, mpp = utils.open_geo_data()

    converter = geo_transform.Converter(ds)

    innsbruck = (47.267222, 11.392778)
    i, j = converter.polar2pixel(*innsbruck, 0, 0)  # coordinates off innsbruck, should be in map
    print(f'{(i, j)=}')
    utils.assert_in_bounds(ds, i, j, 1)

    recovered_innsbruck = converter.pixel2polar(j, i, 0, 0)
    distance = np.linalg.norm(np.asarray(recovered_innsbruck) - np.asarray(innsbruck))
    print(f'{innsbruck=}, {recovered_innsbruck=}, ~{distance:.10f} degree error')
