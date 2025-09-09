#include "Player/Camera/BasePlayerCameraManager.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/ArrowComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Player/BasePlayer.h"

UBasePlayerCameraManager::UBasePlayerCameraManager()
{
	PrimaryComponentTick.bCanEverTick = true;	
}

void UBasePlayerCameraManager::OnRegister()
{
	Super::OnRegister();
	
	CameraBoom = NewObject<USpringArmComponent>(GetOwner(), USpringArmComponent::StaticClass(), TEXT("SpringArm"));
	FollowCamera = NewObject<UCameraComponent>(GetOwner(), UCameraComponent::StaticClass(), TEXT("Camera"));
	CameraUnEquipLook = NewObject<UArrowComponent>(GetOwner(), UArrowComponent::StaticClass(), TEXT("CameraUnequipLook"));
	CameraEquipLook = NewObject<UArrowComponent>(GetOwner(), UArrowComponent::StaticClass(), TEXT("CameraEquipLook"));
	CameraZoomInLook = NewObject<UArrowComponent>(GetOwner(), UArrowComponent::StaticClass(), TEXT("CameraZoomInLook"));
	CameraRightMoveLook = NewObject<UArrowComponent>(GetOwner(), UArrowComponent::StaticClass(), TEXT("CameraRightMoveLook"));
	CameraLeftMoveLook = NewObject<UArrowComponent>(GetOwner(), UArrowComponent::StaticClass(), TEXT("CameraLeftMoveLook"));
}

void UBasePlayerCameraManager::BeginPlay()
{
	Super::BeginPlay();
	CameraBoom->bUsePawnControlRotation = true;
	// 카메라가 늦게 따라오는 설정
	//CameraBoom->bEnableCameraLag = true;
	//CameraBoom->bEnableCameraRotationLag = true;

	FollowCamera->bUsePawnControlRotation = false;

	CameraBoomSocketLocation = CameraBoom->GetRelativeLocation();
	CameraBoomSocketLocation.X -= CameraBoom->TargetArmLength;

	CameraBoom->SocketOffset = CameraUnEquipLook->GetRelativeLocation() - CameraBoomSocketLocation;

	if (CameraChangeCurve)
	{
		FOnTimelineFloat   UpdateUnEquip;
		FOnTimelineEvent   FinishedUnEquip;

		UpdateUnEquip.BindUFunction(this, FName("OnUnEquipUpdate"));
		FinishedUnEquip.BindUFunction(this, FName("OnUnEquipFinished"));

		UnEquipTimeline.AddInterpFloat(CameraChangeCurve, UpdateUnEquip);             // 트랙 추가
		UnEquipTimeline.SetTimelineFinishedFunc(FinishedUnEquip);                     // 완료 델리게이트
		UnEquipTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
		UnEquipTimeline.SetLooping(false);

		FOnTimelineFloat   UpdateEquip;
		FOnTimelineEvent   FinishedEquip;

		UpdateEquip.BindUFunction(this, FName("OnEquipUpdate"));
		FinishedEquip.BindUFunction(this, FName("OnEquipFinished"));

		EquipTimeline.AddInterpFloat(CameraChangeCurve, UpdateEquip);             // 트랙 추가
		EquipTimeline.SetTimelineFinishedFunc(FinishedEquip);                     // 완료 델리게이트
		EquipTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
		EquipTimeline.SetLooping(false);

		FOnTimelineFloat   UpdateZoomIn;
		FOnTimelineEvent   FinishedZoomIn;

		UpdateZoomIn.BindUFunction(this, FName("OnZoomInUpdate"));
		FinishedZoomIn.BindUFunction(this, FName("OnZoomInFinished"));

		ZoomInTimeline.AddInterpFloat(CameraChangeCurve, UpdateZoomIn);             // 트랙 추가
		ZoomInTimeline.SetTimelineFinishedFunc(FinishedZoomIn);                     // 완료 델리게이트
		ZoomInTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
		ZoomInTimeline.SetLooping(false);
	}
}

void UBasePlayerCameraManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UnEquipTimeline.TickTimeline(DeltaTime);
	EquipTimeline.TickTimeline(DeltaTime);
	ZoomInTimeline.TickTimeline(DeltaTime);
}

