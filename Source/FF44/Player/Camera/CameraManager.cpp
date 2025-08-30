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
	CameraBoom->bDoCollisionTest = false; // ī�޶� �浹 �׽�Ʈ ��Ȱ��ȭ
	// ī�޶� �ʰ� ������� ����
	//CameraBoom->bEnableCameraLag = true;
	//CameraBoom->bEnableCameraRotationLag = true;

	FollowCamera->bUsePawnControlRotation = false;

	if (CameraChangeCurve)
	{
		FOnTimelineFloat   UpdateDefault;
		FOnTimelineEvent   FinishedDefault;

		UpdateDefault.BindUFunction(this, FName("OnDefaultUpdate"));
		FinishedDefault.BindUFunction(this, FName("OnDefaultFinished"));

		DefaultTimeline.AddInterpFloat(CameraChangeCurve, UpdateDefault);             // Ʈ�� �߰�
		DefaultTimeline.SetTimelineFinishedFunc(FinishedDefault);                     // �Ϸ� ��������Ʈ
		DefaultTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
		DefaultTimeline.SetLooping(false);

		FOnTimelineFloat   UpdateZoomIn;
		FOnTimelineEvent   FinishedZoomIn;

		UpdateZoomIn.BindUFunction(this, FName("OnZoomInUpdate"));
		FinishedZoomIn.BindUFunction(this, FName("OnZoomInFinished"));

		ZoomInTimeline.AddInterpFloat(CameraChangeCurve, UpdateZoomIn);             // Ʈ�� �߰�
		ZoomInTimeline.SetTimelineFinishedFunc(FinishedZoomIn);                     // �Ϸ� ��������Ʈ
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
	// SocketOffset : Camera�� �޷��ִ� ���� ��ġ
	// SpringArm�� ���� ��ǥ�� �󿡼��� Camera ��ġ�� �ǽðڴ�
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
