// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/RuntimeMeshComponentStatic.h"
#include "Components/RuntimeMeshComponentStatic.h"
#include "Providers/RuntimeMeshProviderSphere.h"
#include "MyActor.generated.h"


UCLASS()
class EXJOBB2_API AMyActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMyActor();

	void FoliageSpawner();
	double rotation = 0;
	double PlanetSize = 636000000;

	UStaticMeshComponent* StaticMeshComponent;
	UPROPERTY()
	UStaticMesh* meshToUse;
	UPROPERTY()
	UInstancedStaticMeshComponent* InstancedStaticMeshComponent;
	UPROPERTY()
		TArray<UStaticMeshComponent*> MeshArray;
	UPROPERTY()
		TArray<FVector> RandomPosition;

	float counter;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
		USceneComponent* RootComp;



	UTexture2D* LoadTextureFromPath(const FString& FullFilePath);
	//TArray<UTexture2D> Textures;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