void UBasePlayerCameraManager::SetCameraMode(ECameraMode _NewMode)
{
	if (CurrentCameraMode == _NewMode ||
		UnEquipTimeline.IsPlaying()||
		EquipTimeline.IsPlaying() ||
		ZoomInTimeline.IsPlaying())
		return;

	switch (_NewMode)
	{
	case ECameraMode::UnEquip:
	{
		UnEquipTimeline.PlayFromStart();
	}
		break;

	case ECameraMode::Equip:
	{
		EquipTimeline.PlayFromStart();
	}
		break;

	case ECameraMode::ZoomIn:
	{
		ZoomInTimeline.PlayFromStart();
	}
		break;

	default:
		break;
	}
}

void UBasePlayerCameraManager::MoveCameraRight()
{
	if (CurrentCameraMode != ECameraMode::Equip) return;


}

void UBasePlayerCameraManager::MoveCameraLeft()
{
	if (CurrentCameraMode != ECameraMode::Equip) return;


}

void UBasePlayerCameraManager::OnUnEquipUpdate(float _Value)
{
	auto TargetPos = -CameraBoomSocketLocation + CameraUnEquipLook->GetRelativeLocation();
	auto TargetRot = CameraUnEquipLook->GetRelativeRotation();
	LerpCameraOffset(TargetPos, TargetRot, _Value);
}

void UBasePlayerCameraManager::OnUnEquipFinished()
{
	Cast<ABasePlayer>(GetOwner())->ZeroControllerPitch();

	CurrentCameraMode = ECameraMode::UnEquip;
}

void UBasePlayerCameraManager::OnEquipUpdate(float _Value)
{
	auto TargetPos = -CameraBoomSocketLocation + CameraEquipLook->GetRelativeLocation();
	auto TargetRot = CameraEquipLook->GetRelativeRotation();
	LerpCameraOffset(TargetPos, TargetRot, _Value);
}

void UBasePlayerCameraManager::OnEquipFinished()
{
	Cast<ABasePlayer>(GetOwner())->ZeroControllerPitch();

	CurrentCameraMode = ECameraMode::Equip;
}

void UBasePlayerCameraManager::OnZoomInUpdate(float _Value)
{
	auto TargetPos = -CameraBoomSocketLocation + CameraZoomInLook->GetRelativeLocation();
	auto TargetRot = CameraZoomInLook->GetRelativeRotation();
	LerpCameraOffset(TargetPos, TargetRot, _Value);
}

void UBasePlayerCameraManager::OnZoomInFinished()
{
	Cast<ABasePlayer>(GetOwner())->ZeroControllerPitch();

	CurrentCameraMode = ECameraMode::ZoomIn;
}

void UBasePlayerCameraManager::LerpCameraOffset(const FVector& _TargetPos, const FRotator& _TargetRot, float _Value)
{
	// SocketOffset : Camera가 달려있는 실제 위치
	// SpringArm의 로컬 좌표계 상에서의 Camera 위치가 되시겠다
	auto Current = CameraBoom->SocketOffset;
	auto NewLoc = FMath::Lerp(Current, _TargetPos, _Value);
	CameraBoom->SocketOffset = NewLoc;

	auto Contorller = Cast<ACharacter>(GetOwner())->GetController();
	auto Rotator = Contorller->GetControlRotation();

	if (Rotator.Pitch >= 180.f && Rotator.Pitch < 520.f)
	{
		Rotator.Pitch = FMath::Lerp(Rotator.Pitch, 360.f, _Value);
	}
	else if (Rotator.Pitch >= -180.f && Rotator.Pitch < 180.f)
	{
		Rotator.Pitch = FMath::Lerp(Rotator.Pitch, _TargetRot.Pitch, _Value);
	}
	else if (Rotator.Pitch >= -520.f && Rotator.Pitch < -180.f)
	{
		Rotator.Pitch = FMath::Lerp(Rotator.Pitch, -360.f, _Value);
	}

	Contorller->SetControlRotation(Rotator);
}
