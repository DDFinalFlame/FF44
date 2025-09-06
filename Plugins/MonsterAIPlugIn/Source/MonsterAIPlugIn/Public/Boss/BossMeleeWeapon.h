#pragma once

#include "CoreMinimal.h"
#include "Weapon/MonsterBaseWeapon.h"
#include "GameplayTagContainer.h"  
#include "MonsterTags.h"
#include "BossMeleeWeapon.generated.h"

class UBoxComponent;
class UGameplayEffect;                  
class AAIController;                    
class UBlackboardComponent;             
UCLASS()
class MONSTERAIPLUGIN_API ABossMeleeWeapon : public AMonsterBaseWeapon
{
	GENERATED_BODY()
public:
	ABossMeleeWeapon();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FVector BoxExtent = FVector(15.f, 15.f, 15.f);

protected:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Hitbox = nullptr;
	
// ===== Grab ���� ���� =====
// ������ Ÿ�ֿ̹��� Ʈ���� (�ִ� ��Ƽ���̷� on/off)
// ��: ������ ��ȿ ������ �������� true
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grab|Window")
	bool bGrabOpenerWindow = false;          // [ADD]

	// ���� â���� ���� �� �ߵ� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grab|Window")
	bool bTriggeredThisWindow = false;       // [ADD]

	// BB Ű�� (AI�� ���� Ÿ��/Ȯ�� ��ȣ)
	UPROPERTY(EditAnywhere, Category = "Grab|Blackboard")
	FName BB_TargetActorKey = TEXT("TargetActor");     // [ADD]

	UPROPERTY(EditAnywhere, Category = "Grab|Blackboard")
	FName BB_GrabConfirmedKey = TEXT("bGrabConfirmed");// [ADD]

	// ���� GA�� ����� �̺�Ʈ �±� (���� GA_Boss_Grab�� �̰� Listen)
	UPROPERTY(EditAnywhere, Category = "Grab|Event")
	FGameplayTag BossGrabTriggerTag = MonsterTags::Event_Boss_Grab_Trigger;

	// ������(�÷��̾�)���Ե� ��Ư������ ��Ʈ�� ��ȣ�� ������
	UPROPERTY(EditAnywhere, Category = "Grab|Event")
	bool bNotifyVictimOnSpecialHit = true;

	// �����ڿ��� ���� �̺�Ʈ �±� (�÷��̾� GA�� �� �±׷� Listen)
	UPROPERTY(EditAnywhere, Category = "Grab|Event")
	FGameplayTag VictimSpecialHitTag = MonsterTags::Event_Player_Grab_Trigger;


protected:

	// �Ļ����� ���׷� Ʈ���š��� �ȾƳֱ� ���� ApplyHit Ȯ��
	virtual void ApplyHit(AActor* Victim, const FHitResult& Hit) override;  // [ADD]

	// Ư�� ������ ���� ����
	UPROPERTY(EditAnywhere, Category = "Grab|Filter")
	FGameplayTagContainer BlockVictimTags; 

public:
	// �ִ� ��Ƽ����(�Ǵ� ANS)���� â on/off
	UFUNCTION(BlueprintCallable, Category = "Grab|Window")
	void SetGrabOpenerWindow(bool bOpen);             // [ADD]

	UFUNCTION(BlueprintCallable, Category = "Grab|Window")
	void ResetGrabWindowTrigger();                    // [ADD]
};
