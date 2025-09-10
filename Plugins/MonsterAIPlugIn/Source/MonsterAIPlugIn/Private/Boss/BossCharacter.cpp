// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/BossCharacter.h"
#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterTags.h"
#include "Data/staticName.h"
#include "Boss/BossMeleeWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "MonsterAttributeSet.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"





ABossCharacter::ABossCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    Grab = CreateDefaultSubobject<UCameraComponent>(TEXT("GrabCamera"));
    Grab->SetupAttachment(GetMesh());
}

void ABossCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        SpawnAndAttachWeapons();
        ActivatePhaseWatcherOnce();
    }

    TryCacheBlackboardOnce();
}


void ABossCharacter::TryCacheBlackboardOnce()
{
    if (CachedBB) return;

    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        CachedBB = AICon->GetBlackboardComponent();
        if (!CachedBB)
        {
            // ���� ƽ�� �ٽ� �õ� (BB �ʱ�ȭ ���� ���)
            GetWorldTimerManager().SetTimerForNextTick(this, &ABossCharacter::TryCacheBlackboardOnce);
        }
    }
    else
    {
        GetWorldTimerManager().SetTimerForNextTick(this, &ABossCharacter::TryCacheBlackboardOnce);
    }
}


void ABossCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HasAuthority()) return;

    if (AbilitySystemComponent)
    {
        if (const UMonsterAttributeSet* Attr = Cast<UMonsterAttributeSet>(AttributeSet))
        {
            if (!bDeathSoundPlayed && Attr->GetHealth() <= 0.f)
            {
                bDeathSoundPlayed = true;

                StopBattleBGM(true, BattleBGMFadeOut);

                if (DeathSound)
                {
                    UGameplayStatics::PlaySoundAtLocation(
                        this,
                        DeathSound,
                        GetActorLocation()
                    );
                }

                if (!bDeathExtrasSpawned)
                {
                    bDeathExtrasSpawned = true;
                    SpawnDeathSideActors();
                }
            }
        }
    }

    if (!CachedBB) TryCacheBlackboardOnce();
    if (CachedBB)
    {
        const uint8 Raw = CachedBB->GetValueAsEnum(KEY_BossState);
        const EBossState_BB State = static_cast<EBossState_BB>(Raw);

        if (State == EBossState_BB::CombatReady)
        {
            StartBattleBGM();
        }

    }
}

void ABossCharacter::SpawnDeathSideActors()
{
    if (!DeathSpawnClass1 && (!DeathSpawnClass2)) return;
    UWorld* World = GetWorld();
    if (!World) return;

    const FVector Loc = GetActorLocation();
    const FRotator Rot = GetActorRotation();

    const FVector F = GetActorForwardVector(); // ��
    const FVector R = GetActorRightVector();   // ������

    const FVector Base = Loc - F * DeathSpawnBackOffset + FVector(0, 0, DeathSpawnZOffset);
    const FVector LeftPos = Base - R * DeathSpawnSideOffset; // ���� ��
    const FVector RightPos = Base + R * DeathSpawnSideOffset; // ������ ��

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    World->SpawnActor<AActor>(DeathSpawnClass1, LeftPos, Rot, Params);
    World->SpawnActor<AActor>(DeathSpawnClass2, RightPos, Rot, Params);
}

void ABossCharacter::ActivatePhaseWatcherOnce()
{
    if (!HasAuthority()) return;
    if (!AbilitySystemComponent)
    {
        // ASC�� ���� �ʱ�ȭ ���̸� ���� ƽ�� ��õ�
        GetWorldTimerManager().SetTimerForNextTick(this, &ABossCharacter::ActivatePhaseWatcherOnce);
        return;
    }

    // ASC�� ActorInfo�� �� ������ ���ε��ƴ��� Ȯ�� (InitAbilityActorInfo ���Ŀ��� ��)
    if (AbilitySystemComponent->GetAvatarActor() != this)
    {
        GetWorldTimerManager().SetTimerForNextTick(this, &ABossCharacter::ActivatePhaseWatcherOnce);
        return;
    }

    if (!bPhaseWatcherActivated)
    {
        if (Phase1AbilityClass)
        {
            AbilitySystemComponent->TryActivateAbilityByClass(Phase1AbilityClass);
        }

        if (Phase2AbilityClass)
        {
            AbilitySystemComponent->TryActivateAbilityByClass(Phase2AbilityClass);
        }
        bPhaseWatcherActivated = true;
    }
}


