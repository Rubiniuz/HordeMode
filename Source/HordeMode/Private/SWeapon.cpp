// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"

static bool DebugWeaponDrawing = false;
FAutoConsoleVariableRef CVARDebugWeaopnDrawing(TEXT("Horde.DebugWeapons"), DebugWeaponDrawing, TEXT("Draws Debug for Weapons"), ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "BeamEnd";
}

void ASWeapon::Fire()
{
    //Trace world from pawn eyes to crosshair

	AActor* MyOwner = GetOwner();

	if(MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		//fire from weapon not from camera
		/*FTransform MuzzleTransform = MeshComp->GetSocketTransform(MuzzleSocketName);

		FVector EyeLocation = MuzzleTransform.GetLocation();
		FRotator EyeRotation = MuzzleTransform.Rotator();

		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);*/
		FTransform MuzzleTransform = MeshComp->GetSocketTransform(MuzzleSocketName);
		FVector MuzzleLoc = MuzzleTransform.GetLocation();

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;

		//ParticleSystem target param
		TracerEndPoint = TraceEnd;

		FHitResult Hit;
		if(GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
		{
		    // Hit blocked // add damage

			AActor* HitActor = Hit.GetActor();

			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

			if(ImpactFX)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}

			TracerEndPoint = Hit.ImpactPoint;
		}

		//temp line : #Include "DrawDebugHelpers.h
		if(DebugWeaponDrawing == true)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
		}

		PlayFireFX();

	}
	
}

void ASWeapon::PlayFireFX()
{
	if (MuzzleFX)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleFX, MeshComp, MuzzleSocketName);
	}

	if (TracerFX)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerFX, MuzzleLocation);

		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if(MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if(PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}