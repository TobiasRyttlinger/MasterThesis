// Fill out your copyright notice in the Description page of Project Settings.

#include "Quadtree.h"
#include "RuntimeMeshComponent.h"

Quadtree::Quadtree()
{
	RootNode = nullptr;
}

Quadtree::Quadtree(AMyActor* in, FVector inlocalUp, double inRadius, int inLodLevel)
{

	QT_Actor = in;
	PlanetRadius = inRadius;
	PlanetLod = inLodLevel;
	LocalUp = inlocalUp;
	StaticProvider = NewObject<URuntimeMeshProviderStatic>(in);
	StaticProviderTree = NewObject<URuntimeMeshProviderStaticMesh>(in);

	StaticProviderTree->SetStaticMesh(in->StaticMeshComponent->GetStaticMesh());
	StaticProviderTree->SetComplexCollisionLOD(0);
	StaticProviderTree->SetMaxLOD(3);

	Thresholds.Add(INFINITY); //1
	Thresholds.Add(PlanetRadius / 1); //2
	Thresholds.Add(PlanetRadius / 2); //3
	Thresholds.Add(PlanetRadius / 3); //4
	Thresholds.Add(PlanetRadius / 4); //5
	Thresholds.Add(PlanetRadius / 5); //6
	Thresholds.Add(PlanetRadius / 6); //7
	Thresholds.Add(PlanetRadius / 8); //8
	//Thresholds.Add(PlanetRadius / 9); //9
	//Thresholds.Add(PlanetRadius / 10); //10
	//Thresholds.Add(PlanetRadius / 11); //11
	

	AxisA = FVector(inlocalUp.Y, inlocalUp.Z, inlocalUp.X);
	AxisB = FVector::CrossProduct(inlocalUp, AxisA);
	PlanetMax = PlanetRadius * 1.00000139119;
	RootNode = CreateNode(nullptr, LocalUp * PlanetRadius, PlanetRadius, PlanetLod, LocalUp, AxisA, AxisB, 10);
	//VisiblechildrenNodes.Add(RootNode);

	//InitialiseNodes(RootNode);
	GetVisibleChildren(RootNode);

}

Quadtree::~Quadtree()
{
	//	StaticProvider->ConditionalBeginDestroy();
}

TSharedPtr<QuadtreeNode> Quadtree::GetRootNode()
{
	return RootNode;
}

int Quadtree::GetMaxDistance() const
{
	return MaxDistance;
}

double Quadtree::GetDistance(FVector A, FVector B) {
	return FMath::Sqrt(FMath::Pow(B.X - A.X, 2.0f) + FMath::Pow(B.Y - A.Y, 2.0f) + FMath::Pow(B.Z - A.Z, 2.0f));
}

bool Quadtree::AreSame(double a, double b)
{
	return fabs(a - b) < 0.00000001;
}

bool Quadtree::less(double a, double b)
{
	return a < b || AreSame(a, b);
}
static bool init = false;
void Quadtree::UpdateMesh(TSharedPtr<QuadtreeNode> CurrentNode) {
	if (!init) {
		FVector Cube1 = FVector(350056256, -80821560, 524809024);
		FVector Cube2 = FVector(348383104, -79336432, 525860064);
		double CubeDist = FVector::Dist(Cube1, Cube2);
		//UE_LOG(LogTemp, Warning, TEXT("Angle: %f"), CubeDist);
	}

	FVector CameraPos = QT_Actor->GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();

	FVector MeshPos = CurrentNode->GetPosition();
	MeshPos.Normalize(1.0f);
	MeshPos = MeshPos * PlanetRadius;

	//Distance between camera and meshcenter
	double distance = FVector::Dist(CameraPos, MeshPos);

	//Find Lat and Long for meshcenter
	FVector2D LatLong;
	LatLong.X = FMath::CeilToDouble(FMath::Asin(CameraPos.Z / this->PlanetRadius) * 180 / PI);
	LatLong.Y = -FMath::CeilToDouble(FMath::Atan2(CameraPos.Y, CameraPos.X) * 180 / PI);

	float scale = 1.5f;
	if (CurrentNode->NodeLOD > 7) {
		scale = 3.0f;
	}

	if (distance <= CurrentNode->NodeRadius * scale) {
		//IF nodes has children check them
		if (CurrentNode->GetChildNodes().Num() > 0) {
			for (auto child : CurrentNode->GetChildNodes()) {
				UpdateMesh(child);
			}
		}
		//Else initialise new nodes
		else {
			if (CurrentNode->NodeLOD < Thresholds.Num()) {
				InitialiseNodes(CurrentNode);
			}
			else {
				GetVisibleChildren(CurrentNode);
			}

		}
	}
	//If distance is larger than radius
	else {
	
		//If the node has children delete them and render the node
		if (CurrentNode->GetChildNodes().Num() > 0) {
			for (auto child : CurrentNode->GetChildNodes()) {
				child->ClearChildren();
				StaticProvider->RemoveSection(0, child->SectionID);
				StaticProviderTree->RemoveSection(0, 0);
				
			}
			CurrentNode->ClearChildren();

		}

		VisiblechildrenNodes.Add(CurrentNode);
	
	}
}

