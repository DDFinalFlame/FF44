// 해당 class 사용시, Category = "Cameras"의 Component들을 Pawn에 추가해줄 것

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"

#include "BasePlayerCameraManager.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UArrowComponent;

UENUM(BlueprintType)
enum class ECameraMode : uint8
{
	Default  UMETA(DisplayName = "Default"),
	ZoomIn   UMETA(DisplayName = "Zoom In"),
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FF44_API UBasePlayerCameraManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBasePlayerCameraManager();

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Timeline)
	ECameraMode CurrentCameraMode = ECameraMode::Default;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
	UArrowComponent* CameraDefaultLook;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
	UArrowComponent* CameraZoomInLook;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
	UArrowComponent* CameraRightMoveLook;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
	UArrowComponent* CameraLeftMoveLook;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Timeline)
	UCurveFloat* CameraChangeCurve;

	FTimeline DefaultTimeline;
	FTimeline ZoomInTimeline;

	FTimeline MoveRightTimeline;
	FTimeline MoveLeftTimeline;

public:
	// Call Functions
	UFUNCTION(BlueprintCallable, Category = "CameraChange")
	void SetCameraMode(ECameraMode _NewMode);

	UFUNCTION(BlueprintCallable, Category = "CameraMove")
	void MoveCameraRight();

	UFUNCTION(BlueprintCallable, Category = "CameraMove")
	void MoveCameraLeft();

public:
	// Getters
	UFUNCTION(BlueprintCallable, Category = "CameraChange")
	ECameraMode GetCurrentCameraMode() const { return CurrentCameraMode; }

	UFUNCTION(BlueprintCallable, Category = "CameraChange")
	bool IsCameraChanging() const { return DefaultTimeline.IsPlaying() || ZoomInTimeline.IsPlaying(); }

protected:
	// Delegate Functions
	UFUNCTION()	void OnDefaultUpdate(float _Value);
	UFUNCTION()	void OnDefaultFinished();
	UFUNCTION()	void OnZoomInUpdate(float _Value);
	UFUNCTION()	void OnZoomInFinished();

private:
	void LerpCameraOffset(const FVector& _TargetPos, const FRotator& _TargetRot, float _Value);
};