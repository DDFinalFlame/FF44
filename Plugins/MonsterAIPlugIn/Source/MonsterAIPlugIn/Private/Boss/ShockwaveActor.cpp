// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/ShockwaveActor.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Kismet/GameplayStatics.h"
#include "MonsterTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "DrawDebugHelpers.h"

AShockwaveActor::AShockwaveActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(false);

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    SetRootComponent(Sphere);

    Sphere->SetSphereRadius(10.f);
    Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Sphere->SetCollisionObjectType(ECC_WorldDynamic);
    Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    Sphere->SetCollisionResponseToChannel(PawnChannel, ECR_Overlap);
    Sphere->SetGenerateOverlapEvents(true);

    // BeginOverlap ���ε�
    Sphere->OnComponentBeginOverlap.AddDynamic(this, &AShockwaveActor::OnSphereBeginOverlap);

    // �ʿ�� ���� ����(������)
    InitialLifeSpan = 5.0f;
}

void AShockwaveActor::Initialize(UAbilitySystemComponent* InSourceASC,
    TSubclassOf<UGameplayEffect> InDamageGE,
    float InDamage,
    float InMaxRadius,
    float InExpandSpeed,
    AActor* InSourceActor)
{
    SourceASC = InSourceASC;
    DamageGE = InDamageGE;
    Damage = InDamage;
    MaxRadius = FMath::Max(10.f, InMaxRadius);
    ExpandSpeed = FMath::Max(1.f, InExpandSpeed);
    SourceActor = InSourceActor;
}

void AShockwaveActor::BeginPlay()
{
    Super::BeginPlay();

    if (NiagaraTemplate)
    {
        NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
            NiagaraTemplate, RootComponent, NAME_None,
            FVector::ZeroVector, FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true, true
        );

        if (NiagaraComp)
        {
            NiagaraComp->Activate(true);
            NiagaraComp->SetHiddenInGame(false);
            NiagaraComp->SetVisibility(true, true);
            ////////////////////////
            // 1) ���� ������ ���� (�Ķ���� ����)
            NiagaraComp->SetWorldScale3D(FVector(10.f));

            // 2) �Ķ���͵� ���� ��������
            NiagaraComp->SetVariableFloat(TEXT("Scale Overall"), 10.f);
            //////////////////////
            // �ٿ��� ��â: �ް��� Ŀ���� ����Ʈ�� ī�޶� �������ҿ��� �߸��� �ʵ���
            //  ���� ������ ũ��(��: 10~30). �ʿ�� ������Ʈ�� �°� �����ϼ���.
            NiagaraComp->SetBoundsScale(20.0f);

            // ���ۺ��� ���� ���̰�(���� �Ķ���Ͱ� �� ���� �־ ���̵��� ������)
            const float StartScale =
                FMath::Max(0.05f, Sphere->GetUnscaledSphereRadius() / FMath::Max(1.f, VisualBaseRadius));
            NiagaraComp->SetWorldScale3D(FVector(StartScale));
        }
    }

    // �ð�/���� ���� ��ŸƮ �ݰ�
    CurrentRadius = Sphere->GetUnscaledSphereRadius();
}

void AShockwaveActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // �ð��� �ݰ� ���� (��Ƽ�� �Ű� �� ���Ŵ� �ϼż� �������� �Ӵϴ�)
    CurrentRadius = FMath::Min(CurrentRadius + ExpandSpeed * DeltaSeconds, MaxRadius);

    // ������ ���� �浹 �ݰ游 ����
    if (IsServerAuth())
    {
        Sphere->SetSphereRadius(CurrentRadius, true);
        if (CurrentRadius >= MaxRadius - KINDA_SMALL_NUMBER)
        {
            Destroy();
            return;
        }
    }

    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),           // �߽�: ���� ��ġ
        CurrentRadius,                // �ݰ�
        32,                           // ���׸�Ʈ �� (�������� �Ų�����)
        FColor(200, 0, 200, 128),     // ���� (����)
        false,                        // ���� ǥ�� ���� (false = ���� �ð� �� �����)
        -1.0f,                        // Duration (���� = 1������)
        0,                            // Depth Priority
        1.5f                          // �� �β�
    );

    if (NiagaraComp)
    {
        const float VisualScale = FMath::Max(0.01f, CurrentRadius / FMath::Max(1.f, VisualBaseRadius));

        //1) ���� �Ķ����(������ ����)
        NiagaraComp->SetVariableFloat(TEXT("Scale Overall"), VisualScale);
        NiagaraComp->SetVariableFloat(TEXT("Scale X Minimum"), VisualScale);
        NiagaraComp->SetVariableFloat(TEXT("Scale X Maximum"), VisualScale);
        NiagaraComp->SetVariableFloat(TEXT("Scale Y Minimum"), VisualScale);
        NiagaraComp->SetVariableFloat(TEXT("Scale Y Maximum"), VisualScale);
        NiagaraComp->SetVariableFloat(TEXT("Scale Z Minimum"), VisualScale);
        NiagaraComp->SetVariableFloat(TEXT("Scale Z Maximum"), VisualScale);

        //2) ������: �Ķ���Ͱ� �� ���� �־ ��������Ʈ �����ϡ��� �ݵ�� ���̰�
        NiagaraComp->SetWorldScale3D(FVector(VisualScale));
    }
}

void AShockwaveActor::OnSphereBeginOverlap(UPrimitiveComponent* Overlapped,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!IsServerAuth())
        return;

    if (!OtherActor || OtherActor == this || OtherActor == SourceActor)
        return;

    // �� ����
    if (HitActors.Contains(OtherActor))
        return;

    HitActors.Add(OtherActor);
    ApplyDamageToActor(OtherActor);

    FGameplayEventData Payload;
    Payload.EventTag = MonsterTags::Event_Player_Hit;
    Payload.Instigator = this;
    Payload.Target = OtherActor;
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, Payload.EventTag, Payload);

}

void AShockwaveActor::ApplyDamageToActor(AActor* TargetActor)
{
    if (!SourceASC || !DamageGE || Damage <= 0.f)
        return;

    // ��� ASC
    UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
    if (!TargetASC)
        return;

    // EffectContext: Instigator=����, Causer=�� ShockwaveActor
    FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
    Ctx.AddInstigator(SourceActor ? SourceActor : GetOwner(), this);

    FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageGE, 1.f, Ctx);
    if (!Spec.IsValid())
        return;

    // SetByCaller�� ���� ���� (������Ʈ���� ���� �±׿� �����ּ���)
    Spec.Data->SetSetByCallerMagnitude(MonsterTags::Data_Damage, Damage);

    SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}