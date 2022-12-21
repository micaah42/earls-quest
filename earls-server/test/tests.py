

import tempfile
import numpy as np
import open3d as o3d
from osgeo import gdal
from scipy import spatial

ds = gdal.Open(r'E:\coding\MapTest\Source\MapTest\resources\DGM_Tirol_5m_epsg31254_2006_2019.tiff')
print(f'projection: {ds.GetProjection()}')

raster_band = ds.GetRasterBand(1)
print(f'block size: {raster_band.GetBlockSize()}')

size = 256

x, y = np.meshgrid(5 * np.arange(size), 5 * np.arange(size))
z = raster_band.ReadAsArray(raster_band.XSize // 2, raster_band.YSize // 2, size, size)
xyz = np.asarray((x.reshape(-1), y.reshape(-1), z.reshape(-1))).transpose()
print(f'{xyz.shape=}')

points = o3d.utility.Vector3dVector(xyz)
print(f'{points=}')

cloud = o3d.geometry.PointCloud()
cloud.points = points
print(f'{cloud=}')

indices = spatial.Delaunay(xyz[:, :2]).simplices
indices = o3d.utility.Vector3iVector(indices)

triangle_mesh = o3d.geometry.TriangleMesh(points, indices)
triangle_mesh.compute_vertex_normals()
print(f'{triangle_mesh=}')

mesh_smp = triangle_mesh.simplify_quadric_decimation(target_number_of_triangles=6500)
print(f'{mesh_smp=}')

o3d.visualization.draw_geometries([mesh_smp])

