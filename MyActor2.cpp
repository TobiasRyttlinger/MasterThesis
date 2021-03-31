// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor2.h"
#include <Runtime/Engine/Public/DDSLoader.h>
#include <AssetRegistryModule.h>

// Sets default values
AMyActor2::AMyActor2()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}


// Called when the game starts or when spawned
void AMyActor2::BeginPlay()
{
	Super::BeginPlay();

	FString MatPath = "Material'/Game/simplex_Mat.simplex_Mat'";

	Mat = LoadObjFromPath<UMaterial>(FName(MatPath));
	DynMat = UMaterialInstanceDynamic::Create(Mat, this);

	counter = 0;
	FVector localUp = this->GetActorUpVector();
	FVector Position = FVector(0, 0, 0);
	StaticProvider = NewObject<URuntimeMeshProviderStatic>(this);
	int SectionID = 0;
	int TextureID = 0;
	for (float y = 0; y < 10; y++) {
		for (float x = 0; x < 10; x++) {
			if (FMath::Fmod(x, 2.0f) == 0 && FMath::Fmod(y, 2.0f) == 0) {
				Position = FVector(x, y, 0);

				GenerateNodeMesh(StaticProvider, localUp, Position, SectionID, TextureID);
				SectionID++;
			}
		}
	}
}

static bool isInit = false;

// Called every frame
void AMyActor2::Tick(float DeltaTime)
{

	int rand = FMath::RandRange(0, 39);
	FString IntAsString = FString::FromInt(rand);
	FString sPath = "C:/Users/Admin/Downloads/Tiles/" + IntAsString + ".png";
	FString newPath = "Texture2D'/Game/simplex.simplex'";



	if (counter > 0.0f) {
		LoadTextureFromPath(sPath);
		DynMat->SetTextureParameterValue(FName("Texture"), Texture);
		for (int i = 0; i < 25; i++) {
			StaticProvider->SetupMaterialSlot(i, TEXT("Material"), DynMat);
		}

		counter = 0;
	}

	GEngine->ForceGarbageCollection(true);


	isInit = true;
	counter += DeltaTime;
	Super::Tick(DeltaTime);

}

TArray<double> AMyActor2::CalculateHeightMap(UTexture2D* TexIn) {

	FTexture2DMipMap* MyMipMap = &TexIn->PlatformData->Mips[0];
	TexIn->SRGB = false;
	TexIn->UpdateResource();

	const FColor* FormatedImageData = static_cast<const FColor*>(TexIn->PlatformData->Mips[0].BulkData.LockReadOnly());
	TArray<double> HeightMap;
	FColor tempColor;

	for (int x = 0; x < MyMipMap->SizeX; x++) {
		for (int y = 0; y < MyMipMap->SizeY; y++) {

			tempColor = FormatedImageData[(y * MyMipMap->SizeX) + x];

			double Value = (((double)tempColor.R / 255.0) + ((double)tempColor.G / 255.0) + ((double)tempColor.B / 255.0)) / 3.0;
			HeightMap.Add(Value);
		}
	}

	TexIn->PlatformData->Mips[0].BulkData.Unlock();
	return HeightMap;
}

void AMyActor2::LoadTextureFromPath(const FString& FullFilePath)
{

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	//Load From File
	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *FullFilePath))
	{
		Texture = nullptr;
	}

	//Create T2D!
	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		TArray<uint8> UncompressedRGBA = {};
		if (ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, UncompressedRGBA))
		{
			Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_R8G8B8A8);
			/*	Texture->MipGenSettings = TMGS_LeaveExistingMips;
				Texture->PlatformData->SetNumSlices(0);
				Texture->NeverStream = false;*/

				//Valid?
			if (!Texture)
			{
				Texture = nullptr;
			}

			//Copy! std::Swap();
			void* TextureData = Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedRGBA.GetData(), UncompressedRGBA.Num());
			Texture->PlatformData->Mips[0].BulkData.Unlock();

			//Update!
			Texture->UpdateResource();
		}
		UncompressedRGBA.Empty();
	}
	RawFileData.Empty();


}


void AMyActor2::GenerateNodeMesh(URuntimeMeshProviderStatic* StaticProviderIn, FVector localUp, FVector Position, int SectionID, int TextureID) {


	RMCS = NewObject<URuntimeMeshComponentStatic>(this);
	RMCS->RegisterComponent();
	RMCS->Initialize(StaticProviderIn);

	int NoiseSamplesPerLine = 8;
	double NoiseInputScale = 0.01; // Making this smaller will "stretch" the perlin noise terrain
	double NoiseOutputScale = 2000; // Making this bigger will scale the terrain's height

	TArray<uint8> RawFileData = {};

	FString TexturePath = "Texture2D'/Game/K4242F.K4242F'";
	FString sPath = "Material'/Game/Elevation/10N000E_20101117_gmted_mea300_Mat.10N000E_20101117_gmted_mea300_Mat'";
	FString LocalPath = "C:/Users/Admin/Downloads/Tiles/1.png";

	LoadTextureFromPath(LocalPath);

	TArray<double> HeightMap = CalculateHeightMap(Texture);


	Vertices = GenerateVertices(Vertices, NoiseSamplesPerLine, localUp, Position, HeightMap);
	Triangles = GenerateTriangles(NoiseSamplesPerLine, Triangles, 0);
	TexCoords = GenerateUVS(TexCoords, NoiseSamplesPerLine);

	TArray<FColor> Colors;
	Colors.Init(FColor::White, NoiseSamplesPerLine * NoiseSamplesPerLine);

	TArray<FVector> Normals;
	Normals.Init(localUp, NoiseSamplesPerLine * NoiseSamplesPerLine);

	TArray<FRuntimeMeshTangent> Tangents;
	Tangents.Init(FRuntimeMeshTangent(0, 0, 0), NoiseSamplesPerLine * NoiseSamplesPerLine);

	FRuntimeMeshCollisionSettings runtimeMeshSettings;
	runtimeMeshSettings.bUseComplexAsSimple = true;
	runtimeMeshSettings.bUseAsyncCooking = true;

	UMaterial* terrainMaterialInstance = LoadObjFromPath<UMaterial>(FName(*sPath));
	StaticProviderIn->SetupMaterialSlot(0, TEXT("Material"), terrainMaterialInstance);
	StaticProviderIn->SetCollisionSettings(runtimeMeshSettings);
	StaticProviderIn->CreateSectionFromComponents(0, SectionID, SectionID, Vertices, Triangles, Normals, TexCoords, Colors, Tangents, ERuntimeMeshUpdateFrequency::Frequent, true);

}

