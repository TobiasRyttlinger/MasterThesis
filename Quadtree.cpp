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

	Thresholds.Add(INFINITY); //1
	Thresholds.Add(PlanetRadius / 1); //2
	Thresholds.Add(PlanetRadius / 2); //3
	Thresholds.Add(PlanetRadius / 3); //4
	Thresholds.Add(PlanetRadius / 4); //5
	Thresholds.Add(PlanetRadius / 5); //6
	Thresholds.Add(PlanetRadius / 6); //7
	Thresholds.Add(PlanetRadius / 7); //8
	Thresholds.Add(PlanetRadius / 7); //9
	Thresholds.Add(PlanetRadius / 7); //10
	//Thresholds.Add(PlanetRadius * FMath::Pow(0.45, 5)); //0


	AxisA = FVector(inlocalUp.Y, inlocalUp.Z, inlocalUp.X);
	AxisB = FVector::CrossProduct(inlocalUp, AxisA);
	PlanetMax = PlanetRadius * 1.00000139119;
	RootNode = CreateNode(nullptr, LocalUp * PlanetRadius, PlanetRadius, PlanetLod, LocalUp, AxisA, AxisB, 0);
	RootNode->SetNodePosition(10);
	//RootNode->GenerateNodeMesh(in, StaticProvider, inlocalUp, 0);

	InitialiseNodes(RootNode);
	//GetVisibleChildren(RootNode);
}

Quadtree::~Quadtree()
{

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
	return fabs(a - b) < 0.000001f;
}

void Quadtree::UpdateMesh(TSharedPtr<QuadtreeNode> CurrentNode) {

	FVector CameraPos = QT_Actor->GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();

	FVector MeshPos = CurrentNode->GetPosition();
	MeshPos.Normalize(1.0f);
	MeshPos = MeshPos * PlanetRadius;

	//Distance between camera and meshcenter
	double distance = FVector::Dist(MeshPos, CameraPos);
	FVector BoxCenter, origin;
	QT_Actor->GetActorBounds(false, origin, BoxCenter);
	double CameraHeight = GetDistance(FVector(0, 0, 0), CameraPos);
	double HorizonDistance = FMath::Sqrt(FMath::Pow(CameraHeight, 2) - FMath::Pow(this->PlanetRadius, 2));
	double OverTheHorizonDist = FMath::Sqrt(FMath::Pow(this->PlanetMax, 2) - FMath::Pow(this->PlanetRadius, 2));
	double VisibilitySphere = HorizonDistance + OverTheHorizonDist;

	//UE_LOG(LogTemp, Warning, TEXT("CurrentNode->GetPosition(): %s"), *CurrentNode->GetPosition().ToString());
	//UE_LOG(LogTemp, Warning, TEXT("MeshPos: %s"), *MeshPos.ToString());
	//UE_LOG(LogTemp, Warning, TEXT("distance: %f"), distance);

	if (distance < CurrentNode->NodeRadius * 2.0f) {
		if (CurrentNode->GetChildNodes().Num() > 0) {
			CurrentNode->Rendered = false;
			for (auto childs : CurrentNode->GetChildNodes()) {
				UpdateMesh(childs);
			}
		}
		else {
			InitialiseNodes(CurrentNode);

		}
	}
	else {
		for (auto childs : CurrentNode->GetChildNodes()) {
			if (childs->Rendered == true) {
				if (childs->SectionID > 0) {
					StaticProvider->RemoveSection(0, childs->SectionID);
				}
			}
		}
		if (CurrentNode->GetChildNodes().Num() > 0) {
			CurrentNode->ClearChildren();
		}
		GetVisibleChildren(CurrentNode);

	}
}

