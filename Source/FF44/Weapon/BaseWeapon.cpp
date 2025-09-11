#include "Weapon/BaseWeapon.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

// Debug
#include "Kismet/KismetSystemLibrary.h"

// Includes
#include "Monster/MonsterCharacter.h"
#include "Player/BasePlayer.h"
#include "Player/BasePlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "DrawDebugHelpers.h" 
#include "Boss/WeakPointActor.h" 

ABaseWeapon::ABaseWeapon()
{
	//PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    SetRootComponent(WeaponMesh);

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

void ABaseWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // 1) ���� ����
    if (!HasAuthority()) return;

    // 2) �⺻ ����
    if (!OtherActor || OtherActor == GetOwner()) return;
    //if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor)) return;

    // 3) HitResult �غ�
    FHitResult HR = SweepResult;

    // Sweep ������ ���ų� ��������� ����
    if (!bFromSweep || !HR.Component.IsValid())
    {
        HR = FHitResult();

        // (a) Actor/Component ����
        HR.HitObjectHandle = OtherActor;     // HR.Actor �ƴ�
        HR.Component = OtherComp;

        // (b) �浹 ����/���� ����
        const FVector Tip =
            WeaponCollision ? WeaponCollision->GetComponentLocation()
            : (OverlappedComp ? OverlappedComp->GetComponentLocation()
                : GetActorLocation());

        FVector ClosestPoint = OtherActor->GetActorLocation();
        float   Dist = -1.f;

        if (OtherComp)
        {
            // UE5.6: 3��° ���ڴ� FName, ��ȯ���� �Ÿ�(float). -1�̸� ����.
            Dist = OtherComp->GetClosestPointOnCollision(Tip, ClosestPoint /*, NAME_None*/);
        }

        if (Dist >= 0.f)
        {
            HR.ImpactPoint = ClosestPoint;
            HR.ImpactNormal = (Tip - ClosestPoint).GetSafeNormal();
        }
        else
        {
            HR.ImpactPoint = Tip;
            HR.ImpactNormal = GetActorForwardVector();
        }

        // (c) ���̷�Ż�̸� �� �̸� ����
        if (USkeletalMeshComponent* Skel = Cast<USkeletalMeshComponent>(OtherComp))
        {
            HR.BoneName = Skel->FindClosestBone(HR.ImpactPoint);
        }
    }
    // �����ڽ�.
    //if (UWorld* World = GetWorld())
    //{
    //    // ImpactPoint ��ġ�� �ݰ� 10¥�� �ʷϻ� ���Ǿ 3�� ���� ǥ��
    //    DrawDebugSphere(World, HR.ImpactPoint, 10.0f, 12, FColor::Green, false, 3.0f);

    //    // ImpactNormal ���� Ȯ�ο� ����
    //    DrawDebugLine(World, HR.ImpactPoint, HR.ImpactPoint + HR.ImpactNormal * 50.0f, FColor::Red, false, 1.0f, 0, 1.5f);
    //}

    // ���� �±� �޾Ƣa
    if (OtherActor->ActorHasTag(FName(TEXT("Boss"))))
    {
        if (OtherActor->GetClass()->ImplementsInterface(UAbilitySystemInterface::StaticClass()))
        {
            if (auto abilityActor = Cast<IAbilitySystemInterface>(OtherActor))
                if (auto player = Cast<ABasePlayer>(GetOwner()))
                    player->GetBasePlayerController()
                    ->InitBossUI(abilityActor->GetAbilitySystemComponent());
        }            
    }

    if (AWeakPointActor* WP = Cast<AWeakPointActor>(OtherActor))
    {
        WP->NotifyHitByPlayerWeapon(SweepResult, GetOwner());   // �Ǵ� WP->NotifyHitByPlayerWeapon(HR, GetOwner());
        return;
    }
    // 4-2) ������Ʈ �±׷� �ĺ�(���� �޽� �� �Ϻθ� ������ ��)
    if (OtherComp && OtherComp->ComponentHasTag(FName(TEXT("BossWeakPoint"))))
    {
        if (AWeakPointActor* WP2 = Cast<AWeakPointActor>(OtherComp->GetOwner()))
        {
            WP2->NotifyHitByPlayerWeapon(SweepResult, GetOwner());
            return;
        }
        // ������ ���� Actor�� �ƴϸ� ���⼭ Ŀ���� ���� ���� ����
    }



    // 4) �̺�Ʈ ���̷ε� ���� + HitResult�� TargetData�� ���
    FGameplayEventData Payload;
    Payload.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Monster.Hit"));
    Payload.Instigator = GetOwner();
    Payload.Target = OtherActor;
    Payload.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(HR);

    // 5) ����(�ǰ��ڿ���)
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, Payload.EventTag, Payload);
}