void  AMyActor2::ChangeMaterial(URuntimeMeshProviderStatic* StaticProviderIn, int TextureID) {

}



//----------------------------------------------------------------------------------//
//Mesh Generation functions
TArray<FVector2D> AMyActor2::GenerateUVS(TArray<FVector2D>TexCoordsIn, int Resolution) {
	TexCoordsIn.Init(FVector2D(0, 0), Resolution * Resolution);

	for (int x = 0; x < Resolution; x++) {
		for (int y = 0; y < Resolution; y++) {
			int index = GetIndexForGridCoordinates(y, x, Resolution);
			FVector2D Percentage = FVector2D(x, y) / (Resolution);
			TexCoordsIn[index] = Percentage;
		}
	}
	return TexCoordsIn;
}


TArray<FVector> AMyActor2::GenerateVertices(TArray<FVector>VerticesIn, int Resolution, FVector localUp, FVector Position, TArray<double> HeightMap) {
	VerticesIn.Init(FVector(0, 0, 0), Resolution * Resolution); // 64x64

	FVector axisA = FVector(localUp.Y, localUp.Z, localUp.X);
	FVector axisB = FVector::CrossProduct(localUp, axisA);
	int VertCounter = 0;
	FVector PointOnCube;

	for (int y = 0; y < Resolution; y++) {
		for (int x = 0; x < Resolution; x++) {
			int index = GetIndexForGridCoordinates(x, y, Resolution);

			FVector2D Percentage = FVector2D(x, y) / (Resolution);

			PointOnCube = Position + ((Percentage.X - 0.5f) * 2 * axisA + (Percentage.Y - 0.5f) * 2 * axisB);
			if (HeightMap.Num() > 0 && index <= HeightMap.Num() - 1) {
				PointOnCube.Z = HeightMap[index];
			}

			//Get mesh by multiplying the normalised points with desired mesh size
			PointOnCube = FVector(PointOnCube.X * 200, PointOnCube.Y * 200, PointOnCube.Z * 10);

			VerticesIn[index] = PointOnCube;

		}

	}
	return VerticesIn;
}

int AMyActor2::GetIndexForGridCoordinates(int x, int y, int NoiseSamplesPerLine) {
	return x + y * NoiseSamplesPerLine;
}

FVector2D AMyActor2::GetPositionForGridCoordinates(int x, int y, int NoiseResolution) {
	return FVector2D(
		x * NoiseResolution,
		y * NoiseResolution
	);
}

TArray<int>  AMyActor2::GenerateTriangles(int NoiseSamplesPerLine, TArray<int>TrianglesIn, int TriangleOffset) {

	int NumberOfQuadsPerLine = NoiseSamplesPerLine - 1; // We have one less quad per line than the amount of vertices, since each vertex is the start of a quad except the last ones
	// In our triangles array, we need 6 values per quad
	int TrianglesArraySize = NumberOfQuadsPerLine * NumberOfQuadsPerLine * 6;
	TrianglesIn.Init(0, TrianglesArraySize);
	int TriangleIndex = 0;

	for (int y = 0; y < NumberOfQuadsPerLine; y++) {
		for (int x = 0; x < NumberOfQuadsPerLine; x++) {

			// Getting the indexes of the four vertices making up this quad
			int bottomLeftIndex = GetIndexForGridCoordinates(x, y, NoiseSamplesPerLine);
			int topLeftIndex = GetIndexForGridCoordinates(x, y + 1, NoiseSamplesPerLine);
			int topRightIndex = GetIndexForGridCoordinates(x + 1, y + 1, NoiseSamplesPerLine);
			int bottomRightIndex = GetIndexForGridCoordinates(x + 1, y, NoiseSamplesPerLine);

			// Assigning the 6 triangle points to the corresponding vertex indexes, by going counter-clockwise.
			TrianglesIn[TriangleIndex] = bottomLeftIndex + TriangleOffset;
			TrianglesIn[TriangleIndex + 1] = topLeftIndex + TriangleOffset;
			TrianglesIn[TriangleIndex + 2] = topRightIndex + TriangleOffset;
			TrianglesIn[TriangleIndex + 3] = bottomLeftIndex + TriangleOffset;
			TrianglesIn[TriangleIndex + 4] = topRightIndex + TriangleOffset;
			TrianglesIn[TriangleIndex + 5] = bottomRightIndex + TriangleOffset;
			TriangleIndex += 6;
		}
	}
	return TrianglesIn;
}