void Quadtree::GetVisibleChildren(TSharedPtr<QuadtreeNode> CurrentNode) {

	FVector CameraPos = QT_Actor->GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	FVector MeshPos = CurrentNode->GetPosition();
	MeshPos.Normalize(1.0f);
	MeshPos = MeshPos * PlanetRadius;
	double distance = GetDistance(CameraPos, CurrentNode->GetPosition());

	FVector BoxCenter, origin;
	QT_Actor->GetActorBounds(false, origin, BoxCenter);
	double CameraHeight = GetDistance(origin, CameraPos);
	double HorizonDistance = FMath::Sqrt(FMath::Pow(CameraHeight, 2) - FMath::Pow(this->PlanetRadius, 2));
	double OverTheHorizonDist = FMath::Sqrt(FMath::Pow(this->PlanetMax, 2) - FMath::Pow(this->PlanetRadius, 2));
	double VisibilitySphere = HorizonDistance + OverTheHorizonDist;

	float Angle = FMath::Acos((FMath::Pow(PlanetRadius, 2) + FMath::Pow(CameraHeight, 2) - FMath::Pow(distance, 2)) / (2 * PlanetRadius * CameraHeight));


	if (Angle < 1.45f && VisibilitySphere < distance) {

		VisiblechildrenNodes.Add(CurrentNode);
		CurrentNode->Rendered = true;
	}



}


void Quadtree::GenerateTerrain(TArray<TSharedPtr<QuadtreeNode>> inNodes) {
	if (inNodes.Num() == 0) {
		return;
	}
	int triangleIndex = 0;
	for (auto Node : inNodes) {
		if (Node->GetVertices().Num() > 0) {
			Node->GenerateNodeMesh(QT_Actor, StaticProvider, Node->GetLocalUp(), triangleIndex);
		}
		else {
			Node->GenerateNodeMesh(QT_Actor, StaticProvider, Node->GetLocalUp(), 0);
		}

		triangleIndex +=16;
	}
	VisiblechildrenNodes.Empty();
	StaticProvider->MarkAllLODsDirty();

}

void Quadtree::InitialiseNodes(TSharedPtr<QuadtreeNode> parentNode)
{
	double halfRadius = parentNode->GetRadius() * 0.5f;
	if (parentNode->NodeLOD < Thresholds.Num()) {
		UE_LOG(LogTemp, Warning, TEXT("Generate nodes for LOD level: %d"), parentNode->GetLOD());
		// Bottom right node
		FVector brPos = parentNode->GetPosition() - (AxisA - AxisB) * halfRadius;
		FVector blPos = parentNode->GetPosition() - (AxisA + AxisB) * halfRadius;
		FVector trPos = parentNode->GetPosition() + (AxisA + AxisB) * halfRadius;
		FVector tlPos = parentNode->GetPosition() + (AxisA - AxisB) * halfRadius;



		TSharedPtr<QuadtreeNode> br = CreateNode(parentNode, brPos, halfRadius, parentNode->NodeLOD + 1, parentNode->GetLocalUp(), AxisA, AxisB, 3);
		TSharedPtr<QuadtreeNode> bl = CreateNode(parentNode, blPos, halfRadius, parentNode->NodeLOD + 1, parentNode->GetLocalUp(), AxisA, AxisB, 4);
		TSharedPtr<QuadtreeNode> tr = CreateNode(parentNode, trPos, halfRadius, parentNode->NodeLOD + 1, parentNode->GetLocalUp(), AxisA, AxisB, 1);
		TSharedPtr<QuadtreeNode> tl = CreateNode(parentNode, tlPos, halfRadius, parentNode->NodeLOD + 1, parentNode->GetLocalUp(), AxisA, AxisB, 2);

		if (parentNode->NodeLOD < Thresholds.Num()) {
			UpdateMesh(tr);
			UpdateMesh(tl);
			UpdateMesh(br);
			UpdateMesh(bl);
		}
	}


}

TSharedPtr<QuadtreeNode> Quadtree::CreateNode(TSharedPtr<QuadtreeNode> parent, FVector Position, double radius, int lodLevel, FVector localUp, FVector axisA, FVector axisB, int pos)
{
	// Bottom left node
	TSharedPtr<QuadtreeNode> newNode = MakeShared<QuadtreeNode>();
	if (parent.IsValid())
	{
		parent->AddChildNode(newNode);
	}
	newNode->SetParentNode(TSharedPtr<QuadtreeNode>(parent));
	newNode->initialiseNode(parent, Position, radius, lodLevel, localUp, axisA, axisB, pos);
	return newNode;
}