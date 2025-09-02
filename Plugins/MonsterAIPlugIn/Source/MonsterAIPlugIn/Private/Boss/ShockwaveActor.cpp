// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/ShockwaveActor.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "MonsterTags.h"
#include "DrawDebugHelpers.h"

// Sets default values
AShockwaveActor::AShockwaveActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    SetRootComponent(Sphere);

    // �浹 �⺻��: Pawn�� ������
    Sphere->InitSphereRadius(50.f);
    Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Sphere->SetCollisionObjectType(ECC_WorldDynamic);
    Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Sphere->SetGenerateOverlapEvents(true);

    Sphere->OnComponentBeginOverlap.AddDynamic(this, &AShockwaveActor::OnSphereBeginOverlap);

    NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
    NiagaraComp->SetupAttachment(RootComponent);
    NiagaraComp->bAutoActivate = false;
}

void AShockwaveActor::SetInstigatorActor(AActor* _Instigator)
{
    InstigatorActor = _Instigator;
}


void AShockwaveActor::BeginPlay()
{
    Super::BeginPlay();

    CurrentRadius = StartRadius;
    Sphere->SetSphereRadius(CurrentRadius, true);
    Sphere->UpdateOverlaps();

    if (NiagaraSystemAsset)
    {
        NiagaraComp->SetAsset(NiagaraSystemAsset);
        NiagaraComp->Activate(true);

        // NS ���� ����: �����ϸ� ù �� ����
        UpdateNiagaraScale();
    }


    // �浹 ä�� Ŀ���͸���� �ʿ��ϸ� ���⼭ �ݿ�
    Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    Sphere->SetCollisionResponseToChannel(PawnChannel, ECR_Overlap);
}

void AShockwaveActor::Tick(float _DeltaSeconds)
{
    Super::Tick(_DeltaSeconds);

    UpdateRadius(_DeltaSeconds);

    if (bDrawDebug)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation(), CurrentRadius, 24, FColor::Cyan, false, 0.f, 0, 2.f);
    }

    // ���� ����
    if (CurrentRadius >= MaxRadius)
    {
        Destroy();
    }
}

void AShockwaveActor::UpdateRadius(float _DeltaSeconds)
{
    CurrentRadius = FMath::Min(CurrentRadius + ExpandSpeed * _DeltaSeconds, MaxRadius);

    Sphere->SetSphereRadius(CurrentRadius, true);
    Sphere->UpdateOverlaps();

    // NS ���־� ������ ����ȭ
    UpdateNiagaraScale();
}

void AShockwaveActor::OnSphereBeginOverlap(UPrimitiveComponent* _OverlappedComponent, AActor* _OtherActor,
    UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex,
    bool _bFromSweep, const FHitResult& _SweepResult)
{
    if (!_OtherActor || _OtherActor == this) return;

    // ù ��ħ���� ����
    if (HitActors.Contains(_OtherActor)) return;

    // �ڽ�(����)���Դ� ���� X
    if (InstigatorActor.IsValid() && _OtherActor == InstigatorActor.Get()) return;

    ApplyDamageOnce(_OtherActor);
}




void AShockwaveActor::ApplyDamageOnce(AActor* _TargetActor)
{
    if (!_TargetActor) return;

    // ��� ASC ã��(�÷��̾�/�� ����)
    UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(_TargetActor);
    if (!TargetASC) return;

    // �ν�Ƽ������ ASC (����)
    UAbilitySystemComponent* InstASC = nullptr;
    if (InstigatorActor.IsValid())
    {
        InstASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstigatorActor.Get());
    }

    if (!GE_ShockwaveDamage)
    {
        // GE�� ��� �־ SetByCaller�� ���� ������Ʈ��� ������ ����
        return;
    }

    // ���ؽ�Ʈ ����
    UAbilitySystemComponent* ContextASC = InstASC ? InstASC : TargetASC; // ������ ASC ����
    FGameplayEffectContextHandle Ctx = ContextASC->MakeEffectContext();

    AActor* InstigatorA = InstigatorActor.IsValid() ? InstigatorActor.Get() : this;
    Ctx.AddInstigator(InstigatorA, this);

    // ���� ����
    FGameplayEffectSpecHandle Spec = ContextASC->MakeOutgoingSpec(GE_ShockwaveDamage, 1.f, Ctx);
    if (Spec.IsValid())
    {
        // ������ ������ SetByCaller Ű ���(��: MonsterTags::Data_Damage)
        Spec.Data->SetSetByCallerMagnitude(MonsterTags::Data_Damage, ShockwaveDamage);

        TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

        HitActors.Add(_TargetActor);
    }
}


void AShockwaveActor::UpdateNiagaraScale()
{
    if (!NiagaraComp) return;

    const float SafeBase = FMath::Max(1.f, BaseVisualRadius);
    const float Scale = FMath::Max(0.01f, CurrentRadius / SafeBase);

    if (bUseScaleOverall && NSParam_ScaleOverall != NAME_None)
    {
        NiagaraComp->SetVariableFloat(NSParam_ScaleOverall, Scale);
    }
    else
    {
        if (NSParam_ScaleX != NAME_None) NiagaraComp->SetVariableFloat(NSParam_ScaleX, Scale);
        if (NSParam_ScaleY != NAME_None) NiagaraComp->SetVariableFloat(NSParam_ScaleY, Scale);
        if (NSParam_ScaleZ != NAME_None) NiagaraComp->SetVariableFloat(NSParam_ScaleZ, 1.0f); // ��� ���̸� Z�� 1
    }
}