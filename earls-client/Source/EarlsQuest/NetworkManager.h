// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Async/Future.h"

class FHttpModule;

class NetworkManager
{
public:
	NetworkManager();

	// --- --- --- --- --- --- --- --- --- --- --- --- ---
	TFuture<TArray<uint8>> Get(const FString& url, const FString& content);


private:
	FHttpModule& Module;
};
