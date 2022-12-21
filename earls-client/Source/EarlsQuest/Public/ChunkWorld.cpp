// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkWorld.h"
#include "Chunk.h"
#include "MeshData.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AChunkWorld::AChunkWorld() {
	// Set this actor to call Tick() every frame.  You can turn this off to
	// improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AChunkWorld::BeginPlay() {
	Super::BeginPlay();
	Sampler.reset(new MeshFactory(NumPixels, PixelSize));
}

void AChunkWorld::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	FVector Location = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	TChunkIndex CameraChunkIndex{ Location.X / (NumPixels * PixelSize), Location.Y / (NumPixels * PixelSize) };
	UE_LOG(LogTemp, Warning, TEXT("camera chunk: (%d, %d)"), CameraChunkIndex.i, CameraChunkIndex.j);

	std::set<TChunkIndex> InBoxChunks;
	for (int i = -DrawDistance; i < DrawDistance; ++i) {
		for (int j = -DrawDistance; j < DrawDistance; ++j) {
			TChunkIndex ChunkIndex{ i + CameraChunkIndex.i, j + CameraChunkIndex.j };
			
			InBoxChunks.insert(ChunkIndex);
			
			auto ChunkPosition = FetchChunk(ChunkIndex);
			Loaded.insert({ ChunkIndex, nullptr });
		}
	}

	for (auto const& item : Loaded) {
		if (InBoxChunks.find(item.first) == InBoxChunks.end()) {
			// delete item if it is loaded but not contained in the box
			// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("deleting: [%f, %f]"), item.first.X, item.first.Y));
		}
	}
}

FVector2D AChunkWorld::FetchChunk(TChunkIndex& ChunkIndex)
{
	FVector2D ChunkPosition{ static_cast<double>(ChunkIndex.i * PixelSize * (NumPixels - 1)), static_cast<double>(ChunkIndex.j * PixelSize * (NumPixels - 1)) };

	if (Loaded.find(ChunkIndex) != Loaded.end()) {
		return ChunkPosition;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("fetching: [%f, %f]"), ChunkPosition.X, ChunkPosition.Y));


	auto RequestUrl = FString::Printf(
		TEXT("http://localhost:5000/height?lat=%f&long=%f&x=%f&y=%f&size=%d&pixel_size=%d"),
		OriginLatidude,
		OriginLongitude,
		ChunkPosition.X,
		-ChunkPosition.Y,
		NumPixels,
		PixelSize
	);

	auto HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(RequestUrl);
	HttpRequest->SetVerb("GET");
	HttpRequest->OnProcessRequestComplete().BindLambda([this, ChunkPosition](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
		Async(EAsyncExecution::Thread, [=]() {AChunkWorld::ProcessImageResponse(Request, Response, bWasSuccessful, ChunkPosition); });
	});
	HttpRequest->ProcessRequest();

	return ChunkPosition;
}

void AChunkWorld::ProcessImageResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FVector2D ChunkPosition)
{
	if (!bWasSuccessful) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("request failed: %d"), Response->GetResponseCode()));
		return;
	}

	auto Image = open3d::io::CreateImageFromMemory("png", Response->GetContent().GetData(), Response->GetContentLength());
	if (Image->width_ != NumPixels || Image->height_ != NumPixels) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("invalid image size: %dx%d"), Image->width_, Image->height_));
		return;
	}

	auto MeshData = Sampler->Meshify(*Image, Triangles, Clamping);

	Async(EAsyncExecution::TaskGraphMainThread, [this, MeshData, ChunkPosition]() {
		AChunkWorld::SpawnMeshActor(MeshData, ChunkPosition);
	});
}

void AChunkWorld::SpawnMeshActor(std::shared_ptr<TMeshData> MeshData, FVector2D ChunkPosition)
{
	auto Transform = FTransform(
		FRotator::ZeroRotator,
		FVector{ ChunkPosition.X, ChunkPosition.Y, 0 },
		FVector::OneVector
	);

	auto const Chunk = GetWorld()->SpawnActorDeferred<AChunk>(ChunkType, Transform, this);
	Chunk->Material = Material;

	Chunk->SetMesh(*MeshData);
	UGameplayStatics::FinishSpawningActor(Chunk, Transform);
	auto Message = FString::Printf(TEXT("spawned chunk: %d, %d, vertices: %d"), ChunkPosition.X, ChunkPosition.Y, MeshData->Vertices.Num());
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, Message);
}

#if 0
void AChunkWorld::FillChunk(TChunkIndex ChunkIndex)
{
	auto Shift = FVector(ChunkIndex.i * (GridPixels - 1) * PixelSize, ChunkIndex.j * (GridPixels - 1) * PixelSize, 0);
	auto Transform = FTransform(FRotator::ZeroRotator, Shift, FVector::OneVector);

	auto const Chunk = GetWorld()->SpawnActorDeferred<AChunk>(ChunkType, Transform, this);
	Chunk->Material = Material;

	if (Chunk == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("failed to spawn chunk"));
		return;
	}

	auto RequestUrl = FString::Printf(TEXT("http://localhost:5000/height?x=%d&y=%d&size=%d"), ChunkIndex.i * GridPixels, ChunkIndex.j * GridPixels, GridPixels);

	auto HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(RequestUrl);
	HttpRequest->SetVerb("GET");

	HttpRequest->OnProcessRequestComplete().BindLambda([=](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {

		if (!bWasSuccessful) {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("failed to spawn chunk: %d, %d"), ChunkIndex.i, ChunkIndex.j));
			return;
		}

	auto Image = open3d::io::CreateImageFromMemory("png", Response->GetContent().GetData(), Response->GetContentLength());
	if (Image->width_ != GridPixels || Image->height_ != GridPixels) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("invalid image size: %dx%d"), Image->width_, Image->height_));
		return;
	}

	auto Result = Async(EAsyncExecution::Thread, [=]() {
		auto MeshData = Sampler->Meshify(*Image, Triangles, Clamping);
	Async(EAsyncExecution::TaskGraphMainThread, [=]() {
		Chunk->SetMesh(*MeshData);
	UGameplayStatics::FinishSpawningActor(Chunk, Transform);
	Loaded.insert({ ChunkIndex, Chunk });
	auto Message = FString::Printf(TEXT("spawned chunk: %d, %d, vertices: %d"), ChunkIndex.i, ChunkIndex.j, MeshData.Vertices.Num());
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, Message);
	}); });

	});

	HttpRequest->ProcessRequest();
}
#endif