double Quadtree::Dot(FVector a, FVector b) {
	return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
}

double Quadtree::Abs(FVector a) {
	return FMath::Sqrt(FMath::Pow(a.X, 2) + FMath::Pow(a.Y, 2) + FMath::Pow(a.Z, 2));
}

void Quadtree::GetVisibleChildren(TSharedPtr<QuadtreeNode> CurrentNode) {

	FVector CameraPos = QT_Actor->GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	FVector MeshPos = CurrentNode->GetPosition();
	MeshPos.Normalize(1.0f);
	MeshPos = MeshPos * PlanetRadius;
	double distance = GetDistance(CameraPos, MeshPos);

	APlayerCameraManager* camManager = QT_Actor->GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	FVector camForward = camManager->GetCameraRotation().Vector();
	//camForward.Z = 0;

	double Nomerator = Dot(camForward, FVector(MeshPos.X, MeshPos.Y, 0));
	double Denomerator = Abs(camForward) * Abs(FVector(MeshPos.X, MeshPos.Y, 0));
	double DirectionAngle = FMath::Acos(Nomerator / Denomerator) * (180 / PI);

	double CameraHeight = GetDistance(FVector(0, 0, 0), CameraPos);
	double HorizonDistance = FMath::Sqrt(FMath::Pow(CameraHeight, 2) - FMath::Pow(this->PlanetRadius, 2));
	double OverTheHorizonDist = FMath::Sqrt(FMath::Pow(this->PlanetMax, 2) - FMath::Pow(this->PlanetRadius, 2));
	double VisibilitySphere = HorizonDistance + OverTheHorizonDist;

	float Angle = FMath::Acos((FMath::Pow(PlanetRadius, 2) + FMath::Pow(CameraHeight, 2) - FMath::Pow(distance, 2)) / (2 * PlanetRadius * CameraHeight));

	//UE_LOG(LogTemp, Warning, TEXT("Angle: %f"), Angle);

	//if (Angle < 1.45 && VisibilitySphere >= distance) {
		VisiblechildrenNodes.Add(CurrentNode);
	//}
}

void Quadtree::GenerateTerrain(TArray<TSharedPtr<QuadtreeNode>> inNodes) {
	if (inNodes.Num() == 0) {
		return;
	}
	int triangleIndex = 0;
	for (auto Node : inNodes) {
		//Node->Rendered = true;
		if (Node->NodeLOD > 0) {
			Node->GenerateNodeMesh(QT_Actor, StaticProvider, StaticProviderTree, Node->GetLocalUp(), triangleIndex);
		}
		else {
			Node->GenerateNodeMesh(QT_Actor, StaticProvider, StaticProviderTree, Node->GetLocalUp(), 0);
		}

		triangleIndex += 32;
	}
	VisiblechildrenNodes.Empty();
}

