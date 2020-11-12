// Fill out your copyright notice in the Description page of Project Settings.

#include "HordeMode/Public/SCharacter.h"

#include "CableComponent.h"
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
	
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(RootComponent);

	CableComp = CreateDefaultSubobject<UCableComponent>(TEXT("CableComp"));
	CableComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	JumpMaxCount = MaxJumps;
	bGrappleConnected = false;
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateReeling();
	UpdateGrapple();
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

void ASCharacter::Grapple()
{
	if(bGrappleConnected == false)
	{
		FHitResult hit;
		FVector start = SphereComp->GetComponentLocation();
		FVector end = start + (CameraComp->GetForwardVector() * HookLength);
		if(GetWorld()->SweepSingleByChannel(hit, start, end, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(15.0f)))
		{
			UE_LOG(LogTemp, Warning, TEXT("Sweep succesfull"));
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
		//SwingForce *= 10000.0f;

		//ForceComp->AddForce(SwingForce);

	}
	else
	{
		GetCharacterMovement()->GravityScale = 1.0f;
		//bSimGravityDisabled = false;
	}
}

void ASCharacter::ReelIn()
{
	if (bGrappleConnected == true)
	{
		bReelIn = true;
	}
}

void ASCharacter::ReelOut()
{
	if (bGrappleConnected == true)
	{
		bReelOut = true;
	}
}

void ASCharacter::StopReelIn()
{
	if (bGrappleConnected == true)
	{
		bReelIn = false;
	}
}

void ASCharacter::StopReelOut()
{
	if (bGrappleConnected == true)
	{
		bReelOut = false;
	}
}

void ASCharacter::UpdateReeling()
{
    if(bGrappleConnected == true)
    {
		if(FVector::Dist(GetActorLocation(), HookPoint) > 50.0f)
		{
			if (bReelOut == true)
			{
				UE_LOG(LogTemp, Warning, TEXT("Reeling Out"));
				FVector Dir = GetActorLocation() - HookPoint;
				Dir.Normalize(0.0001);

				Dir *= ReelingSpeed;
				LaunchCharacter(Dir, false, false);
			}
			else if (bReelIn == true)
			{
				UE_LOG(LogTemp, Warning, TEXT("Reeling In"));
				FVector Dir = GetActorLocation() - HookPoint;
				Dir.Normalize(0.0001);
				Dir *= -1.0f;

				Dir *= ReelingSpeed;
				LaunchCharacter(Dir, false, false);
			}
		}
    }
}
