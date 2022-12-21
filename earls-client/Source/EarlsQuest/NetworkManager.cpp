// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkManager.h"

#include "Http.h"
#include "HttpModule.h"

NetworkManager::NetworkManager() : Module{ FHttpModule::Get() }
{}

TFuture<TArray<uint8>> NetworkManager::Get(const FString& Url, const FString& Content)
{
	auto HttpRequest = Module.CreateRequest();
	HttpRequest->SetVerb("GET");

	HttpRequest->SetURL(Url);
	HttpRequest->SetHeader("User-Agent", "EarlAgent");
	HttpRequest->SetHeader("Content-Type", "application/json");

	if (!HttpRequest->ProcessRequest()) {
		UE_LOG(LogTemp, Error, TEXT("Failed to start HTTP Request."));
		return TFuture<TArray<uint8>>();
	}

	return Async(EAsyncExecution::ThreadPool, [HttpRequest]() {
		TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response = HttpRequest->GetResponse();
		while (Response->GetResponseCode() == 0) {
			FPlatformProcess::Sleep(0.05);
			Response = HttpRequest->GetResponse();
		}
		return Response->GetContent();
	});
}