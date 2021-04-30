#include "QuadtreeNode.h"
#include "DrawDebugHelpers.h"


QuadtreeNode::QuadtreeNode()
{
}


QuadtreeNode::~QuadtreeNode()
{
	GEngine->ForceGarbageCollection(true);

	RMC = NULL;
}

void QuadtreeNode::initialiseNode(AMyActor* in, URuntimeMeshProviderStatic* StaticProviderIn, TSharedPtr<QuadtreeNode> Parent, FVector inPosition, double radius, int lodLevel, FVector localUp, FVector axisA, FVector axisB, int pos) {
	parentNode = Parent;
	Position = inPosition;	
	NodeRadius = radius;
	NodeLOD = lodLevel;
	LocalUp = localUp;
	AxisA = axisA;
	AxisB = axisB;
	NodePosition = pos;
	SectionID = 0;
	Texture = nullptr;
	FString tempSectionID = FString("0");
	initialised = false;
	LatLong = ToLatLong(Position);
	MeshResolution = 32;

	//Memory Leak solved
	RMC = NewObject<URuntimeMeshComponentStatic>(in);
	RMC->RegisterComponent();
	RMC->Initialize(StaticProviderIn);

	HeightMap.Init(0, MeshResolution * MeshResolution);
	FRuntimeMeshCollisionSettings runtimeMeshSettings;

	runtimeMeshSettings.bUseComplexAsSimple = false;
	runtimeMeshSettings.bUseAsyncCooking = true;
	//URuntimeMeshModifierNormals::CalculateNormalsTangents(MeshData, true);
	StaticProviderIn->SetCollisionSettings(runtimeMeshSettings);



	if (Parent.IsValid()) {

		FString ParentID = FString::FromInt(Parent->SectionID);

		FString NodeID = FString::FromInt(NodePosition);
		FString LodID = FString::FromInt(NodeLOD);


		if (NodeLOD == 5) {

			ParentID = 0;
		}
	

		tempSectionID = ParentID + NodeID;


	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, tempSectionID);
	SectionID = FCString::Atoi(*tempSectionID);

	//Load Texture
	FString TexturePath = GetTexture(LocalUp);
	Texture = LoadObjFromPath<UTexture2D>(FName(TexturePath));
	if (HeightMap.Num() == 0) {
		HeightMap.Init(0, MeshResolution * MeshResolution);
	}

	//Load HeightMapTexture
	if (NodeLOD == 8) {
		FString HeightmapPath = GetHeightMap(LocalUp);
		HeightMapTexture = LoadObjFromPath<UTexture2D>(FName(HeightmapPath));
		//Load Heightmap pixel values or initialise empty array.
		if (HeightMapTexture != nullptr) {
		
			HeightMapTexture->SRGB = false;
			HeightMapTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
			HeightMapTexture->UpdateResource();
			CalculateHeightMap();

		}
		else {
			HeightmapPath = "Texture2D'/Game/Heightmaps/LOD8/NoData.NoData'";
			HeightMapTexture = LoadObjFromPath<UTexture2D>(FName(HeightmapPath));
			HeightMapTexture->SRGB = false;
			HeightMapTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
			HeightMapTexture->UpdateResource();


		}
		//GetPixelValues();
	}

//	Texture = HeightMapTexture;
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
FVector2D QuadtreeNode::ToLatLong(FVector inPosition) {
	FVector2D temp;
	temp.X = FMath::Asin(inPosition.Y / 636000000) * (180 / PI);
	temp.Y = FMath::Atan2(inPosition.Z, inPosition.X) * (180 / PI);

	if (temp.X < 0.5) {
		temp.X = 0;
	}
	else {
		temp.X = FMath::CeilToDouble((temp.X) / 10) * 10;
	}
	if (temp.Y < 0.5) {
		temp.Y = 0;
	}
	else {
		temp.Y = FMath::CeilToDouble((temp.Y) / 10) * 10;
	}

	//UE_LOG(LogTemp, Warning, TEXT("ToLatLong:, %s"), *temp.ToString());
	return temp;
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
{
	return childNodes.Num() > 0;
}

FVector QuadtreeNode::ToCartesian(double Longitude, double Latitude) {
	double cosLat = FMath::Cos(Latitude * PI / 180.0);
	double sinLat = FMath::Sin(Latitude * PI / 180.0);
	double cosLon = FMath::Cos(Longitude * PI / 180.0);
	double sinLon = FMath::Sin(Longitude * PI / 180.0);
	double rad = 636000000;

	double x = rad * cosLat * cosLon;
	double y = rad * cosLat * sinLon;
	double z = rad * sinLat;

	return FVector(x, y, z);
}

void QuadtreeNode::readFile()
{

	FString CompleteFilePath = "C:/Users/Admin/Downloads/Cubemap//50N000E.pgw";
	TArray<FString> FileData;
	FileData.Init("TEST", 5);
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*CompleteFilePath))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Could not Find File"));
		return;
	}

	FFileHelper::LoadFileToStringArray(FileData, *CompleteFilePath);

	double Longitude = -FCString::Atod(*FileData[4]);
	double Latitude = FCString::Atod(*FileData[5]);
	//50m data lat Long
	Latitude = 55.60587;
	Longitude = -13.00073;


	FVector Cartesian = ToCartesian(Longitude, Latitude);
	//UE_LOG(LogTemp, Warning, TEXT("Cartesian: %s"), *Cartesian.ToString());
	FileData.Empty();
}


