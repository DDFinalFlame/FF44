#include "Player/Camera/CameraManager.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/ArrowComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Player/BasePlayer.h"

UCameraManager::UCameraManager()
{
	PrimaryComponentTick.bCanEverTick = true;	
}

void UCameraManager::OnRegister()
{
	Super::OnRegister();
	
	CameraBoom = NewObject<USpringArmComponent>(GetOwner(), USpringArmComponent::StaticClass(), TEXT("SpringArm"));
	FollowCamera = NewObject<UCameraComponent>(GetOwner(), UCameraComponent::StaticClass(), TEXT("Camera"));
	CameraDefaultLook = NewObject<UArrowComponent>(GetOwner(), UArrowComponent::StaticClass(), TEXT("CameraDefaultLook"));		
	CameraZoomInLook = NewObject<UArrowComponent>(GetOwner(), UArrowComponent::StaticClass(), TEXT("CameraZoomInLook"));
}

void UCameraManager::BeginPlay()
{
	Super::BeginPlay();
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = false; // 카메라 충돌 테스트 비활성화
	// 카메라가 늦게 따라오는 설정
	//CameraBoom->bEnableCameraLag = true;
	//CameraBoom->bEnableCameraRotationLag = true;

	FollowCamera->bUsePawnControlRotation = false;

	if (CameraChangeCurve)
	{
		FOnTimelineFloat   UpdateDefault;
		FOnTimelineEvent   FinishedDefault;

		UpdateDefault.BindUFunction(this, FName("OnDefaultUpdate"));
		FinishedDefault.BindUFunction(this, FName("OnDefaultFinished"));

		DefaultTimeline.AddInterpFloat(CameraChangeCurve, UpdateDefault);             // 트랙 추가
		DefaultTimeline.SetTimelineFinishedFunc(FinishedDefault);                     // 완료 델리게이트
		DefaultTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
		DefaultTimeline.SetLooping(false);

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

void UCameraManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DefaultTimeline.TickTimeline(DeltaTime);
	ZoomInTimeline.TickTimeline(DeltaTime);
}

void UCameraManager::ChangeDefaultCamera()
{
	DefaultTimeline.PlayFromStart();

	if (ZoomInTimeline.IsPlaying())
		ZoomInTimeline.Stop();
}

void UCameraManager::ChangeZoomInCamera()
{
	ZoomInTimeline.PlayFromStart();

	if (DefaultTimeline.IsPlaying())
		DefaultTimeline.Stop();
}

void UCameraManager::SetCameraMode(ECameraMode _NewMode)
{
	if (CurrentCameraMode == _NewMode ||
		DefaultTimeline.IsPlaying() ||
		ZoomInTimeline.IsPlaying())
		return;

	switch (_NewMode)
	{
	case ECameraMode::Default:
		ChangeDefaultCamera();
		break;

	case ECameraMode::ZoomIn:
		ChangeZoomInCamera();
		break;

	default:
		break;
	}
}

void UCameraManager::OnDefaultUpdate(float _Value)
{
	auto Target = FVector::ZeroVector;
	LerpCameraBoomSocketOffset(Target, _Value);
}

void UCameraManager::OnDefaultFinished()
{
	Cast<ABasePlayer>(GetOwner())->ZeroControllerPitch();

	CurrentCameraMode = ECameraMode::Default;
}

void UCameraManager::OnZoomInUpdate(float _Value)
{
	auto Target = -CameraDefaultLook->GetRelativeLocation() + CameraZoomInLook->GetRelativeLocation();
	LerpCameraBoomSocketOffset(Target, _Value);
}

void UCameraManager::OnZoomInFinished()
{
	Cast<ABasePlayer>(GetOwner())->ZeroControllerPitch();

	CurrentCameraMode = ECameraMode::ZoomIn;
}

void UCameraManager::LerpCameraBoomSocketOffset(const FVector& _Target, float _Value)
{
	// SocketOffset : Camera가 달려있는 실제 위치
	// SpringArm의 로컬 좌표계 상에서의 Camera 위치가 되시겠다
	auto Current = CameraBoom->SocketOffset;
	auto NewLoc = FMath::Lerp(Current, _Target, _Value);
	CameraBoom->SocketOffset = NewLoc;

	auto Contorller = Cast<ACharacter>(GetOwner())->GetController();
	auto Rotator = Contorller->GetControlRotation();

	if(Rotator.Pitch>=180.f)
		Rotator.Pitch -= 360.f;

	Rotator.Pitch = FMath::Lerp(Rotator.Pitch, 0.f, _Value);
	Contorller->SetControlRotation(Rotator);
}
