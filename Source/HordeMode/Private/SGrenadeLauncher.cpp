// Fill out your copyright notice in the Description page of Project Settings.


#include "SGrenadeLauncher.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"

void ASGrenadeLauncher::Fire()
{

	// try and fire a projectile
	if (Projectile)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation("MuzzleSocket");
		FRotator MuzzleRotation = MeshComp->GetSocketRotation("MuzzleSocket");

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ActorSpawnParams.Owner = this->GetOwner();

		AActor* MyOwner = GetOwner();

		if (MyOwner)
		{
			FVector EyeLocation;
			FRotator EyeRotation;
			MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
			// spawn the projectile at the muzzle
			GetWorld()->SpawnActor<ASProjectile>(Projectile, MuzzleLocation, EyeRotation, ActorSpawnParams);
		}
	}

}