FString QuadtreeNode::GetTexture(FVector VecUpin) {
	FString LocalPath = "Texture2D'/Game/BlueMarble/posx.posx'";

	//Default maps Blue marble
	if (VecUpin.X > 0) {
		LocalPath = "Texture2D'/Game/BlueMarble/posx.posx'";
	}
	else if (VecUpin.Y > 0) {
		LocalPath = "Texture2D'/Game/BlueMarble/posz.posz'";
	}
	else if (VecUpin.X < 0) {
		LocalPath = "Texture2D'/Game/BlueMarble/posz.posz'";
	}
	else if (VecUpin.Y < 0) {
		LocalPath = "Texture2D'/Game/BlueMarble/negz.negz'";
	}
	else if (VecUpin.Z < 0) {
		LocalPath = "Texture2D'/Game/BlueMarble/negy.negy'";
	}
	else {
		LocalPath = "Texture2D'/Game/BlueMarble/posy.posy'";

		if (NodeLOD > 0) {
			LocalPath = FString("Texture2D'/Game/Imagery/LOD") + FString::FromInt(NodeLOD) + FString("/") + FString::FromInt(SectionID) + FString(".") + FString::FromInt(SectionID) + FString("'");
			//UE_LOG(LogTemp, Warning, TEXT("LocalPath: %s"), *LocalPath);
		}
	}
	return LocalPath;
}

FString QuadtreeNode::GetHeightMap(FVector VecUpin) {
	FString LocalPath = "Texture2D'/Game/Heightmaps/posx.posx'";

	//Default maps Blue marble
	if (VecUpin.X > 0) {
		LocalPath = "Texture2D'/Game/Heightmaps/negz.negz'";
	}
	else if (VecUpin.Y > 0) {
		LocalPath = "Texture2D'/Game/Heightmaps/posx.posx'";
	}
	else if (VecUpin.X < 0) {
		LocalPath = "Texture2D'/Game/Heightmaps/posz.posz'";
	}
	else if (VecUpin.Y < 0) {
		LocalPath = "Texture2D'/Game/Heightmaps/negx.negx'";
	}
	else if (VecUpin.Z < 0) {
		LocalPath = "Texture2D'/Game/Heightmaps/negy.negy'";
	}

	else {
		LocalPath = "Texture2D'/Game/Heightmaps/posy.posy'";

		if (NodeLOD > 0) {
			LocalPath = FString("Texture2D'/Game/Heightmaps/LOD") + FString::FromInt(NodeLOD) + FString("/") + FString::FromInt(SectionID) + FString(".") + FString::FromInt(SectionID) + FString("'");
			//UE_LOG(LogTemp, Warning, TEXT("LocalPath: %s"), *LocalPath);
		}
	}
	return LocalPath;
}

