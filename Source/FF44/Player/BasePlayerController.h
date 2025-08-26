// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h" 
#include "BasePlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class FF44_API ABasePlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputMapping")
	TArray<UInputMappingContext*> InputMappingContexts;

	virtual void SetupInputComponent() override;

private:
	FGenericTeamId PlayerTeamId = FGenericTeamId(0);

public:
	// �������̽� ����(override)
	virtual FGenericTeamId GetGenericTeamId() const override { return PlayerTeamId; }

	// �ʿ��ϸ� �ٲٴ� �Լ��� ����
	FORCEINLINE void SetPlayerTeamId(uint8 Id) { PlayerTeamId = FGenericTeamId(Id); }
};
