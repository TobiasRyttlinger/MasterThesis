#include "QuadtreeNode.h"
#include "DrawDebugHelpers.h"


QuadtreeNode::QuadtreeNode()
{
}

///** Shallow copies a QuadtreeNode. */
//QuadtreeNode::QuadtreeNode(const QuadtreeNode& copy)
//{
//	SetParentNode(copy.parentNode);
//	childNodes = copy.childNodes;
//	Radius = copy.Radius;
//	LodLevel = copy.LodLevel;
//	localUp = copy.localUp;
//	AxisA = copy.AxisA;
//	AxisB = copy.AxisB;
//	Position = copy.Position;
//	childNodes = copy.childNodes;
//}

QuadtreeNode::~QuadtreeNode()
{
}

void QuadtreeNode::initialiseNode(TSharedPtr<QuadtreeNode> Parent, FVector inPosition, double radius, int lodLevel, FVector localUp, FVector axisA, FVector axisB, int pos) {
	parentNode = Parent;
	Position = inPosition;
	NodeRadius = radius;
	NodeLOD = lodLevel;
	LocalUp = localUp;
	AxisA = axisA;
	AxisB = axisB;
	NodePosition = pos;
	Texture = nullptr;
	FString tempSectionID = "10";
	initialised = false;

	if (Parent.IsValid()) {
		FString ParentID = FString::FromInt(Parent->SectionID);
		FString NodeID = FString::FromInt(NodePosition);
		 tempSectionID = ParentID + NodeID;
	}


	SectionID = FCString::Atoi(*tempSectionID);
	//UE_LOG(LogTemp, Warning, TEXT("distance: %d"), SectionID);
}

void QuadtreeNode::SetNodePosition(int position) {
	SectionID = position;
}

void QuadtreeNode::SetParentNode(TSharedPtr<QuadtreeNode> parentNodein)
{
	this->parentNode = parentNodein;
}

TArray<FVector> QuadtreeNode::GetVertices() {
	return Vertices;
}
TArray<int> QuadtreeNode::GetTriangles() {
	return Triangles;
}


TSharedPtr<QuadtreeNode>  QuadtreeNode::GetParentNode()
{
	return parentNode;
}

void QuadtreeNode::AddChildNode(TSharedPtr<QuadtreeNode> node)
{
	childNodes.Add(node);

}

/**Delete unused childnodes*/
void QuadtreeNode::ClearChildren() {
	if (childNodes.Num() > 0) {
		for (auto child : GetChildNodes()) {

			child->ClearChildren();
		}
	}
	childNodes.Empty();
}

/** Get the distance from the root node of the tree. */
double QuadtreeNode::GetRadius() {
	return this->NodeRadius;
}

/** Get the distance from the root node of the tree. */
FVector QuadtreeNode::GetPosition() {
	return this->Position;
}


int QuadtreeNode::GetLOD() {
	return NodeLOD;
}

/** Get the distance from the root node of the tree. */
FVector QuadtreeNode::GetLocalUp() {
	return this->LocalUp;
}

TArray<TSharedPtr<QuadtreeNode>> QuadtreeNode::GetChildNodes()
{
	return childNodes;
}

bool QuadtreeNode::HasChildNodes() const
{	//UE_LOG(LogTemp, Warning, TEXT("Adding child node nr: %d"), childNodes.Num());

	return childNodes.Num() > 0;
}

void QuadtreeNode::readFile()
{
	FString CompleteFilePath = "C:/Users/Admin/Downloads/Cubemap//50N030E.pgw";
	TArray<FString> FileData;
	FileData.Init("TEST", 5);
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*CompleteFilePath))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Could not Find File"));
		return;
	}

	FFileHelper::LoadFileToStringArray(FileData, *CompleteFilePath);
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Longitude: ")) + FileData[4]);
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("Latitude: ")) + FileData[5]);
	FileData.Empty();
}

