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

        // ���� �� ã������ ������ �����Ǵ� ���͸� ����
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

    // 1����: Ŭ���� ��Ʈ�� �˻�, 2����: �±׷� �˻�
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
    if (TargetBoss) // �̹� ã������ �ڵ� ����
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

    // Ŭ����/�±� ���� üũ
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

    // �� �̻� ���� ���ʿ�
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
    if (!HasAuthority()) return; // ���������� �ߵ�

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
        TryResolveBoss(); // ���������� �� �� �� �õ�
    }

    // 1) ���� ���� ��ȯ(������)
    SetBossStateBB();

    // 2) ���� ��Ʈ�� ����: Event �� Ability �� Montage ������ ����������
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

    // TODO: (����) ���������� ��� �� ���� ��ġ

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

    // uint8 ���� �״�� �� (�����Ϳ��� enum �ε����� �����ּ���)
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

    // �������� ����ϸ� ��Ʈ��ũ �����Ǵ� ĳ���ʹ� Ŭ�󿡵� �ݿ��˴ϴ�(������ Networked Character�� ���).
    BossChar->PlayAnimMontage(BossIntroMontage, 1.f);
}