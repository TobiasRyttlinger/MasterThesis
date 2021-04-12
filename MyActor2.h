// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/RuntimeMeshComponentStatic.h"
#include "RuntimeMeshComponent.h"
#include "Providers/RuntimeMeshProviderStatic.h"
#include "RuntimeMeshProvider.h"
#include "Kismet/GameplayStatics.h"
#include "ImageUtils.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "HAL/UnrealMemory.h"
#include "DDSLoader.h"


#include "MyActor2.generated.h"


UCLASS()
class EXJOBB2_API AMyActor2 : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMyActor2();

	float realtimeSeconds = UGameplayStatics::GetRealTimeSeconds(GetWorld());

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		URuntimeMeshProviderStatic* StaticProvider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		URuntimeMeshProvider* Provider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		URuntimeMeshComponentStatic* RMCS;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		URuntimeMeshComponent* RMC;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FVector> Vertices;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<int> Triangles;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FVector2D> TexCoords;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<UTexture2D*> TextureArray;

	double counter;

	template <typename ObjClass>
	static FORCEINLINE ObjClass* LoadObjFromPath(const FName& Path)
	{
		if (Path == NAME_None) return nullptr;

		return Cast<ObjClass>(StaticLoadObject(ObjClass::StaticClass(), nullptr, *Path.ToString()));
	}

	static FORCEINLINE UMaterialInstance* LoadMaterialFromPath(const FName& Path)
	{
		if (Path == NAME_None) return nullptr;

		return LoadObjFromPath<UMaterialInstance>(Path);
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterial* Mat = nullptr;

		UTexture2D* Texture = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterialInstanceDynamic* DynMat = nullptr;

	void LoadTextureFromPath(const FString& FullFilePath);

	TArray<double> CalculateHeightMap(UTexture2D* TexIn);

	void GenerateNodeMesh(URuntimeMeshProviderStatic* StaticProvider, FVector LocalUp, FVector Position, int i, int j);


	TArray<FVector2D> GenerateUVS(TArray<FVector2D> TexCoords, int Resolution);

	TArray<FVector> GenerateVertices(TArray<FVector>Vertices, int Resolution, FVector LocalUp, FVector Position, TArray<double> HeightMap);

	int GetIndexForGridCoordinates(int x, int y, int NoiseSamplesPerLine);

	FVector2D GetPositionForGridCoordinates(int x, int y, int NoiseResolution);

	TArray<int> GenerateTriangles(int NoiseSamplesPerLine, TArray<int>Triangles, int TriangleOffset);

	void ChangeMaterial(URuntimeMeshProviderStatic* StaticProvider, int TextureID);
};