void QuadtreeNode::GenerateNodeMesh(AMyActor* in, URuntimeMeshProviderStatic* StaticProviderIn, FVector LocalUpIn, int LodLevel) {

	readFile();
	RMC = NewObject<URuntimeMeshComponentStatic>(in);
	RMC->RegisterComponent();

	RMC->Initialize(StaticProviderIn);

	int NoiseSamplesPerLine = 16;
	double NoiseInputScale = 0.01; // Making this smaller will "stretch" the perlin noise terrain
	double NoiseOutputScale = 2000; // Making this bigger will scale the terrain's height

	TArray<uint8> RawFileData = {};

	FString TexturePath = "Texture2D'/Game/simplex.simplex'";
	FString sPath = "Material'/Game/simplex_Mat.simplex_Mat'";
	FString LocalPath = "Texture2D'/Game/Heightmaps/posx.posx'";

		if (LocalUpIn.X > 0) {
			 LocalPath = "Texture2D'/Game/Heightmaps/negz.negz'";
		}
		else if (LocalUpIn.Y > 0) {
			LocalPath = "Texture2D'/Game/Heightmaps/posx.posx'";
		}
		else if (LocalUpIn.Z > 0) {
			 LocalPath = "Texture2D'/Game/Heightmaps/posy.posy'";
		}

		else if (LocalUpIn.X < 0) {
			 LocalPath = "Texture2D'/Game/Heightmaps/posz.posz'";
		}
		else if (LocalUpIn.Y < 0) {
			LocalPath = "Texture2D'/Game/Heightmaps/negx.negx'";
		}
		else if (LocalUpIn.Z < 0) {
			LocalPath = "Texture2D'/Game/Heightmaps/negy.negy'";
		}
	
	
	Mat = LoadObjFromPath<UMaterial>(FName(sPath));
	DynMat = UMaterialInstanceDynamic::Create(Mat, in);
	UMaterial* terrainMaterialInstance = LoadObjFromPath<UMaterial>(FName(*sPath));
	UTexture2D* temptexture = LoadObjFromPath<UTexture2D>(FName(*LocalPath));

	TArray<double> HeightMap;
	HeightMap.Init(0, NoiseSamplesPerLine * NoiseSamplesPerLine);
	
	//if(NodeLOD >= 8){ HeightMap = CalculateHeightMap(temptexture); }

	Vertices = GenerateVertices(in,Vertices, NoiseSamplesPerLine, LocalUpIn, HeightMap);
	Triangles = GenerateTriangles(NoiseSamplesPerLine, Triangles, 0);
	GenerateUVS(NoiseSamplesPerLine);

	TArray<FColor> Colors;
	Colors.Init(FColor::White, NoiseSamplesPerLine * NoiseSamplesPerLine);

	TArray<FVector> Normals;
	Normals.Init(LocalUpIn, NoiseSamplesPerLine * NoiseSamplesPerLine);

	TArray<FRuntimeMeshTangent> Tangents;
	Tangents.Init(FRuntimeMeshTangent(0, 0, 0), NoiseSamplesPerLine * NoiseSamplesPerLine);

	FRuntimeMeshCollisionSettings runtimeMeshSettings;
	runtimeMeshSettings.bUseComplexAsSimple = false;
	runtimeMeshSettings.bUseAsyncCooking = true;

	DynMat->SetTextureParameterValue(FName("Texture"), temptexture);

	StaticProviderIn->SetupMaterialSlot(0, TEXT("Material"), DynMat);

	StaticProviderIn->SetCollisionSettings(runtimeMeshSettings);
	StaticProviderIn->CreateSectionFromComponents(0, SectionID, 0, Vertices, Triangles, Normals, TexCoords, Colors, Tangents, ERuntimeMeshUpdateFrequency::Frequent, true);

	//GEngine->ForceGarbageCollection(true);
}

