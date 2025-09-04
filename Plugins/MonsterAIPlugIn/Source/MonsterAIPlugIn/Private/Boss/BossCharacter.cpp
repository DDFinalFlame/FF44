// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/BossCharacter.h"
#include "Boss/BossHPBarWidget.h"
#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterTags.h"
#include "Data/staticName.h"
#include "Boss/BossMeleeWeapon.h"
#include "Kismet/GameplayStatics.h"



ABossCharacter::ABossCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

}

void ABossCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        SpawnAndAttachWeapons();
        ActivatePhaseWatcherOnce();
    }


    if (BossHPWidgetClass)
    {
        BossHPWidget = CreateWidget<UBossHPBarWidget>(
            UGameplayStatics::GetPlayerController(this, 0),
            BossHPWidgetClass
        );

        if (BossHPWidget)
        {
            BossHPWidget->AddToViewport(50);

            // ȭ�� ��� �߾� ����
            BossHPWidget->SetAnchorsInViewport(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
            BossHPWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.0f));
            BossHPWidget->SetPositionInViewport(FVector2D(0.0f, 40.0f), false);

            BossHPWidget->InitBossName(FText::FromString(TEXT("Boss")));
        }
    }

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            UMonsterAttributeSet::GetHealthAttribute()
        ).AddUObject(this, &ABossCharacter::OnHealthChanged);
    }

}


void ABossCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    if (BossHPWidget)
    {
        BossHPWidget->UpdateHP(Data.NewValue, GetMaxHealth());
    }

    if (Data.NewValue <= 0.f && BossHPWidget)
    {
        BossHPWidget->RemoveFromParent();
        BossHPWidget = nullptr;
    }
}

float ABossCharacter::GetMaxHealth() const
{
    if (AbilitySystemComponent)
    {
        const UMonsterAttributeSet* AttrSet = AbilitySystemComponent->GetSet<UMonsterAttributeSet>();
        if (AttrSet)
        {
            return AttrSet->GetMaxHealth(); 
        }
    }
    return 0.f;
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