
#include "Boss/GA_BossPhase1.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

#include "MonsterTags.h"
#include "MonsterAttributeSet.h"
#include "Monster/MonsterCharacter.h"
#include "Boss/BossCharacter.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/staticName.h"


static FVector RandomPointInAnnulus2D(const FVector& Center, float Rmin, float Rmax)
{
    const float R = FMath::Sqrt(FMath::FRandRange(Rmin * Rmin, Rmax * Rmax));
    const float Theta = FMath::FRandRange(0.f, 2 * PI);
    return Center + FVector(R * FMath::Cos(Theta), R * FMath::Sin(Theta), 0.f);
}

UGA_BossPhase1::UGA_BossPhase1()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(MonsterTags::Ability_Boss_PhaseStart);
    SetAssetTags(AssetTags);

    ActivationBlockedTags.AddTag(MonsterTags::State_Dying);
    ActivationBlockedTags.AddTag(MonsterTags::State_Dead);
    BlockAbilitiesWithTag.AddTag(MonsterTags::Ability_HitReact);
}

void UGA_BossPhase1::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, Info, ActivationInfo, TriggerEventData);

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!ASC || !Boss)
    {
        EndAbility(Handle, Info, ActivationInfo, true, false);
        return;
    }

    const float CurHP = ASC->GetNumericAttribute(UMonsterAttributeSet::GetHealthAttribute());
    const float MaxHP = ASC->GetNumericAttribute(UMonsterAttributeSet::GetMaxHealthAttribute());
    const float Ratio = (MaxHP > 0.f) ? (CurHP / MaxHP) : 0.f;

    if (Ratio <= StartHpRatioThreshold && !bPhaseStarted)
    {
        StartPhase();
        return;
    }

    // ���� �Ӱ� �� �� HP ��ȭ ���� ���ε�
    BindHPThresholdWatch();
}

void UGA_BossPhase1::BindHPThresholdWatch()
{
    if (HPChangeHandle.IsValid()) return;

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        HPChangeHandle = ASC->GetGameplayAttributeValueChangeDelegate(
            UMonsterAttributeSet::GetHealthAttribute()
        ).AddUObject(this, &UGA_BossPhase1::OnHPChangedNative);
    }
}

void UGA_BossPhase1::UnbindHPThresholdWatch()
{
    if (!HPChangeHandle.IsValid()) return;

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        ASC->GetGameplayAttributeValueChangeDelegate(
            UMonsterAttributeSet::GetHealthAttribute()
        ).Remove(HPChangeHandle);
    }
    HPChangeHandle.Reset();
}

void UGA_BossPhase1::OnHPChangedNative(const FOnAttributeChangeData& Data)
{
    if (bPhaseStarted) { UnbindHPThresholdWatch(); return; }

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        const float Max = ASC->GetNumericAttribute(UMonsterAttributeSet::GetMaxHealthAttribute());
        if (Max <= 0.f) return;

        const float Ratio = Data.NewValue / Max;
        if (Ratio <= StartHpRatioThreshold)
        {
            UnbindHPThresholdWatch();
            StartPhase();
        }
    }
}