void ABossCharacter::SetBossState_EBB(uint8 NewState)
{
    if (!HasAuthority()) return;
    AAIController* AICon = Cast<AAIController>(GetController());
    if (!AICon) return;
    UBlackboardComponent* BB = AICon->GetBlackboardComponent();
    if (!BB) return;

    BB->SetValueAsEnum(KEY_BossState, NewState);

    //if (static_cast<EBossState_BB>(NewState) == EBossState_BB::CombatReady)
    //{
    //    StartBattleBGM();
    //}
}

void ABossCharacter::SetBossState_Name(FName BBKeyName, uint8 NewState)
{
    if (!HasAuthority()) return;
    AAIController* AICon = Cast<AAIController>(GetController());
    if (!AICon) return;
    if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
    {
        BB->SetValueAsEnum(BBKeyName, NewState);
    }
}


void ABossCharacter::SetBlackboardTargetActor(FName BBKeyName, AActor* NewTarget)
{
   // if (!HasAuthority()) return;

    AAIController* AICon = Cast<AAIController>(GetController());
    if (!AICon) return;

    if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
    {
        BB->SetValueAsObject(BBKeyName, NewTarget);
    }
}


void ABossCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    // �����
    UE_LOG(LogTemp, Warning, TEXT("Boss Landed!"));

    // Phase2 GA���� ��ٸ��� ���� �̺�Ʈ �۽�
    FGameplayEventData Data;
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        this, MonsterTags::Event_Boss_Land, Data);
}

void ABossCharacter::SpawnAndAttachWeapons()
{
    if (!HasAuthority()) return;
    if (Weapon || !WeaponClass) return;

    // �޽�/���� �غ� ���� �� ������ üũ �� ���� ��õ�
    if (!GetMesh())
    {
        GetWorldTimerManager().SetTimerForNextTick(this, &ABossCharacter::SpawnAndAttachWeapons);
        return;
    }

    // ���� ���缺 �˻� (�ϳ��� ������ ���� ƽ ��õ�)
    for (const FName& SocketName : WeaponAttachSocketNames)
    {
        if (!GetMesh()->DoesSocketExist(SocketName))
        {
            GetWorldTimerManager().SetTimerForNextTick(this, &ABossCharacter::SpawnAndAttachWeapons);
            return;
        }
    }

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (const FName& SocketName : WeaponAttachSocketNames)
    {
        AMonsterBaseWeapon* NewWeapon =
            GetWorld()->SpawnActor<AMonsterBaseWeapon>(WeaponClass, FTransform::Identity, Params);
        if (!NewWeapon) continue;

        // ���̽� ������ Init: ���Ͽ� ���� + ������ ���ε�
        NewWeapon->Init(this, GetMesh(), SocketName);

        // ���̽��� ��� �� Begin/EndAttackWindow�� �ڵ����� ����� ���⡯ ����
        RegisterWeapon(NewWeapon);
    }

}


void ABossCharacter::StartBattleBGM()
{
    if (!BattleBGM) return;

    if (BattleBGMComp && BattleBGMComp->IsPlaying())
        return;

    USceneComponent* AttachParent = GetMesh() ? GetMesh() : GetRootComponent();
    BattleBGMComp = UGameplayStatics::SpawnSoundAttached(
        BattleBGM,
        AttachParent,
        NAME_None,
        FVector::ZeroVector,
        EAttachLocation::KeepRelativeOffset,
        /*bStopWhenAttachedToDestroyed=*/ true
    );

    if (BattleBGMComp)
    {
        BattleBGMComp->SetVolumeMultiplier(BattleBGMVolume);
        if (!BattleBGMComp->IsPlaying())
            BattleBGMComp->Play();
    }
}

void ABossCharacter::StopBattleBGM(bool bFadeOut, float FadeOutTime)
{
    if (BattleBGMComp)
    {
        if (bFadeOut) BattleBGMComp->FadeOut(FadeOutTime, 0.f);
        else          BattleBGMComp->Stop();
        BattleBGMComp = nullptr;
    }
}
