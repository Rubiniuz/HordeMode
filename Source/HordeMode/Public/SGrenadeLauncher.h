// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SProjectile.h"
#include "SWeapon.h"
#include "SGrenadeLauncher.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

UCLASS()
class HORDEMODE_API ASGrenadeLauncher : public ASWeapon
{
	GENERATED_BODY()

protected:

	void Fire() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, CATEGORY = "Weapon|Projectile")
	TSubclassOf<ASProjectile> Projectile;

};