void UGA_BossPhase1::StartPhase()
{
    if (bPhaseStarted) return;
    bPhaseStarted = true;
    ////�ǰݸ�� X
    //if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    //{
    //    FGameplayTagContainer CancelTags;
    //    CancelTags.AddTag(MonsterTags::Ability_HitReact);
    //    ASC->CancelAbilities(&CancelTags);
    //}


    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!ASC || !Boss)
    {
        K2_EndAbility();
        return;
    }

    Boss->GetWorldTimerManager().SetTimerForNextTick(this, &UGA_BossPhase1::BeginStartSequence);
    



    // A) ���� GE ���� (���� GCN �ڵ� ����)
    if (GE_BossInvuln)
    {
        FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
        Ctx.AddInstigator(Boss, Boss->GetInstigatorController());
        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GE_BossInvuln, 1.f, Ctx);
        InvulnHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
    }

    // B) ���� ����: ��Ÿ�� + ����
    //if (StartMontage)
    //{
    //    if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
    //        this, NAME_None, StartMontage, 1.f, NAME_None, false))
    //    {
    //        Task->ReadyForActivation();
    //    }
    //}
    if (StartSound)
    {
        if (ACharacter* Char = Cast<ACharacter>(Boss))
        {
            UGameplayStatics::SpawnSoundAttached(StartSound, Char->GetMesh());
        }
        else
        {
            UGameplayStatics::PlaySoundAtLocation(Boss, StartSound, Boss->GetActorLocation());
        }
    }

    // C) ��ȯ
    AliveMinionCount = SpawnMinions(Boss);

    // D) ��ȯ�� ��� �̺�Ʈ ��� (�̴Ͼ� Death GA���� Event.Minion.Died �۽�)
    if (UAbilityTask_WaitGameplayEvent* WaitEvt =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, MonsterTags::Event_Minion_Died))
    {
        WaitEvt->EventReceived.AddDynamic(this, &UGA_BossPhase1::OnMinionDiedEvent);
        WaitEvt->ReadyForActivation();
    }

    if (ABossCharacter* BossCharacter = Cast<ABossCharacter>(GetAvatarActorFromActorInfo()))
    {
        BossCharacter->SetBossState_EBB((uint8)EBossState_BB::InPhase1);
    }


    // E) ĳ���� ���� ����
    StartCastTick();
}

AActor* UGA_BossPhase1::GetPhaseTargetPlayer() const
{
    // �ʿ信 ���� �÷��̾� �ε���/Ÿ�� ��Ģ ����
    return UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

// ----- �̴Ͼ� BB ���� �õ� (���� �� true) -----
bool UGA_BossPhase1::TrySetupMinionBlackboard(AMonsterCharacter* MC, AActor* Player)
{
    if (!MC) return false;

    // ��Ʈ�ѷ� ����
    AAIController* AICon = Cast<AAIController>(MC->GetController());
    if (!AICon)
    {
        MC->SpawnDefaultController();
        AICon = Cast<AAIController>(MC->GetController());
        if (!AICon) return false; // ���� ��Ʈ�ѷ��� ���� �� ��õ�
    }

    UBlackboardComponent* BB = AICon->GetBlackboardComponent();
    if (!BB) return false; // BB ���� �ʱ�ȭ �� �� ��õ�

    // �� ����
    BB->SetValueAsObject(KEY_TargetActor, Player);
    BB->SetValueAsEnum(KEY_MonsterState, DesiredMinionState);


    return true;
}

// ----- ��⿭�� �߰� -----
void UGA_BossPhase1::EnqueueMinionInit(AMonsterCharacter* MC)
{
    if (!MC) return;
    PendingInitMinions.AddUnique(MC);

    // Ÿ�̸Ӱ� ���� ���� ������ ����
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (Boss && !Boss->GetWorldTimerManager().IsTimerActive(MinionInitTimerHandle))
    {
        Boss->GetWorldTimerManager().SetTimer(
            MinionInitTimerHandle, this, &UGA_BossPhase1::ProcessPendingMinionInits, 0.1f, true, 0.1f);
    }
}

// ----- ��⿭ ó�� -----
void UGA_BossPhase1::ProcessPendingMinionInits()
{
    AActor* Player = GetPhaseTargetPlayer();
    if (!Player)
    {
        // �÷��̾� ������ ���� ƽ�� �ٽ�
        return;
    }

    // �ڿ������� ����
    for (int32 i = PendingInitMinions.Num() - 1; i >= 0; --i)
    {
        TWeakObjectPtr<AMonsterCharacter> WeakMC = PendingInitMinions[i];
        AMonsterCharacter* MC = WeakMC.Get();
        if (!MC)
        {
            PendingInitMinions.RemoveAtSwap(i);
            continue;
        }

        if (TrySetupMinionBlackboard(MC, Player))
        {
            PendingInitMinions.RemoveAtSwap(i);
        }
    }

    // ��� ó�� ������ Ÿ�̸� ����
    if (PendingInitMinions.Num() == 0)
    {
        if (AActor* Boss = GetAvatarActorFromActorInfo())
        {
            Boss->GetWorldTimerManager().ClearTimer(MinionInitTimerHandle);
        }
    }
}


int32 UGA_BossPhase1::SpawnMinions(AActor* Boss)
{
    if (!Boss || !MinionClass) return 0;

    UWorld* World = Boss->GetWorld();
    if (!World) return 0;

    const int32 Count = FMath::Max(1, FMath::RandRange(SpawnCountMin, SpawnCountMax));
    const FVector Origin = Boss->GetActorLocation();
    const float StepDeg = 360.f / Count;

    UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(World);
    AActor* Player = GetPhaseTargetPlayer();

    for (int32 i = 0; i < Count; ++i)
    {
        const float Deg = StepDeg * i;
        const FVector Dir = UKismetMathLibrary::GetForwardVector(FRotator(0.f, Deg, 0.f));
        const FVector Desired = Origin + Dir * SpawnRadius;

        FVector SpawnLoc = Desired;
        if (Nav)
        {
            FNavLocation Out;
            if (Nav->ProjectPointToNavigation(Desired, Out, FVector(200.f)))
            {
                SpawnLoc = Out.Location;
            }
        }

        const FTransform T(FRotator::ZeroRotator, SpawnLoc);
        AActor* Spawned = World->SpawnActor<AActor>(MinionClass, T);
        if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(Spawned))
        {
            MC->SetOwnerBoss(GetAvatarActorFromActorInfo()); 

            if (!Player || !TrySetupMinionBlackboard(MC, Player))
            {
                EnqueueMinionInit(MC);
            }
        }
    }
    return Count;
}

