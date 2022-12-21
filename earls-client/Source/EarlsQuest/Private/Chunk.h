// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <open3d/Open3D.h>

#include "Http.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MeshData.h"

#include "Chunk.generated.h"

class TriangleLookup;

class UProceduralMeshComponent;

UCLASS()
class AChunk : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AChunk();

	TObjectPtr<UMaterialInterface> Material;
	void SetMesh(const TMeshData& MeshData);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	TObjectPtr<UProceduralMeshComponent> Mesh;
};
