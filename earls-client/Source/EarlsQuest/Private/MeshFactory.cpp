// Fill out your copyright notice in the Description page of Project Settings.

#include "MeshFactory.h"



MeshFactory::MeshFactory(int size, float scale) : _size(size), _scale(scale)
{
	_triangles.reserve(_size * _size);

	for (int i = 1; i < _size; i++) 
		for (int j = 1; j < _size; j++) 
			_triangles.push_back({ index(i, j) , index(i, j - 1), index(i - 1, j - 1) });

	for (int i = 1; i < _size; i++)
		for (int j = 1; j < _size; j++) 
			_triangles.push_back({ index(i - 1, j - 1), index(i - 1, j), index(i, j) });
}

MeshFactory::~MeshFactory()
{
}

std::shared_ptr<TMeshData> MeshFactory::Meshify(const open3d::geometry::Image &img, int triangles, float clamping)
{
	// create input mesh from image
	std::vector<Eigen::Vector3d> Vertices(_triangles.size());
	for (int i = 0; i < _size; ++i)
		for (int j = 0; j < _size; ++j)
			Vertices[index(i, j)] = Eigen::Vector3d(_scale * i, _scale * j, *img.PointerAt<uint16>(i, j));

	open3d::geometry::TriangleMesh InMesh(Vertices, _triangles);
	
	// do downsampling and computations
	auto OutMesh = InMesh.SimplifyQuadricDecimation(triangles, INFINITY, clamping);
	OutMesh->ComputeVertexNormals(true);

	// convert to unreal engine types
	auto MeshData = std::make_shared<TMeshData>();
	
	MeshData->Vertices.Reserve(OutMesh->vertices_.size());
	for (auto const& v : OutMesh->vertices_)
		MeshData->Vertices.Push(FVector{ v.x(), v.y(), v.z() });

	MeshData->Triangles.Reserve(3 * OutMesh->triangles_.size());
	for (auto const& t : OutMesh->triangles_)
		MeshData->Triangles.Append({ t.x(), t.y(), t.z() });

	//MeshData->Normals.Reserve(OutMesh->vertex_normals_.size());
	//for (auto const& v : OutMesh->vertex_normals_)
	//	MeshData->Normals.Push(FVector{ v.x(), v.y(), v.z() });

	
	// create vertex colors
	static TArray<FColor> PALETTE = {
		FColor{11, 45, 0},
		FColor{27, 44, 40},
		FColor{37, 40, 35},
		FColor{38, 38, 38}
	};

	MeshData->Colors.Reserve(OutMesh->vertex_normals_.size());
	for (auto const& v : OutMesh->vertex_normals_) {
		MeshData->Colors.Push(PALETTE[min(PALETTE.Num() - 1, 10 * (v.x() * v.x() + v.y() * v.y()))]);
	}

	return MeshData;
}

int MeshFactory::index(int row, int col)
{
	return row * _size + col;
}
