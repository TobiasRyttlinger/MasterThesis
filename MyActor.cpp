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
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
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

}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{

     if(counter >= 0.1f) {

    QT_1.UpdateMesh(QT_1.GetRootNode());
    QT_2.UpdateMesh(QT_2.GetRootNode());
    QT_3.UpdateMesh(QT_3.GetRootNode());
    QT_4.UpdateMesh(QT_4.GetRootNode());
    QT_5.UpdateMesh(QT_5.GetRootNode());
    QT_6.UpdateMesh(QT_6.GetRootNode());

    QT_1.GenerateTerrain(QT_1.VisiblechildrenNodes);
    QT_2.GenerateTerrain(QT_2.VisiblechildrenNodes);
    QT_3.GenerateTerrain(QT_3.VisiblechildrenNodes);
    QT_4.GenerateTerrain(QT_4.VisiblechildrenNodes);
    QT_5.GenerateTerrain(QT_5.VisiblechildrenNodes);
    QT_6.GenerateTerrain(QT_6.VisiblechildrenNodes);

    counter = 0;
     } 
       // GEngine->ForceGarbageCollection(true);
    counter += DeltaTime;
    Super::Tick(DeltaTime);


}

