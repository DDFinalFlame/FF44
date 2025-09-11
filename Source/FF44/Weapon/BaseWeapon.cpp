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
    // 1) 서버 전용
    if (!HasAuthority()) return;

    // 2) 기본 필터
    if (!OtherActor || OtherActor == GetOwner()) return;
    //if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor)) return;

    // 3) HitResult 준비
    FHitResult HR = SweepResult;

    // Sweep 정보가 없거나 비어있으면 보정
    if (!bFromSweep || !HR.Component.IsValid())
    {
        HR = FHitResult();

        // (a) Actor/Component 세팅
        HR.HitObjectHandle = OtherActor;     // HR.Actor 아님
        HR.Component = OtherComp;

        // (b) 충돌 지점/법선 추정
        const FVector Tip =
            WeaponCollision ? WeaponCollision->GetComponentLocation()
            : (OverlappedComp ? OverlappedComp->GetComponentLocation()
                : GetActorLocation());

        FVector ClosestPoint = OtherActor->GetActorLocation();
        float   Dist = -1.f;

        if (OtherComp)
        {
            // UE5.6: 3번째 인자는 FName, 반환값이 거리(float). -1이면 내부.
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

        // (c) 스켈레탈이면 본 이름 보정
        if (USkeletalMeshComponent* Skel = Cast<USkeletalMeshComponent>(OtherComp))
        {
            HR.BoneName = Skel->FindClosestBone(HR.ImpactPoint);
        }
    }
    // 디버깅박스.
    //if (UWorld* World = GetWorld())
    //{
    //    // ImpactPoint 위치에 반경 10짜리 초록색 스피어를 3초 동안 표시
    //    DrawDebugSphere(World, HR.ImpactPoint, 10.0f, 12, FColor::Green, false, 3.0f);

    //    // ImpactNormal 방향 확인용 라인
    //    DrawDebugLine(World, HR.ImpactPoint, HR.ImpactPoint + HR.ImpactNormal * 50.0f, FColor::Red, false, 1.0f, 0, 1.5f);
    //}

    // 보스 태그 달아줭
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
        WP->NotifyHitByPlayerWeapon(SweepResult, GetOwner());   // 또는 WP->NotifyHitByPlayerWeapon(HR, GetOwner());
        return;
    }
    // 4-2) 컴포넌트 태그로 식별(약점 메시 등 일부만 약점일 때)
    if (OtherComp && OtherComp->ComponentHasTag(FName(TEXT("BossWeakPoint"))))
    {
        if (AWeakPointActor* WP2 = Cast<AWeakPointActor>(OtherComp->GetOwner()))
        {
            WP2->NotifyHitByPlayerWeapon(SweepResult, GetOwner());
            return;
        }
        // 약점이 별도 Actor가 아니면 여기서 커스텀 로직 수행 가능
    }



    // 4) 이벤트 페이로드 구성 + HitResult를 TargetData에 담기
    FGameplayEventData Payload;
    Payload.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Monster.Hit"));
    Payload.Instigator = GetOwner();
    Payload.Target = OtherActor;
    Payload.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(HR);

    // 5) 전송(피격자에게)
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, Payload.EventTag, Payload);
}
