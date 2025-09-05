// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTService_UpdateCrowdState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Monster/MonsterCharacter.h"   // AMonsterCharacter, EMonsterState 가정
#include "AIController.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"      // FOverlapResult / FCollisionShape / FCollisionObjectQueryParams
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"
#include "Engine/HitResult.h"

static constexpr int STATE_Patrol = 2;
static constexpr int STATE_Attack = 3;
static constexpr int STATE_CombatReady = 4;

UBTService_UpdateCrowdState::UBTService_UpdateCrowdState()
{
    NodeName = TEXT("Update State (Proximity + AllyCount → Patrol/CombatReady/Attack)");
    bNotifyBecomeRelevant = false;
    Interval = 0.3f;            // 매 프레임이 아닌 주기 체크
    RandomDeviation = 0.1f;
}

int32 UBTService_UpdateCrowdState::CountNearbyAllies(AActor* Center, float Radius, APawn* SelfPawn) const
{
    if (!Center) return 0;
    UWorld* World = Center->GetWorld();
    if (!World) return 0;

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(World, AMonsterCharacter::StaticClass(), Found);

    int32 Count = 0;
    for (int32 i = 0; i < Found.Num(); ++i)
    {
        AMonsterCharacter* MC = Cast<AMonsterCharacter>(Found[i]);
        if (!MC) continue;
        if (MC == SelfPawn) continue;
        // TODO: 팀/생존 체크 필요 시 추가

        const float DistSq = FVector::DistSquared(Center->GetActorLocation(), MC->GetActorLocation());
        if (DistSq <= Radius * Radius) ++Count;
    }
    return Count;
}

void UBTService_UpdateCrowdState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    AAIController* AIC = OwnerComp.GetAIOwner();
    APawn* SelfPawn = AIC ? AIC->GetPawn() : nullptr;
    if (!SelfPawn) return;

    const float Now = SelfPawn->GetWorld()->GetTimeSeconds();

    // 0) 최근 상태 변경 직후면 스킵(쿨다운)
    if (LastStateChangeTimeKey.SelectedKeyType)
    {
        const float LastChange = BB->GetValueAsFloat(LastStateChangeTimeKey.SelectedKeyName);
        if (LastChange > 0.f && (Now - LastChange) < StateChangeCooldown)
        {
            return;
        }
    }

    // 현재 상태(숫자) 가져오기
    const int OldState = BB->GetValueAsInt(MonsterStateKey.SelectedKeyName);
    if (OldState == 0)
        return;
    int DesiredState = OldState;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!Target)
    {
        DesiredState = STATE_Patrol;
    }
    else
    {
        // 1) 거리 재사용(없으면 계산)
        float Dist = -1.f;
        if (DistanceToTargetKey.SelectedKeyType)
        {
            const float FromBB = BB->GetValueAsFloat(DistanceToTargetKey.SelectedKeyName);
            if (FromBB > 0.f && FromBB < FLT_MAX) Dist = FromBB;
        }
        if (Dist < 0.f)
        {
            Dist = FVector::Dist(Target->GetActorLocation(), SelfPawn->GetActorLocation());
            if (DistanceToTargetKey.SelectedKeyType)
                BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Dist);
        }

        // 2) 히스테리시스 반경
        const float EnterR = ProximityRadius;
        const float ExitR = ProximityRadius + ProximityExitPadding;


        if (bUseProximity)
        {
            // Patrol 상태일 때만 전투권 '진입' 판단
            if (OldState == STATE_Patrol)
            {
                if (Dist <= EnterR)
                {
                    const int32 Allies = CountAttackingAllies(Target, CheckRadius, SelfPawn);
                    DesiredState = (Allies >= ThresholdCount) ? STATE_CombatReady : STATE_Attack;
                }
                // Dist > EnterR 이면 여전히 Patrol 유지
            }
            else
            {
                const int32 Allies = CountAttackingAllies(Target, CheckRadius, SelfPawn);
                DesiredState = (Allies >= ThresholdCount) ? STATE_CombatReady : STATE_Attack;

            }
        }
        else
        {
            // Proximity 미사용 시: 기존 로직과 동일
            const int32 Allies = CountNearbyAllies(Target, CheckRadius, SelfPawn);
            DesiredState = (Allies >= ThresholdCount) ? STATE_CombatReady : STATE_Attack;
        }
    }

    // 3) 값이 바뀔 때만 기록(중복 갱신 차단)
    if (DesiredState != OldState)
    {
        BB->SetValueAsInt(MonsterStateKey.SelectedKeyName, DesiredState);

        if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(SelfPawn))
        {
            MC->SetMonsterState(static_cast<EMonsterState>(DesiredState));
        }

        if (LastStateChangeTimeKey.SelectedKeyType)
            BB->SetValueAsFloat(LastStateChangeTimeKey.SelectedKeyName, Now);
    }
}