void UGA_BossPhase1::StartCastTick()
{
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!Boss || CastInterval <= 0.f) return;

    Boss->GetWorldTimerManager().SetTimer(
        CastTimerHandle,
        [this]() { DoCastOnce(); },
        CastInterval,
        true,
        CastInterval // �ʱ� ����
    );
}

void UGA_BossPhase1::DoCastOnce()
{
    if (CastLoopMontage)
    {
        if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, CastLoopMontage, 1.f, NAME_None, false))
        {
            Task->ReadyForActivation();
        }

        if (!FallingRockClass) return;

        UWorld* World = GetWorld();
        AActor* Player = GetPhaseTargetPlayer();
        if (!World || !Player) return;

        int32 numRocks = FMath::Clamp(FMath::RandRange(RocksPerCastMin, RocksPerCastMax), 1, 50);
        FVector playerLoc = Player->GetActorLocation();

        for (int32 i = 0; i < numRocks; ++i)
        {
            // �÷��̾� ���� ���� ������(����)
            FVector groundXY = RandomPointInAnnulus2D(playerLoc, PlayerAreaRadiusMin, PlayerAreaRadiusMax);
            // ������ �������� ���� ��ġ ����
            FVector spawnLoc = FVector(groundXY.X, groundXY.Y, playerLoc.Z + SpawnHeight);

            FTransform T(FRotator::ZeroRotator, spawnLoc);
            AActor* rock = World->SpawnActorDeferred<AActor>(
                FallingRockClass, T, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

            if (rock)
            {
                rock->FinishSpawning(T);
                if (RockLifeSeconds > 0.f) rock->SetLifeSpan(RockLifeSeconds);
            }
        }
    }

    // TODO: ���⼭ ����ü/�������� �� ���� ĳ���� ���� ȣ��
}

void UGA_BossPhase1::OnMinionDiedEvent(FGameplayEventData Payload)
{
    if (AliveMinionCount > 0) --AliveMinionCount;

    if (AliveMinionCount == 0)
    {
        //������������ �� endAbility
        //if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
        //{
        //    if (InvulnHandle.IsValid())
        //    {
        //        ASC->RemoveActiveGameplayEffect(InvulnHandle); // ���� ����(���� GCN �ڵ� Off)
        //        InvulnHandle.Invalidate();
        //    }
        //}
        //K2_EndAbility();
        BeginEndSequence();
    }
}

