#pragma once

// Custom includes
#include "QuadtreeNode.h"
#include "Templates/SharedPointer.h"
#include "Runtime/Engine/Classes/Components/ActorComponent.h"
#include "Components/RuntimeMeshComponentStatic.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include <Exjobb2/MyActor.h>
#include "Kismet/GameplayStatics.h"


#include "CoreMinimal.h"

class EXJOBB2_API Quadtree
{

public:
	Quadtree();
	Quadtree(AMyActor* in, FVector inlocalUp, double inRadius, int inLodLevel);
	~Quadtree();

	TSharedPtr<QuadtreeNode> GetRootNode();
	double GetDistance(FVector A, FVector B);

	TSharedPtr<QuadtreeNode> GetNode(FVector position);

	int GetMaxDistance() const;

	void UpdateMesh(TSharedPtr<QuadtreeNode> CurrentNode);

	void GetVisibleChildren(TSharedPtr<QuadtreeNode> CurrentNode);

	void GenerateTerrain(TArray<TSharedPtr<QuadtreeNode>> inChilds);

	bool AreSame(double a, double b);

	int GetNeighbourLOD(TSharedPtr<QuadtreeNode> CurrentNode);

	TArray<TSharedPtr<QuadtreeNode>> VisiblechildrenNodes;
	FVector LocalUp;
	double PlanetRadius;
	int PlanetLod;
	FVector AxisA;
	FVector AxisB;
	double PlanetMax;

	TArray<double> Thresholds;

private:
	void InitialiseNodes(TSharedPtr<QuadtreeNode> parent);

	TSharedPtr<QuadtreeNode> CreateNode(TSharedPtr<QuadtreeNode> parent, FVector Position, double radius, int lodLevel, FVector localUp, FVector axisA, FVector axisB, int pos);
private:
	// Root Node
	URuntimeMeshProviderStatic* StaticProvider;
	TSharedPtr<QuadtreeNode> RootNode;
	AMyActor* QT_Actor;
	int MaxDistance;

};