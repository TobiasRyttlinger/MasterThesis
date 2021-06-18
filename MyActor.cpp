// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor.h"
#include "Quadtree.h"

Quadtree QT_1;
Quadtree QT_2;
Quadtree QT_3;
Quadtree QT_4;
Quadtree QT_5;
Quadtree QT_6;

// Sets default values
AMyActor::AMyActor()
{
	
	//static ConstructorHelpers::FObjectFinder<UStaticMeshComponent> Tree(TEXT("StaticMesh'/Game/Pine/0_LOD0_pine_Mat_0.0_LOD0_pine_Mat_0'"));z
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObj(TEXT("StaticMesh'/Game/Pine/0_LOD0_pine_Mat_0.0_LOD0_pine_Mat_0'"));
	if (MeshObj.Object) {
		UE_LOG(LogTemp, Warning, TEXT(" mesh found breaking function"));
	}
	//static ConstructorHelpers::FObjectFinder<UStaticMesh>MyObj(TEXT("StaticMesh'/Game/Pine/0_LOD0_pine_Mat_0.0_LOD0_pine_Mat_0'"));
	
	InstancedStaticMeshComponent = CreateDefaultSubobject< UInstancedStaticMeshComponent >(TEXT("InstancedStaticMeshComponentComponent"));
	//InstancedStaticMeshComponent->AttachTo(RootComponent);
	StaticMeshComponent->SetStaticMesh(MeshObj.Object);
	InstancedStaticMeshComponent->SetStaticMesh(MeshObj.Object);
	InstancedStaticMeshComponent->SetMobility(EComponentMobility::Movable);

	
	StaticMeshComponent->SetRelativeRotation(FRotator(0, 90, 0));
	StaticMeshComponent->SetWorldRotation(FRotator(0, 90, 0));
	//srand(time(0)); //Seed the random system
	//for (int i = 0; i < 10000; i++) {
	//	RandomPosition.Add(FVector(FMath::RandRange(0, 1), FMath::RandRange(0, 1),1));
	//}

	//meshToUse = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, TEXT("StaticMesh'/Game/Pine/StaticMesh.StaticMesh'")));
	//StaticMeshComponent->SetWorldLocation(FVector(348869280, -79578736, 526509792));
		// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	meshToUse = StaticMeshComponent->GetStaticMesh();
}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{

	//FString Cord = "N";
	//TMap<FString,UTexture2D*> TexArray;

	//for (int i = 0; i < 2; i++) {
	//	int NS = 10;
	//	int WE = 0;
	//	while (NS <= 90 ) {
	//		while (WE <= 150) {
	//			//UE_LOG(LogTemp, Warning, TEXT("WE:, %d"), WE);
	//			if (i == 0) {
	//				Cord = FString("N");
	//			}
	//			if (i == 1) {
	//				Cord = FString("S");
	//			}
	//			FString ValW;
	//			FString ValE;

	//			FString ValNS = FString::FromInt(NS) + Cord;

	//			if (WE < 100) {
	//				ValW = FString::FromInt(0) + FString::FromInt(WE) + FString("W");
	//				ValE = FString::FromInt(0) + FString::FromInt(WE) + FString("E");
	//			}
	//			else if (WE <30 ) {
	//				ValW = FString("000W");
	//				
	//				ValE = FString::FromInt(0) + FString::FromInt(0) + FString::FromInt(WE) + FString("E");
	//			}
	//			else {
	//				ValW = FString::FromInt(WE) + FString("W");
	//				ValE = FString::FromInt(WE) + FString("E");
	//			}

	//			FString textureCoords = ValNS + ValW;
	//			FString FullFilePath = FString("C:/Users/Admin/Downloads/HeightmapGlobalPNG/") + textureCoords + FString(".png");

	//			TexArray.Add(textureCoords, LoadTextureFromPath(FullFilePath));
	//			textureCoords = ValNS + ValE;
	//			FullFilePath = FString("C:/Users/Admin/Downloads/HeightmapGlobalPNG/") + textureCoords + FString(".png");
	//			
	//			TexArray.Add(textureCoords, LoadTextureFromPath(FullFilePath));
	//			WE += 30;
	//		}
	//		NS += 20;
	//		WE = 0;
	//	}
	//}
	//for (auto& Elem : TexArray){
	//	UE_LOG(LogTemp, Warning, TEXT("TexArray:, %s"), *Elem.Key);
	//}


	counter = 0;
	Super::BeginPlay();

	FVector up = this->GetActorUpVector();
	FVector right = this->GetActorRightVector();
	FVector forward = this->GetActorForwardVector();

	FVector down = -up;
	FVector left = -right;
	FVector back = -forward;

	QT_1 = Quadtree(this, up, this->PlanetSize, 0);
	QT_2 = Quadtree(this, down, this->PlanetSize, 0);
	QT_3 = Quadtree(this, right, this->PlanetSize, 0);
	QT_4 = Quadtree(this, forward, this->PlanetSize, 0);
	QT_5 = Quadtree(this, left, this->PlanetSize, 0);
	QT_6 = Quadtree(this, back, this->PlanetSize, 0);
	QT_1.UpdateMesh(QT_1.GetRootNode());
	//QT_2.UpdateMesh(QT_2.GetRootNode());
	//QT_3.UpdateMesh(QT_3.GetRootNode());
	//QT_4.UpdateMesh(QT_4.GetRootNode());
	//QT_5.UpdateMesh(QT_5.GetRootNode());
	//QT_6.UpdateMesh(QT_6.GetRootNode());



}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{

	if (counter >= 0.3f) {
		//GEngine->ForceGarbageCollection(true);
		QT_1.UpdateMesh(QT_1.GetRootNode());
		/*QT_2.UpdateMesh(QT_2.GetRootNode());
		QT_3.UpdateMesh(QT_3.GetRootNode());
		QT_4.UpdateMesh(QT_4.GetRootNode());
		QT_5.UpdateMesh(QT_5.GetRootNode());
		QT_6.UpdateMesh(QT_6.GetRootNode());*/

	

		counter = 0;
		GEngine->ForceGarbageCollection();
	}
	QT_1.GenerateTerrain(QT_1.VisiblechildrenNodes);
	QT_2.GenerateTerrain(QT_2.VisiblechildrenNodes);
	QT_3.GenerateTerrain(QT_3.VisiblechildrenNodes);
	QT_4.GenerateTerrain(QT_4.VisiblechildrenNodes);
	QT_5.GenerateTerrain(QT_5.VisiblechildrenNodes);
	QT_6.GenerateTerrain(QT_6.VisiblechildrenNodes);
	counter += DeltaTime;
	Super::Tick(DeltaTime);
	

}




UTexture2D* AMyActor::LoadTextureFromPath(const FString& FullFilePath)
{
	UTexture2D* Texture = nullptr;
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
	return Texture;
}
