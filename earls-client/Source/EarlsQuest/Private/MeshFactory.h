// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <open3d/Open3D.h>
#include "CoreMinimal.h"

#include <MeshData.h>

class MeshFactory
{
public:
	MeshFactory(int size, float scale);
	~MeshFactory();


	std::shared_ptr<TMeshData> Meshify(const open3d::geometry::Image& img, int triangles, float clamping);

private:
	// utilities
	int index(int row, int col);

	// settings
	int _size;
	float _scale;

	// caching for efficiency
	std::vector<Eigen::Vector3i> _triangles;
};
