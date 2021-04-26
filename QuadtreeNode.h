#pragma once

// Custom includes
#include "Templates/SharedPointer.h"
#include "Components/RuntimeMeshComponentStatic.h"
#include "Providers/RuntimeMeshProviderStatic.h"
#include "Providers/RuntimeMeshProviderSphere.h"
#include "Containers/Array.h"
#include "MyActor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/DefaultValueHelper.h"
#include "Math/Vector.h"
#include "ImageUtils.h"
#include "Serialization/StructuredArchive.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "HAL/UnrealMemory.h"
#include "DDSLoader.h"
// Core includes
#include "CoreMinimal.h"


class EXJOBB2_API QuadtreeNode
{
public:
	QuadtreeNode();
	QuadtreeNode(const QuadtreeNode& copy);
	~QuadtreeNode();

	/** Calculate Array of pixelvalues. */
	void CalculateHeightMap();
	/** Texture streaming function. */
	void LoadTextureFromPath(const FString& FullFilePath);

	/**Runtime Mesh Component for this node*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		URuntimeMeshComponentStatic* RMC;



	/** Set the parent node for this node. */
	void SetParentNode(TSharedPtr<QuadtreeNode> parentNode);


	/** Initialise this node. */
	void initialiseNode(AMyActor* in, URuntimeMeshProviderStatic* StaticProviderIn, TSharedPtr<QuadtreeNode> Parent, FVector Position, double radius, int lodLevel, FVector localUp, FVector axisA, FVector axisB, int pos);

	/** Get the parent node to this node. */
	TSharedPtr<QuadtreeNode> GetParentNode();

	/* Add a child node to this node. */
	void AddChildNode(TSharedPtr<QuadtreeNode> node);

	/** Get child nodes belonging to this node. */
	TArray<TSharedPtr<QuadtreeNode>> GetChildNodes();

	/**Delete unused childnodes*/
	void ClearChildren();

	/** Draw a debug box to outline where the box is. */
	void DrawBoxAroundNode(UWorld* world, FColor colour);

	/** Get the distance from the root node of the tree. */
	int GetDistance();

	FString GetTexture(FVector VecUpin);
	/** Get the distance from the root node of the tree. */
	double GetRadius();

	/** Convert to cartesian coordinates from longitude and latitude. */
	FVector ToCartesian(double Longitude, double Latitude);

	/** Get the distance from the root node of the tree. */
	FVector GetPosition();

	/** Get the distance from the root node of the tree. */
	FVector GetLocalUp();

	/** Get the distance from the root node of the tree. */
	int GetLOD();

	/** Find if a given position is inside this node. */
	bool PositionInsideNode(FVector position);

	/** Get the node which contains a given position. */
	TSharedPtr<QuadtreeNode> GetNode(FVector position);

	/** Convert to LatLong Coordinates */
	FVector2D ToLatLong(FVector inPosition);


	/* Get a copy of the node position member variable. */
	int GetNodePosition() const;

	/** Set the value of a node position member variable. */
	void SetNodePosition(int position);

	/** Check if this node has any children nodes. */
	bool HasChildNodes() const;

	void ClearData();

	void GenerateNodeMesh(AMyActor* in, URuntimeMeshProviderStatic* StaticProvider, FVector LocalUp, int LodLevel);

	void readFile();

	void GenerateUVS(int Resolution, TArray<FVector> inVerts);

	TArray<FVector> GenerateVertices(AMyActor* in, TArray<FVector>VerticesIn, int Resolution, FVector localUp);

	int GetIndexForGridCoordinates(int x, int y, int NoiseSamplesPerLine);

	FVector2D GetPositionForGridCoordinates(int x, int y, int NoiseResolution);

	TArray<int> GenerateTriangles(int NoiseSamplesPerLine, TArray<int>Triangles, int TriangleOffset);
	FString GetHeightMap(FVector VecUpin);
	TArray<FVector> GetVertices();
	TArray<int> GetTriangles();
	TSharedPtr<QuadtreeNode> GetRootNode();
	/** LOD of the curent node mesh*/
	int NodeLOD;
	TSharedPtr<QuadtreeNode> parentNode;
	/** Radius of the curent node mesh*/
	double NodeRadius;
	int SectionID;
	bool Rendered;
	TArray<FVector2D> TexCoords;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterial* Mat = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* Texture = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterialInstanceDynamic* DynMat = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* HeightMapTexture = nullptr;



	int MeshResolution;
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

private:
	/** Get the root node of this quadtree. */


	/** Get the node which contains a given position. */
	TSharedPtr<QuadtreeNode> GetNodeContainingPosition(FVector2D position);


private:
	/** The bounding box for this node. */
	TSharedPtr<FBox2D> boundingBox;

	/** Child nodes held by this node. */
	TArray<TSharedPtr<QuadtreeNode>> childNodes;

	/** The position of this node relevant to it's parent. */
	int NodePosition;

	double GetNoiseValueForGridCoordinates(int x, int y);

	/** Position of the curent node mesh*/
	FVector Position;

	/** LocalUp of the curent node mesh*/
	FVector LocalUp;
	TArray<double> HeightMap;
	/** Mesh data of the curent node*/
	TArray<FVector> SphericalCoords;
	FVector2D LatLong;
	TArray<FVector> Wgs84;
	TArray<FVector> Vertices;
	TArray<int> Triangles;

	FVector AxisA;
	FVector AxisB;

	bool initialised;

};