int32 UBTService_UpdateCrowdState::CountAttackingAllies(AActor* Center, float Radius, APawn* SelfPawn) const
{
    //if (!Center) return 0;
    //UWorld* World = Center->GetWorld();
    //if (!World) return 0;

    //TArray<FOverlapResult> Hits;
    //FCollisionObjectQueryParams ObjQ;
    //ObjQ.AddObjectTypesToQuery(ECC_Pawn);          // 몬스터가 Pawn이면
    //ObjQ.AddObjectTypesToQuery(ECC_WorldDynamic);  // 몬스터가 WorldDynamic이면 필요 시 추가/수정

    //FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
    //FCollisionQueryParams Q(TEXT("CountAttackingAllies"), /*bTraceComplex=*/false, SelfPawn);

    //int32 Count = 0;
    //const FVector CenterLoc = Center->GetActorLocation();

    //if (World->OverlapMultiByObjectType(Hits, CenterLoc, FQuat::Identity, ObjQ, Sphere, Q))
    //{
    //    for (int32 i = 0; i < Hits.Num(); ++i)
    //    {
    //        AActor* A = Hits[i].GetActor();
    //        if (!A || A == SelfPawn) continue;

    //        AMonsterCharacter* MC = Cast<AMonsterCharacter>(A);
    //        if (!MC) continue;

    //        // 팀/생존 체크 필요하면 여기서 추가 (예: MC->IsDead() 등)
    //        if (MC->IsAttacking())
    //        {
    //            ++Count;
    //        }
    //    }
    //}
    //return Count;
    if (!Center) return 0;
    UWorld* World = Center->GetWorld();
    if (!World) return 0;

    TArray<FOverlapResult> Hits;

    // 가능하면 Pawn만
    FCollisionObjectQueryParams ObjQ;
    ObjQ.AddObjectTypesToQuery(ECC_Pawn);
    // ObjQ.AddObjectTypesToQuery(ECC_WorldDynamic); // 무기/히트박스 등이 들어오면 중복↑ → 필요할 때만

    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams Q(TEXT("CountAttackingAllies"), /*bTraceComplex=*/false, SelfPawn);

    int32 Count = 0;
    const FVector CenterLoc = Center->GetActorLocation();

    if (World->OverlapMultiByObjectType(Hits, CenterLoc, FQuat::Identity, ObjQ, Sphere, Q))
    {
        // 같은 몬스터 중복 방지
        TSet<const AMonsterCharacter*> Seen;

        for (int32 i = 0; i < Hits.Num(); ++i)
        {
            AActor* A = Hits[i].GetActor();
            if (!A || A == SelfPawn) continue;

            AMonsterCharacter* MC = Cast<AMonsterCharacter>(A);
            if (!MC) continue;

            if (Seen.Contains(MC)) continue;  // 이미 센 몬스터
            Seen.Add(MC);

            // 팀/생존 체크 필요시 추가 (예: if (MC->IsDead()) continue;)
            if (MC->IsAttacking())
            {
                ++Count;
            }
        }
    }
    return Count;
}