TArray<double> QuadtreeNode::CalculateHeightMap(UTexture2D* TexIn) {

	FTexture2DMipMap* MyMipMap = &TexIn->PlatformData->Mips[0];
	TexIn->SRGB = false;
	TexIn->UpdateResource();

	const FColor* FormatedImageData = reinterpret_cast<const FColor*>(TexIn->PlatformData->Mips[0].BulkData.LockReadOnly());
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

void QuadtreeNode::LoadTextureFromPath(const FString& FullFilePath)
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


void QuadtreeNode::GenerateUVS( int Resolution) {
	TexCoords.Init(FVector2D(0, 0), Resolution * Resolution);
		
		for (int x = 0; x < Resolution; x++) {
				for (int y = 0; y < Resolution; y++) {
					int index = GetIndexForGridCoordinates(x, y, Resolution);
					FVector2D Percentage =  FVector2D(x, y) / (Resolution);

					if (NodeLOD > 0) {
						if (NodeLOD == 1) {
							if (NodePosition == 4) { //Top left 4
								Percentage = FVector2D(x , y + Resolution) / (Resolution * 2);
							}
							if (NodePosition == 1) { //Bottom Right 1
								Percentage = FVector2D(x + Resolution, y+1) / (Resolution * 2);
							
							}
							if (NodePosition == 2) {//Top Right 2
					
								Percentage = FVector2D(x +  Resolution, y + Resolution) / (Resolution * 2);
							
							}
							if (NodePosition == 3) {//Bottom Left
								Percentage = FVector2D(x, y) / (Resolution * 2);
							}
						}
						else {

							if (NodePosition == 4) { //Top left
								Percentage = FVector2D(x, y + Resolution) / (Resolution * 2);
							}
							if (NodePosition == 1) { //Bottom Right
								Percentage = FVector2D(x + Resolution, y) / (Resolution * 2);

							}
							if (NodePosition == 2) {//Top Right
									Percentage.X = x + Resolution / (Resolution * 2);
									Percentage.X = y + Resolution / (Resolution * 2);
							}
							if (NodePosition == 3) {//Bottom Left
								Percentage.X = x + Resolution / (Resolution * 2);
								Percentage.X = y + Resolution / (Resolution * 2);
							}
						}
					}
					else {
						Percentage = FVector2D(x, y) / (Resolution);
					}
				
					TexCoords[index] = Percentage;
				}
			}
	//}
	
}

TArray<FVector> QuadtreeNode::GenerateVertices(AMyActor* in, TArray<FVector>VerticesIn, int Resolution, FVector localUp, TArray<double> HeightMap) {
	VerticesIn.Init(FVector(0,0,0), Resolution * Resolution); // 64x64

	FVector axisA = FVector(LocalUp.Y, LocalUp.Z, LocalUp.X);
	FVector axisB = FVector::CrossProduct(LocalUp, axisA);

	FVector PointOnCube;
	FVector PointOnSphere;
	for (int y = 0; y < Resolution; y++) {
		for (int x = 0; x < Resolution; x++) {
			int index = GetIndexForGridCoordinates(x, y, Resolution);
	
			FVector2D Percentage = FVector2D(x, y) /( Resolution-1);	
		
			PointOnCube = Position + ((Percentage.X - 0.5f) * 2 * axisA + (Percentage.Y - 0.5f) * 2 * axisB) * this->GetRadius();
			PointOnCube.Normalize(1.0f);
		
			PointOnSphere = PointOnCube*in->PlanetSize;
			//PointOnSphere.Z -= 636000000;
			VerticesIn[index] = PointOnSphere;
		}
	}
	return VerticesIn;
}



int QuadtreeNode::GetIndexForGridCoordinates(int x, int y, int NoiseSamplesPerLine) {
	return x + y * NoiseSamplesPerLine;
}

FVector2D QuadtreeNode::GetPositionForGridCoordinates(int x, int y, int NoiseResolution) {
	return FVector2D(
		x * NoiseResolution,
		y * NoiseResolution
	);
}

TArray<int>  QuadtreeNode::GenerateTriangles(int NoiseSamplesPerLine, TArray<int>TrianglesIn,int TriangleOffset) {
	
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



