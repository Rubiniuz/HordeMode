// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

ASProjectile::ASProjectile()
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);

	// Set as root component
	RootComponent = CollisionComp;

	ExplosionComp = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionComp"));
	ExplosionComp->InitSphereRadius(350.0f);
	ExplosionComp->SetupAttachment(CollisionComp);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 1000.0f;
	ProjectileMovement->MaxSpeed = 1000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 5 seconds by default
	InitialLifeSpan = 2.1f;
}

// Called when the game starts or when spawned
void ASProjectile::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(TimerHandle_Explode, this, &ASProjectile::Explode, 2.0f);
}

void ASProjectile::Explode()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactFX, GetActorLocation());
	TArray<AActor*> Actors;
	ExplosionComp->GetOverlappingActors(Actors);
	for (AActor* Actor : Actors)
	{
		UGameplayStatics::ApplyDamage(Actor, 50.0f, GetOwner()->GetInstigatorController(), this, DamageType);
	}
	Destroy();
}
