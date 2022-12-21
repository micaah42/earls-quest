// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <set>

#include "MeshFactory.h"
//#include "NetworkManager.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "ChunkWorld.generated.h"

class AChunk;

struct TChunkIndex {
	int i = 0;
	int j = 0;

	bool operator <(const TChunkIndex& other) const {
		if (i < other.i)
			return true;
		else
			return j < other.j;
	}
};


UCLASS()
class EARLSQUEST_API AChunkWorld : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AChunkWorld();

	UPROPERTY(EditAnywhere, Category = "ChunkWorld");
	TSubclassOf<AActor> ChunkType;

	UPROPERTY(EditInstanceOnly, Category = "ChunkWorld");
	TObjectPtr<UMaterialInterface> Material;

	// how many chunks around the player to load
	UPROPERTY(EditAnywhere, Category = "ChunkWorld");
	int DrawDistance = 5;

	// origin in lat/lon
	UPROPERTY(EditAnyWhere, Category = "ChunkWorld");
	double OriginLatidude = 47.267222;

	UPROPERTY(EditAnyWhere, Category = "ChunkWorld");
	double OriginLongitude = 11.392778;

	// original heightmap images to fetch
	UPROPERTY(EditAnywhere, Category = "ChunkWorld");
	int PixelSize = 5;

	UPROPERTY(EditAnywhere, Category = "ChunkWorld");
	int NumPixels = 100;

	// downsampling rules
	UPROPERTY(EditAnywhere, Category = "ChunkWorld");
	int Triangles = 100;

	UPROPERTY(EditAnywhere, Category = "ChunkWorld");
	float Clamping = 1000;




protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

private:
	FVector2D FetchChunk(TChunkIndex &ChunkIndex);
	void ProcessImageResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FVector2D ChunkPosition);
	void SpawnMeshActor(std::shared_ptr<TMeshData> MeshData, FVector2D ChunkPosition);

	std::unique_ptr<MeshFactory> Sampler;
	std::map<TChunkIndex, AChunk*> Loaded;
};
