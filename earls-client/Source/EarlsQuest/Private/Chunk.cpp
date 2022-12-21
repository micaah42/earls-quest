// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunk.h"

#include "Json.h"
#include "HttpModule.h"
#include "ProceduralMeshComponent.h"

// Sets default values
AChunk::AChunk()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");
	Mesh->SetCastShadow(false);
	SetRootComponent(Mesh);
}

void AChunk::SetMesh(const TMeshData &MeshData)
{

	Mesh->SetMaterial(0, Material);
	Mesh->CreateMeshSection(
		0,
		MeshData.Vertices,
		MeshData.Triangles,
		MeshData.Normals,
		{},
		MeshData.Colors,
		{},
		true
	);
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Super::BeginPlay();
}
