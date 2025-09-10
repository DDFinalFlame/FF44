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
            // 다음 틱에 다시 시도 (BB 초기화 지연 대비)
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

    const FVector F = GetActorForwardVector(); // 앞
    const FVector R = GetActorRightVector();   // 오른쪽

    const FVector Base = Loc - F * DeathSpawnBackOffset + FVector(0, 0, DeathSpawnZOffset);
    const FVector LeftPos = Base - R * DeathSpawnSideOffset; // 왼쪽 뒤
    const FVector RightPos = Base + R * DeathSpawnSideOffset; // 오른쪽 뒤

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
        // ASC가 아직 초기화 전이면 다음 틱에 재시도
        GetWorldTimerManager().SetTimerForNextTick(this, &ABossCharacter::ActivatePhaseWatcherOnce);
        return;
    }

    // ASC의 ActorInfo가 이 보스에 바인딩됐는지 확인 (InitAbilityActorInfo 이후여야 함)
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

    // 디버그
    UE_LOG(LogTemp, Warning, TEXT("Boss Landed!"));

    // Phase2 GA에서 기다리는 착지 이벤트 송신
    FGameplayEventData Data;
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        this, MonsterTags::Event_Boss_Land, Data);
}

void ABossCharacter::SpawnAndAttachWeapons()
{
    if (!HasAuthority()) return;
    if (Weapon || !WeaponClass) return;

    // 메시/소켓 준비가 늦을 수 있으니 체크 후 지연 재시도
    if (!GetMesh())
    {
        GetWorldTimerManager().SetTimerForNextTick(this, &ABossCharacter::SpawnAndAttachWeapons);
        return;
    }

    // 소켓 존재성 검사 (하나라도 없으면 다음 틱 재시도)
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

        // 베이스 무기의 Init: 소켓에 부착 + 소유자 바인딩
        NewWeapon->Init(this, GetMesh(), SocketName);

        // 베이스에 등록 → Begin/EndAttackWindow가 자동으로 ‘모든 무기’ 제어
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
