// Fill out your copyright notice in the Description page of Project Settings.

#include "HordeMode/Public/SCharacter.h"

#include "CableComponent.h"
#include "SWeapon.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanJump = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

    JumpCount = 0;
	MaxJumps = 2;
	JumpMaxHoldTime = 0.1f;

	SprintSpeedMultiplier = 1.45f;

	ADSFOV = 65.0f;
	ADSInterpSpeed = 20.0f;

	FireTimer = 0.0f;
	
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(RootComponent);

	CableComp = CreateDefaultSubobject<UCableComponent>(TEXT("CableComp"));
	CableComp->SetupAttachment(RootComponent);

	WeaponAttachSocket = "WeaponSocket";
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	JumpMaxCount = MaxJumps;

	bWantsADS = false;
	DefaultFOV = CameraComp->FieldOfView;

	bGrappleConnected = false;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if(CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocket);
		FireTimer = CurrentWeapon->FiringSpeed;
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateWeapon(DeltaTime);

	UpdateReeling();
	UpdateGrapple();
}

FVector ASCharacter::GetPawnViewLocation() const
{
    if(CameraComp)
    {
		return CameraComp->GetComponentLocation();
    }

	return Super::GetPawnViewLocation();
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookRight", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed,this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::BeginJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ASCharacter::EndJump);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASCharacter::StopSprint);

	PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("ADS", IE_Released, this, &ASCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::BeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::EndFire);

	PlayerInputComponent->BindAction("Grapple", IE_Pressed, this, &ASCharacter::Grapple);
	PlayerInputComponent->BindAction("ReelIn", IE_Pressed, this, &ASCharacter::ReelIn);
	PlayerInputComponent->BindAction("ReelOut", IE_Pressed, this, &ASCharacter::ReelOut);
	PlayerInputComponent->BindAction("ReelIn", IE_Released, this, &ASCharacter::StopReelIn);
	PlayerInputComponent->BindAction("ReelOut", IE_Released, this, &ASCharacter::StopReelOut);

}

void ASCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

void ASCharacter::BeginJump()
{
	if(JumpCount < MaxJumps)
	{
		JumpCount += 1;
		Jump();
	}
}

void ASCharacter::EndJump()
{
	StopJumping();
}

void ASCharacter::ResetJumps()
{
	JumpCount = 0;
}

void ASCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	ResetJumps();
}

void ASCharacter::Sprint()
{
	GetCharacterMovement()->MaxWalkSpeed *= SprintSpeedMultiplier;
}

void ASCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed /= SprintSpeedMultiplier;
}

void ASCharacter::BeginZoom()
{
	bWantsADS = true;
}

void ASCharacter::EndZoom()
{
	bWantsADS = false;
}

void ASCharacter::BeginFire()
{
	bIsFiring = true;
}

void ASCharacter::EndFire()
{
	bIsFiring = false;
}

void ASCharacter::UpdateWeapon(float DeltaTime)
{
	float TargetFOV = bWantsADS ? ADSFOV : DefaultFOV;

	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ADSInterpSpeed);

	CameraComp->SetFieldOfView(NewFOV);

	FireTimer += DeltaTime;
	
	if (FireTimer >= CurrentWeapon->FiringSpeed)
	{
		if (bIsFiring == true)
		{
			CurrentWeapon->Fire();
			FireTimer = 0;
		}
	}
	
}


void ASCharacter::Grapple()
{
	if(bGrappleConnected == false)
	{
		FHitResult hit;
		FVector start = SphereComp->GetComponentLocation();
		FVector end = start + (CameraComp->GetForwardVector() * HookLength);
		if(GetWorld()->SweepSingleByChannel(hit, start, end, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(15.0f)))
		{
			HookPoint = hit.Location;
			bGrappleConnected = true;
			
			FTransform transform = UKismetMathLibrary::MakeTransform(HookPoint, FRotator::ZeroRotator, FVector::ZeroVector);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;

			GrapplingActor = GetWorld()->SpawnActor<AActor>(HookPointActor->GeneratedClass, transform, SpawnParams);
			CableComp->SetAttachEndTo(GrapplingActor, FName());

			CableComp->SetHiddenInGame(false, false);
		}
	}
	else if (bGrappleConnected == true)
	{
		bGrappleConnected = false;
		CableComp->SetHiddenInGame(true, false);
	}
}

void ASCharacter::UpdateGrapple()
{
	if(bGrappleConnected)
	{
		//bSimGravityDisabled = true;
		GetCharacterMovement()->GravityScale = 0.6f;

		UCharacterMovementComponent* ForceComp = GetCharacterMovement();

		FVector Dir = GetActorLocation() - HookPoint;

		float DotProduct = FVector::DotProduct(Dir, GetVelocity());

		Dir.Normalize(0.0001);

		Dir *= DotProduct;
		Dir *= GrappleSpeed;

		ForceComp->AddForce(Dir);

		//FVector SwingForce = CameraComp->GetForwardVector();
		FVector SwingForce = GetMesh()->GetRightVector();
		SwingForce *= 1000.0f;

		ForceComp->AddForce(SwingForce);

	}
	else
	{
		GetCharacterMovement()->GravityScale = 1.0f;
		//bSimGravityDisabled = false;
	}
}

void ASCharacter::ReelIn()
{
	bReelIn = true;
}

void ASCharacter::ReelOut()
{
	bReelOut = true;
}

void ASCharacter::StopReelIn()
{
	bReelIn = false;
}

void ASCharacter::StopReelOut()
{
	bReelOut = false;
}

void ASCharacter::UpdateReeling()
{
    if(bGrappleConnected == true)
    {
		if(FVector::Dist(GetActorLocation(), HookPoint) > 50.0f)
		{
			if (bReelOut == true)
			{
				FVector Dir = FVector(0,0,-0.5f);
				Dir.Z *= ReelingSpeed;

				LaunchCharacter(Dir, false, false);
			}
			else if (bReelIn == true)
			{
				FVector Dir = GetActorLocation() - HookPoint;
				Dir.Normalize(0.0001);
				Dir *= -1.0f;

				Dir *= ReelingSpeed;

				LaunchCharacter(Dir, false, false);
			}
		}
    }
}
