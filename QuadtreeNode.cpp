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

	FString tempSectionID = "10";
	if (Parent.IsValid()) {
		FString ParentID = FString::FromInt(Parent->SectionID);
		FString NodeID = FString::FromInt(NodePosition);
		 tempSectionID = ParentID + NodeID;
	}



	SectionID = FCString::Atoi(*tempSectionID);

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
{
	//UE_LOG(LogTemp, Warning, TEXT("Adding child node nr: %d"), childNodes.Num());
	return childNodes.Num() > 0;
}

TArray<FVector2D> QuadtreeNode::GenerateUVS(TArray<FVector2D>TexCoordsIn, int Resolution) {
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

void QuadtreeNode::GenerateNodeMesh(AMyActor* in, URuntimeMeshProviderStatic* StaticProviderIn, FVector LocalUpIn, int LodLevel) {


	RMC = NewObject<URuntimeMeshComponentStatic>(in);
	RMC->RegisterComponent();
	RMC->Initialize(StaticProviderIn);

	int NoiseSamplesPerLine = 8;
	double NoiseInputScale = 0.01; // Making this smaller will "stretch" the perlin noise terrain
	double NoiseOutputScale = 2000; // Making this bigger will scale the terrain's height

	TArray<uint8> RawFileData = {};

	FString TexturePath = "Texture2D'/Game/simplex.simplex'";
	FString sPath = "Material'/Game/simplex_Mat.simplex_Mat'";
	FString LocalPath = "C:/Users/Admin/Downloads/Tiles/1.png";

	//LoadTextureFromPath(LocalPath);

	TArray<double> HeightMap;/* = CalculateHeightMap(Texture);*/
	HeightMap.Init(0, NoiseSamplesPerLine * NoiseSamplesPerLine);

	Vertices = GenerateVertices(in,Vertices, NoiseSamplesPerLine, LocalUpIn, HeightMap);
	Triangles = GenerateTriangles(NoiseSamplesPerLine, Triangles, 0);
	TexCoords = GenerateUVS(TexCoords, NoiseSamplesPerLine);

	TArray<FColor> Colors;
	Colors.Init(FColor::White, NoiseSamplesPerLine * NoiseSamplesPerLine);

	TArray<FVector> Normals;
	Normals.Init(LocalUpIn, NoiseSamplesPerLine * NoiseSamplesPerLine);

	TArray<FRuntimeMeshTangent> Tangents;
	Tangents.Init(FRuntimeMeshTangent(0, 0, 0), NoiseSamplesPerLine * NoiseSamplesPerLine);

	FRuntimeMeshCollisionSettings runtimeMeshSettings;
	runtimeMeshSettings.bUseComplexAsSimple = true;
	runtimeMeshSettings.bUseAsyncCooking = true;

	UMaterial* terrainMaterialInstance = LoadObjFromPath<UMaterial>(FName(*sPath));
	StaticProviderIn->SetupMaterialSlot(0, TEXT("Material"), terrainMaterialInstance);
	StaticProviderIn->SetCollisionSettings(runtimeMeshSettings);
	StaticProviderIn->CreateSectionFromComponents(0, SectionID, 0, Vertices, Triangles, Normals, TexCoords, Colors, Tangents, ERuntimeMeshUpdateFrequency::Frequent, true);

}

TArray<double> QuadtreeNode::CalculateHeightMap(UTexture2D* TexIn) {

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
			VerticesIn[index] = PointOnSphere;
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("PointOnCube is %s"), *PointOnCube.ToString());
	//UE_LOG(LogTemp, Warning, TEXT("PointOnCube is %s"), *PointOnSphere.ToString());
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



