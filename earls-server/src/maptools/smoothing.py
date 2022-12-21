import numpy as np
import scipy.signal as signal
import scipy.interpolate as interpolate


def convolve(array: np.ndarray, kernel_size: int):
    kernel = np.ones((kernel_size, kernel_size)) / (kernel_size ** 2)
    return signal.convolve2d(array, kernel, 'same', 'symm')


def simplify(array: np.ndarray, down: int = 2):
    iv, jv = np.meshgrid(np.arange(0, array.shape[0], down, dtype=int),
                         np.arange(0, array.shape[1], down, dtype=int))
    return array[iv, jv]