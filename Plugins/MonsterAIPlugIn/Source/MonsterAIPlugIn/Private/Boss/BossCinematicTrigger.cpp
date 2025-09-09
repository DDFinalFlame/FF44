#include "Boss/BossCinematicTrigger.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

ABossCinematicTrigger::ABossCinematicTrigger()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
    SetRootComponent(Box);
    Box->InitBoxExtent(FVector(200.f, 200.f, 100.f));
    Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Box->SetCollisionObjectType(ECC_WorldDynamic);
    Box->SetCollisionResponseToAllChannels(ECR_Ignore);
    Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Box->SetGenerateOverlapEvents(true);

    Marker = CreateDefaultSubobject<UBillboardComponent>(TEXT("Marker"));
    Marker->SetupAttachment(Box);

    bTriggered = false;

    Box->OnComponentBeginOverlap.AddDynamic(this, &ABossCinematicTrigger::OnBeginOverlap);
}

void ABossCinematicTrigger::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;

    if (!TargetBoss && bAutoResolveBoss)
    {
        TryResolveBoss();

        // 아직 못 찾았으면 앞으로 스폰되는 액터를 감시
        if (!TargetBoss)
        {
            if (UWorld* W = GetWorld())
            {
                ActorSpawnedHandle = W->AddOnActorSpawnedHandler(
                    FOnActorSpawned::FDelegate::CreateUObject(this, &ABossCinematicTrigger::OnActorSpawned));
            }
        }
    }
}

void ABossCinematicTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ActorSpawnedHandle.IsValid())
    {
        if (UWorld* W = GetWorld())
        {
            W->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
        }
        ActorSpawnedHandle.Reset();
    }
    Super::EndPlay(EndPlayReason);
}

void ABossCinematicTrigger::TryResolveBoss()
{
    UWorld* W = GetWorld();
    if (!W) return;

    TArray<AActor*> Candidates;

    // 1순위: 클래스 힌트로 검색, 2순위: 태그로 검색
    if (BossClassHint)
    {
        UGameplayStatics::GetAllActorsOfClass(W, BossClassHint, Candidates);
    }
    else if (!BossActorTag.IsNone())
    {
        UGameplayStatics::GetAllActorsWithTag(W, BossActorTag, Candidates);
    }

    if (Candidates.Num() == 0) return;

    const FVector MyLoc = GetActorLocation();
    const float R2 = (ResolveSearchRadius > 0.f) ? ResolveSearchRadius * ResolveSearchRadius : TNumericLimits<float>::Max();

    float BestD2 = TNumericLimits<float>::Max();
    AActor* Best = nullptr;

    for (AActor* A : Candidates)
    {
        if (!A || A == this) continue;
        const float D2 = FVector::DistSquared(MyLoc, A->GetActorLocation());
        if (D2 > R2) continue;
        if (D2 < BestD2)
        {
            BestD2 = D2;
            Best = A;
        }
    }

    if (Best)
    {
        TargetBoss = Best;
    }
}

void ABossCinematicTrigger::OnActorSpawned(AActor* NewActor)
{
    if (TargetBoss) // 이미 찾았으면 핸들 해제
    {
        if (ActorSpawnedHandle.IsValid())
        {
            if (UWorld* W = GetWorld())
                W->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
            ActorSpawnedHandle.Reset();
        }
        return;
    }

    if (!NewActor) return;

    // 클래스/태그 조건 체크
    if (BossClassHint)
    {
        if (!NewActor->IsA(BossClassHint)) return;
    }
    else if (!BossActorTag.IsNone())
    {
        if (!NewActor->ActorHasTag(BossActorTag)) return;
    }

    if (ResolveSearchRadius > 0.f)
    {
        const float D2 = FVector::DistSquared(GetActorLocation(), NewActor->GetActorLocation());
        if (D2 > ResolveSearchRadius * ResolveSearchRadius) return;
    }

    TargetBoss = NewActor;

    // 더 이상 감시 불필요
    if (ActorSpawnedHandle.IsValid())
    {
        if (UWorld* W = GetWorld())
            W->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
        ActorSpawnedHandle.Reset();
    }
}

void ABossCinematicTrigger::OnBeginOverlap(UPrimitiveComponent* _OverlappedComp, AActor* _OtherActor,
    UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex,
    bool _bFromSweep, const FHitResult& _SweepResult)
{
    if (bOneShot && bTriggered) return;
    if (!HasAuthority()) return; // 서버에서만 발동

    if (!_OtherActor) return;

    if (bOnlyPlayer)
    {
        ACharacter* AsChar = Cast<ACharacter>(_OtherActor);
        if (!AsChar || !AsChar->IsPlayerControlled())
        {
            return;
        }
    }

    TriggerOnce(_OtherActor);
}

void ABossCinematicTrigger::TriggerOnce(AActor* _OtherActor)
{
    bTriggered = true;

    if (!TargetBoss && bAutoResolveBoss)
    {
        TryResolveBoss(); // 마지막으로 한 번 더 시도
    }

    // 1) 보스 상태 전환(블랙보드)
    SetBossStateBB();

    // 2) 보스 인트로 실행: Event → Ability → Montage 순서로 선택적으로
    if (BossIntroEventTag.IsValid())
    {
        SendBossIntroEvent();
    }
    if (IntroAbilityClass)
    {
        ActivateBossIntroAbility();
    }
    if (BossIntroMontage)
    {
        PlayBossIntroMontage();
    }

    // TODO: (선택) 레벨시퀀스 재생 훅 연결 위치

    if (bDestroyAfterTrigger)
    {
        Destroy();
    }
    else if (bOneShot)
    {
        Box->SetGenerateOverlapEvents(false);
        SetActorEnableCollision(false);
        SetActorHiddenInGame(true);
    }
}

void ABossCinematicTrigger::SetBossStateBB()
{
    if (!TargetBoss || BossStateBBKey.IsNone()) return;

    AAIController* AI = Cast<AAIController>(Cast<APawn>(TargetBoss) ? Cast<APawn>(TargetBoss)->GetController() : nullptr);
    if (!AI) return;

    UBlackboardComponent* BB = AI->GetBlackboardComponent();
    if (!BB) return;

    // uint8 값을 그대로 씀 (에디터에서 enum 인덱스와 맞춰주세요)
    BB->SetValueAsEnum(BossStateBBKey, BossStateValue);
    BB->SetValueAsBool(InBattleBBKey, true);
}

void ABossCinematicTrigger::SendBossIntroEvent()
{
    if (!TargetBoss || !BossIntroEventTag.IsValid()) return;

    FGameplayEventData Data;
    Data.EventTag = BossIntroEventTag;
    Data.Instigator = this;
    Data.Target = TargetBoss;

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetBoss, BossIntroEventTag, Data);
}

void ABossCinematicTrigger::ActivateBossIntroAbility()
{
    if (!TargetBoss || !IntroAbilityClass) return;

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetBoss);
    if (ASC)
    {
        ASC->TryActivateAbilityByClass(IntroAbilityClass);
    }
}

void ABossCinematicTrigger::PlayBossIntroMontage()
{
    if (!TargetBoss || !BossIntroMontage) return;

    ACharacter* BossChar = Cast<ACharacter>(TargetBoss);
    if (!BossChar || !BossChar->GetMesh() || !BossChar->GetMesh()->GetAnimInstance()) return;

    // 서버에서 재생하면 네트워크 복제되는 캐릭터는 클라에도 반영됩니다(보스가 Networked Character인 경우).
    BossChar->PlayAnimMontage(BossIntroMontage, 1.f);
}