void QuadtreeNode::GenerateNodeMesh(AMyActor* in, URuntimeMeshProviderStatic* StaticProviderIn, FVector LocalUpIn, int LodLevel) {

	//readFile();

	FString sPath = "Material'/Game/simplex_Mat.simplex_Mat'";


	Mat = LoadObjFromPath<UMaterial>(FName(sPath));
	DynMat = UMaterialInstanceDynamic::Create(Mat, in);

	if (Vertices.Num() == 0) {
		Vertices = GenerateVertices(in, Vertices, MeshResolution, LocalUpIn);
		
	}
	
	if (Triangles.Num() == 0) {
		Triangles = GenerateTriangles(MeshResolution, Triangles, 0);
		Vertices = ApplyHeightMap(Vertices);
	}
	
	DynMat->SetTextureParameterValue(FName("Texture"), Texture);

	StaticProviderIn->SetupMaterialSlot(SectionID, FName("Material"), DynMat);

	StaticProviderIn->CreateSectionFromComponents(0, SectionID, SectionID, Vertices, Triangles, Normals, TexCoords, Colors, Tangents, ERuntimeMeshUpdateFrequency::Average, true);

	if (parentNode.IsValid()) {

		StaticProviderIn->RemoveSection(0, parentNode->SectionID);
	}
}

