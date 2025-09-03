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

    // BeginOverlap 바인딩
    Sphere->OnComponentBeginOverlap.AddDynamic(this, &AShockwaveActor::OnSphereBeginOverlap);

    // 필요시 수명 제한(안전망)
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
            // 1) 월드 스케일 강제 (파라미터 무시)
            NiagaraComp->SetWorldScale3D(FVector(10.f));

            // 2) 파라미터도 같이 던져보기
            NiagaraComp->SetVariableFloat(TEXT("Scale Overall"), 10.f);
            //////////////////////
            // 바운즈 팽창: 급격히 커지는 이펙트가 카메라 프러스텀에서 잘리지 않도록
            //  값은 적당히 크게(예: 10~30). 필요시 프로젝트에 맞게 조절하세요.
            NiagaraComp->SetBoundsScale(20.0f);

            // 시작부터 눈에 보이게(유저 파라미터가 안 물려 있어도 보이도록 안전망)
            const float StartScale =
                FMath::Max(0.05f, Sphere->GetUnscaledSphereRadius() / FMath::Max(1.f, VisualBaseRadius));
            NiagaraComp->SetWorldScale3D(FVector(StartScale));
        }
    }

    // 시각/판정 공통 스타트 반경
    CurrentRadius = Sphere->GetUnscaledSphereRadius();
}

void AShockwaveActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // 시각용 반경 증가 (멀티는 신경 안 쓰신다 하셔서 공통으로 둡니다)
    CurrentRadius = FMath::Min(CurrentRadius + ExpandSpeed * DeltaSeconds, MaxRadius);

    // 서버면 실제 충돌 반경만 갱신
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
        GetActorLocation(),           // 중심: 액터 위치
        CurrentRadius,                // 반경
        32,                           // 세그먼트 수 (많을수록 매끄럽게)
        FColor(200, 0, 200, 128),     // 색상 (보라)
        false,                        // 영구 표시 여부 (false = 일정 시간 후 사라짐)
        -1.0f,                        // Duration (음수 = 1프레임)
        0,                            // Depth Priority
        1.5f                          // 선 두께
    );

    if (NiagaraComp)
    {
        const float VisualScale = FMath::Max(0.01f, CurrentRadius / FMath::Max(1.f, VisualBaseRadius));

        //1) 유저 파라미터(있으면 동작)
        NiagaraComp->SetVariableFloat(TEXT("Scale Overall"), VisualScale);
        NiagaraComp->SetVariableFloat(TEXT("Scale X Minimum"), VisualScale);
        NiagaraComp->SetVariableFloat(TEXT("Scale X Maximum"), VisualScale);
        NiagaraComp->SetVariableFloat(TEXT("Scale Y Minimum"), VisualScale);
        NiagaraComp->SetVariableFloat(TEXT("Scale Y Maximum"), VisualScale);
        NiagaraComp->SetVariableFloat(TEXT("Scale Z Minimum"), VisualScale);
        NiagaraComp->SetVariableFloat(TEXT("Scale Z Maximum"), VisualScale);

        //2) 안전망: 파라미터가 안 물려 있어도 ‘컴포넌트 스케일’로 반드시 보이게
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

    // 한 번만
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

    // 대상 ASC
    UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
    if (!TargetASC)
        return;

    // EffectContext: Instigator=보스, Causer=이 ShockwaveActor
    FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
    Ctx.AddInstigator(SourceActor ? SourceActor : GetOwner(), this);

    FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageGE, 1.f, Ctx);
    if (!Spec.IsValid())
        return;

    // SetByCaller로 피해 전달 (프로젝트에서 쓰는 태그에 맞춰주세요)
    Spec.Data->SetSetByCallerMagnitude(MonsterTags::Data_Damage, Damage);

    SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}