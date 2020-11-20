// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCableComponent;
class USphereComponent;
class USpringArmComponent;
class UCameraComponent;
class ASWeapon;

UCLASS()
class HORDEMODE_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Landed(const FHitResult& Hit) override;

	void MoveForward(float Value);
	void MoveRight(float Value);

	void BeginCrouch();
	void EndCrouch();

	void BeginJump();
	void EndJump();
	void ResetJumps();

	void Sprint();
	void StopSprint();

	void BeginZoom();
	void EndZoom();

	void BeginFire();
	void EndFire();

	bool bIsFiring;

	ASWeapon* CurrentWeapon;

	UPROPERTY(VisibleDefaultsOnly, Category = "Shooting")
	FName WeaponAttachSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Shooting")
	TSubclassOf<ASWeapon> StarterWeapon;

	void UpdateWeapon(float DeltaTime);

	float FireTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(EditAnywhere, Category = "Movement|Jumping")
	int JumpCount;

	UPROPERTY(EditAnywhere, Category = "Movement|Jumping")
	int MaxJumps;

	UPROPERTY(EditAnywhere, Category = "Movement|Sprinting")
	float SprintSpeedMultiplier;

	bool bWantsADS;

	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, Category= "Shooting")
	float ADSFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Shooting", meta = (ClampMin = 0.1, ClampMax = 100))
	float ADSInterpSpeed;

	//Grapple stuff
	UPROPERTY(VisibleAnywhere, Category = "Grappling|Components")
	    USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Grappling|Components")
		class UCableComponent* CableComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grappling|Components")
		UBlueprint* HookPointActor; // Actor HookPoint

	UPROPERTY(EditAnywhere, Category = "Grappling|Variables")
	    float HookLength; // 5000

	UPROPERTY(EditAnywhere, Category = "Grappling|Variables")
	    float GrappleSpeed; // -2

	UPROPERTY(EditAnywhere, Category = "Grappling|Variables")
		float ReelingSpeed; // -2


	bool bGrappleConnected;
	FVector HookPoint;
	AActor* GrapplingActor;

	void Grapple();
	void UpdateGrapple();
	void ReelIn();
	void ReelOut();
	void StopReelIn();
	void StopReelOut();
	void UpdateReeling();

	bool bReelIn;
	bool bReelOut;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

};