void QuadtreeNode::CalculateHeightMap() {
	FTexture2DMipMap* MyMipMap = &HeightMapTexture->PlatformData->Mips[5];
	if (MyMipMap == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Breaking at MyMipMap"));
		return;
	}
	FColor* data = (FColor*)HeightMapTexture->PlatformData->Mips[5].BulkData.LockReadOnly();
	FColor tempColor;
	if (data == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Breaking at FormatedImageData"));
		HeightMapTexture->PlatformData->Mips[5].BulkData.Unlock();
		return;
	}

	for (int x = 0; x < MyMipMap->SizeX; x++) {
		for (int y = 0; y < MyMipMap->SizeY; y++) {

			int index = x * MyMipMap->SizeY + y;
			tempColor = data[index];

			double Value = (((double)tempColor.R / 255.0) + ((double)tempColor.G / 255.0) + ((double)tempColor.B / 255.0)) / 3.0;
			HeightMap[index] = Value;
		}
	}
	HeightMapTexture->PlatformData->Mips[5].BulkData.Unlock();

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

void QuadtreeNode::GetPixelValues() {
	FTexture2DMipMap* MyMipMap = &Texture->PlatformData->Mips[5];

	if (MyMipMap == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Breaking at MyMipMap"));
		return;
	}

	FColor* data = (FColor*)Texture->PlatformData->Mips[5].BulkData.LockReadOnly();
	FColor tempColor;

	if (data == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Breaking at FormatedImageData"));
		Texture->PlatformData->Mips[5].BulkData.Unlock();
		return;
	}

	for (int x = 0; x < MyMipMap->SizeX; x++) {
		for (int y = 0; y < MyMipMap->SizeY; y++) {

			int index = x * MyMipMap->SizeY + y;
			tempColor = data[index];

			double Value = (((double)tempColor.R / 255.0) + ((double)tempColor.G / 255.0) + ((double)tempColor.B / 255.0)) / 3.0;

			if (tempColor.G < 155) {
				Value = 0;
			}
			PixelValues.Add(Value);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Generate nodes for LOD level: %d"), PixelValues.Num());

	Texture->PlatformData->Mips[5].BulkData.Unlock();

}

TArray<FVector> QuadtreeNode::GenerateVertices(AMyActor* in, TArray<FVector>VerticesIn, int Resolution, FVector localUp) {
	VerticesIn.Init(FVector(0, 0, 0), Resolution * Resolution);
	TexCoords.Init(FVector2D(0, 0), Resolution * Resolution);
	FVector axisA = FVector(LocalUp.Y, LocalUp.Z, LocalUp.X);
	FVector axisB = FVector::CrossProduct(LocalUp, axisA);

	FVector PointOnCube;
	FVector PointOnSphere;

	for (int y = 0; y < Resolution; y++) {
		for (int x = 0; x < Resolution; x++) {

			int index = GetIndexForGridCoordinates(x, y, Resolution);

			FVector2D Percentage = FVector2D(x, y) / (Resolution - 1);

			PointOnCube = Position + ((Percentage.X - 0.5f) * 2 * axisA + (Percentage.Y - 0.5f) * 2 * axisB) * this->GetRadius();
			TexCoords[index] = Percentage;
			PointOnCube.Normalize();

			

			//if (localUp.X > 0) {
			//	PointOnCube.X += HeightMap[index] * 0.01;
			//}
			//else if (localUp.Y > 0) {
			//	PointOnCube.Y += HeightMap[index] * 0.01;
			//}
			//else if (localUp.Z > 0) {
			//	//PointOnCube.Z += HeightMap[index] * 0.001;
			//}
			//else if (localUp.X < 0) {
			//	PointOnCube.X -= HeightMap[index] * 0.01;
			//}
			//else if (localUp.Y < 0) {
			//	PointOnCube.Z -= HeightMap[index] * 0.01;
			//}
			//else if (localUp.Z < 0) {
			//	PointOnCube.Y -= HeightMap[index] * 0.01;
			//}

			//if (y < 1 && y > Resolution-1) {
			//	if (x < 1 && y > Resolution -1) {
			//		PointOnCube.Z = 0.2 * 0.001;
			//	}
			//}
			//else {
			//	PointOnCube.Z += HeightMap[index] * 0.001;
			//}


			if (x == 0 || y == 0 || x == Resolution-1 || y == Resolution-1) {
			
				VerticesIn[index].Z += 0.3* 0.001;
			
				
			}
			VerticesIn[index] = PointOnCube * 636000000;
		}
	}
	return VerticesIn;
}

TArray<FVector>  QuadtreeNode::ApplyHeightMap(TArray<FVector>VerticesIn) {

	for (int y = 1; y < MeshResolution -1 ; y++) {
		for (int x =1 ; x < MeshResolution -1 ; x++) {

			int index = GetIndexForGridCoordinates(x, y, MeshResolution);
			VerticesIn[index] += Normals[index]*(HeightMap[index] * 0.0005 * 636000000) ;
			/*if (LocalUp.X > 0) {
				VerticesIn[index].X += HeightMap[index] * 0.01;
			}
			else if (LocalUp.Y > 0) {
				VerticesIn[index].Y += HeightMap[index] * 0.01;
			}
			else if (LocalUp.Z > 0) {
				VerticesIn[index].Z += HeightMap[index] * 0.001;
			}
			else if (LocalUp.X < 0) {
				VerticesIn[index].X -= HeightMap[index] * 0.01;
			}
			else if (LocalUp.Y < 0) {
				VerticesIn[index].Z -= HeightMap[index] * 0.01;
			}
			else if (LocalUp.Z < 0) {
				VerticesIn[index].Y -= HeightMap[index] * 0.01;
			}*/
			//VerticesIn[index] = VerticesIn[index] * 636000000;
		}

	}
	
	return VerticesIn;
}

int QuadtreeNode::GetIndexForGridCoordinates(int x, int y, int MeshResolutionin) {
	return x + y * MeshResolutionin;
}

FVector QuadtreeNode::computeNormals(FVector a, FVector b, FVector c)
{
	FVector temp = FVector::CrossProduct(b - a, c - a);
	temp.Normalize();
	return temp;
}

TArray<int>  QuadtreeNode::GenerateTriangles(int MeshResolutionin, TArray<int>TrianglesIn, int TriangleOffset) {

	int NumberOfQuadsPerLine = MeshResolutionin - 1; // We have one less quad per line than the amount of vertices, since each vertex is the start of a quad except the last ones
	// In our triangles array, we need 6 values per quad
	int TrianglesArraySize = NumberOfQuadsPerLine * NumberOfQuadsPerLine * 6;
	TrianglesIn.Init(0, TrianglesArraySize);
	int TriangleIndex = 0;
	Normals.Init(FVector(0, 0, 0), MeshResolutionin * MeshResolutionin);
	Tangents.Init(FVector(0, 0, 0), MeshResolutionin * MeshResolutionin);
	for (int y = 0; y < NumberOfQuadsPerLine; y++) {
		for (int x = 0; x < NumberOfQuadsPerLine; x++) {

			// Getting the indexes of the four quad vertices
			int bottomLeftIndex = GetIndexForGridCoordinates(x, y, MeshResolutionin);
			int topLeftIndex = GetIndexForGridCoordinates(x, y + 1, MeshResolutionin);
			int topRightIndex = GetIndexForGridCoordinates(x + 1, y + 1, MeshResolutionin);
			int bottomRightIndex = GetIndexForGridCoordinates(x + 1, y, MeshResolutionin);

			//Calculate Normals for each triangle
			Normals[bottomLeftIndex] = (computeNormals(Vertices[bottomLeftIndex], Vertices[bottomRightIndex], Vertices[topLeftIndex]));

			//Normals.Add(computeNormals(Vertices[topLeftIndex], Vertices[bottomRightIndex], Vertices[topRightIndex]));
			//Tangents.Add(FVector(0, 0, 0));
			Tangents[bottomLeftIndex] = (-computeNormals(Vertices[bottomLeftIndex], Vertices[bottomRightIndex], Vertices[topLeftIndex]));
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



