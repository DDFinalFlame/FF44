#include "Weapon/BaseWeapon.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
// Debug
#include "Kismet/KismetSystemLibrary.h"

// Includes
#include "MonsterCharacter.h"
#include "Player/BasePlayer.h"
#include "MonsterAttributeSet.h"
#include "Player/BasePlayerAttributeSet.h"
#include "MonsterTags.h"
#include "CombatEventData.h"

ABaseWeapon::ABaseWeapon()
{
	//PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	WeaponCollision = CreateDefaultSubobject<USphereComponent>(TEXT("WeaponCollision"));
	WeaponCollision->SetupAttachment(WeaponMesh);

	WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &ABaseWeapon::OnSphereBeginOverlap);
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//void ABaseWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
//									   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
//									   bool bFromSweep, const FHitResult& SweepResult)
//{
//	if (OtherActor && OtherActor != this)
//	{
//		auto Monster = Cast<AMonsterCharacter>(OtherActor);
//		if (Monster)
//		{
//			Monster->TriggerHitReact(GetOwner());
//		}
//	}
//
//
//}

void ABaseWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    AActor* InstigatorActor = GetOwner();     // ���� ������(�÷��̾�/����)
    if (!InstigatorActor) return;

    // ���� ��/�ڱ� �ڽ� �� ���͸� �ʿ��ϸ� ���⼭ early-return
    // if (IsSameTeam(InstigatorActor, OtherActor)) return;

    // 1) �������� ASC ���
    UAbilitySystemComponent* SourceASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InstigatorActor);
    if (!SourceASC) return;

    // 2) ���� �б�
    float Attack = 0.f;
    float Stemina = 0.f;
    /*if (const UMonsterAttributeSet* M = SourceASC->GetSet<UMonsterAttributeSet>())
        Attack = M->GetAttackPower();
    else */if (const UBasePlayerAttributeSet* P = SourceASC->GetSet<UBasePlayerAttributeSet>())
        Attack = P->GetAttackPower();


   /* if (const UMonsterAttributeSet* M = SourceASC->GetSet<UMonsterAttributeSet>())
        Stemina = M->GetMoveSpeed();
    else */if (const UBasePlayerAttributeSet* P = SourceASC->GetSet<UBasePlayerAttributeSet>())
        Stemina = P->GetStamina();


    // �ʿ�� ����/��ų ��� ����
    const float SkillCoeff = 1.0f;
    const float FinalAttack = Attack * SkillCoeff;

    UCombatEventData* CombatData = NewObject<UCombatEventData>();
    CombatData->AttackPower = FinalAttack;
    CombatData->Defense = Stemina;

    FGameplayEventData Payload;
    Payload.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Attack"));
    Payload.Instigator = InstigatorActor;
    Payload.Target = OtherActor;
    //Payload.EventMagnitude = FinalAttack;    
    Payload.OptionalObject = CombatData;
    
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, Payload.EventTag, Payload);

}