void UGA_BossPhase1::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().ClearTimer(CastTimerHandle);
    }

    UnbindHPThresholdWatch();

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (InvulnHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(InvulnHandle);
            InvulnHandle.Invalidate();
        }
        ASC->RemoveGameplayCue(MonsterTags::GC_Boss_InvulnShield);
    }
    
    if (ABossCharacter* Boss = Cast<ABossCharacter>(GetAvatarActorFromActorInfo()))
    {
        // ĳ����/��ȯ ������ ���� �� ���� �������
        Boss->SetBossState_EBB((uint8)EBossState_BB::Phase1_Attack);
    }

    Super::EndAbility(Handle, Info, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


void UGA_BossPhase1::BeginStartSequence()
{
    // 1) Hit GA ���
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ASC)
    {
        FGameplayTagContainer CancelTags;
        CancelTags.AddTag(MonsterTags::Ability_HitReact);
        ASC->CancelAbilities(&CancelTags);
    }

    // 2) Ȥ�� ���� ��Ÿ�� ����(�׷� �Ǵ� ��ü)
    ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (C && C->GetMesh())
    {
        UAnimInstance* Anim = C->GetMesh()->GetAnimInstance();
        if (Anim)
        {
            // ���� Ȯ��: ��ü ����(�ʿ� �� �׷츸 ������ �ٲټ���)
            Anim->StopAllMontages(0.10f);
            // �Ǵ�: Anim->Montage_StopGroupByName(0.10f, FName("DefaultGroup"));
        }
    }

    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().SetTimerForNextTick(this, &UGA_BossPhase1::PlayStartMontageThenStartCast);
    }
}

void UGA_BossPhase1::PlayStartMontageThenStartCast()
{
    if (!StartMontage)
    {
        StartCastTick();
        return;
    }

    if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, StartMontage, 1.f, NAME_None, /*bStopWhenAbilityEnds=*/false))
    {
        Task->OnCompleted.AddDynamic(this, &UGA_BossPhase1::StartCastTick);
        Task->OnBlendOut.AddDynamic(this, &UGA_BossPhase1::StartCastTick);
        Task->OnInterrupted.AddDynamic(this, &UGA_BossPhase1::StartCastTick);
        Task->OnCancelled.AddDynamic(this, &UGA_BossPhase1::StartCastTick);
        Task->ReadyForActivation();
    }
    else
    {
        // ������
        StartCastTick();
    }
}


void UGA_BossPhase1::BeginEndSequence()
{
    if (bEnding) return;
    bEnding = true;

    AActor* Boss = GetAvatarActorFromActorInfo();

    // 1) ĳ���� ���� �ߴ�
    if (Boss)
    {
        Boss->GetWorldTimerManager().ClearTimer(CastTimerHandle);
    }

    // 2) �ǰ�/��Ÿ GA�� ���� ���ϰ� HitReact �� ���(����)
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        FGameplayTagContainer CancelTags;
        CancelTags.AddTag(MonsterTags::Ability_HitReact);
        ASC->CancelAbilities(&CancelTags);
    }

    // 3) ���� ���� ��Ÿ�� ����(ĳ��Ʈ ���� ��Ÿ�� ��)
    if (ACharacter* C = Cast<ACharacter>(Boss))
    {
        if (UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.10f); // �ʿ��ϸ� �׷츸 ������ �ٲ㵵 OK
        }
    }

    // 4) End ��Ÿ�� ���(������ �ٷ� ����)
    PlayEndMontageAndFinish();
}

void UGA_BossPhase1::PlayEndMontageAndFinish()
{
    if (EndMontage)
    {
        if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, EndMontage, 1.f, NAME_None, /*bStopWhenAbilityEnds=*/false))
        {
            // � ���� ������ Ability ����
            Task->OnCompleted.AddDynamic(this, &UGA_BossPhase1::K2_EndAbility);
            Task->OnBlendOut.AddDynamic(this, &UGA_BossPhase1::K2_EndAbility);
            Task->OnInterrupted.AddDynamic(this, &UGA_BossPhase1::K2_EndAbility);
            Task->OnCancelled.AddDynamic(this, &UGA_BossPhase1::K2_EndAbility);
            Task->ReadyForActivation();
            return;
        }
    }

    // EndMontage�� ���ų� Task ���� ���� �� ������
    K2_EndAbility();
}