void Quadtree::InitialiseNodes(TSharedPtr<QuadtreeNode> parentNode)
{	

	double halfRadius = parentNode->GetRadius() * 0.5;
	UE_LOG(LogTemp, Warning, TEXT("parentNode->GetLOD(): %d"), parentNode->GetLOD());
	if (parentNode->GetLOD() <= Thresholds.Num()) {
		//UE_LOG(LogTemp, Warning, TEXT("Generate nodes for LOD level: %d"), parentNode->GetLOD());
		// Bottom right node

		FVector tlPos = parentNode->GetPosition() + (AxisA - AxisB) * halfRadius;
		FVector trPos = parentNode->GetPosition() + (AxisA + AxisB) * halfRadius;
		FVector brPos = parentNode->GetPosition() + (AxisB - AxisA) * halfRadius;
		FVector blPos = parentNode->GetPosition() - (AxisB + AxisA) * halfRadius;

		TSharedPtr<QuadtreeNode> br = CreateNode(parentNode, brPos, halfRadius, parentNode->NodeLOD + 1, parentNode->GetLocalUp(), AxisA, AxisB, 4);
		TSharedPtr<QuadtreeNode> bl = CreateNode(parentNode, blPos, halfRadius, parentNode->NodeLOD + 1, parentNode->GetLocalUp(), AxisA, AxisB, 3);
		TSharedPtr<QuadtreeNode> tr = CreateNode(parentNode, trPos, halfRadius, parentNode->NodeLOD + 1, parentNode->GetLocalUp(), AxisA, AxisB, 2);
		TSharedPtr<QuadtreeNode> tl = CreateNode(parentNode, tlPos, halfRadius, parentNode->NodeLOD + 1, parentNode->GetLocalUp(), AxisA, AxisB, 1);

		UpdateMesh(tl);
		UpdateMesh(tr);
		UpdateMesh(bl);
		UpdateMesh(br);


	}
	//else if ((parentNode->Texture)) {
	//	//UE_LOG(LogTemp, Warning, TEXT("Generate nodes for LOD level: %d"), parentNode->GetLOD());
	//	// Bottom right node

	//	FVector tlPos = parentNode->GetPosition() + (AxisA - AxisB) * halfRadius;
	//	FVector trPos = parentNode->GetPosition() + (AxisA + AxisB) * halfRadius;
	//	FVector brPos = parentNode->GetPosition() + (AxisB - AxisA) * halfRadius;
	//	FVector blPos = parentNode->GetPosition() - (AxisB + AxisA) * halfRadius;

	//	TSharedPtr<QuadtreeNode> br = CreateNode(parentNode, brPos, halfRadius, parentNode->NodeLOD + 1, parentNode->GetLocalUp(), AxisA, AxisB, 4);
	//	TSharedPtr<QuadtreeNode> bl = CreateNode(parentNode, blPos, halfRadius, parentNode->NodeLOD + 1, parentNode->GetLocalUp(), AxisA, AxisB, 3);
	//	TSharedPtr<QuadtreeNode> tr = CreateNode(parentNode, trPos, halfRadius, parentNode->NodeLOD + 1, parentNode->GetLocalUp(), AxisA, AxisB, 2);
	//	TSharedPtr<QuadtreeNode> tl = CreateNode(parentNode, tlPos, halfRadius, parentNode->NodeLOD + 1, parentNode->GetLocalUp(), AxisA, AxisB, 1);

	//	UpdateMesh(tl);
	//	UpdateMesh(tr);
	//	UpdateMesh(bl);
	//	UpdateMesh(br);


	//}
}

TSharedPtr<QuadtreeNode> Quadtree::CreateNode(TSharedPtr<QuadtreeNode> parent, FVector Position, double radius, int lodLevel, FVector localUp, FVector axisA, FVector axisB, int pos)
{
	// Bottom left node
	TSharedPtr<QuadtreeNode> newNode = MakeShared<QuadtreeNode>();
	if (parent.IsValid())
	{
		parent->AddChildNode(newNode);
		newNode->SetParentNode(TSharedPtr<QuadtreeNode>(parent));
	}
	else {
		newNode->SetParentNode(nullptr);
	}

	newNode->initialiseNode(QT_Actor, StaticProvider,StaticProviderTree, parent, Position, radius, lodLevel, localUp, axisA, axisB, pos);
	return newNode;
}