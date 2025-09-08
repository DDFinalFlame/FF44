// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTService_UpdateCrowdState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Monster/MonsterCharacter.h"   // AMonsterCharacter, EMonsterState ����
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
    NodeName = TEXT("Update State (Proximity + AllyCount �� Patrol/CombatReady/Attack)");
    bNotifyBecomeRelevant = false;
    Interval = 0.3f;            // �� �������� �ƴ� �ֱ� üũ
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
        // TODO: ��/���� üũ �ʿ� �� �߰�

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

    // 0) �ֱ� ���� ���� ���ĸ� ��ŵ(��ٿ�)
    if (LastStateChangeTimeKey.SelectedKeyType)
    {
        const float LastChange = BB->GetValueAsFloat(LastStateChangeTimeKey.SelectedKeyName);
        if (LastChange > 0.f && (Now - LastChange) < StateChangeCooldown)
        {
            return;
        }
    }

    // ���� ����(����) ��������
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
        // 1) �Ÿ� ����(������ ���)
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

        // 2) �����׸��ý� �ݰ�
        const float EnterR = ProximityRadius;
        const float ExitR = ProximityRadius + ProximityExitPadding;


        if (bUseProximity)
        {
            // Patrol ������ ���� ������ '����' �Ǵ�
            if (OldState == STATE_Patrol)
            {
                if (Dist <= EnterR)
                {
                    const int32 Allies = CountAttackingAllies(Target, CheckRadius, SelfPawn);
                    DesiredState = (Allies >= ThresholdCount) ? STATE_CombatReady : STATE_Attack;
                }
                // Dist > EnterR �̸� ������ Patrol ����
            }
            else
            {
                const int32 Allies = CountAttackingAllies(Target, CheckRadius, SelfPawn);
                DesiredState = (Allies >= ThresholdCount) ? STATE_CombatReady : STATE_Attack;

            }
        }
        else
        {
            // Proximity �̻�� ��: ���� ������ ����
            const int32 Allies = CountNearbyAllies(Target, CheckRadius, SelfPawn);
            DesiredState = (Allies >= ThresholdCount) ? STATE_CombatReady : STATE_Attack;
        }
    }

    // 3) ���� �ٲ� ���� ���(�ߺ� ���� ����)
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
    //ObjQ.AddObjectTypesToQuery(ECC_Pawn);          // ���Ͱ� Pawn�̸�
    //ObjQ.AddObjectTypesToQuery(ECC_WorldDynamic);  // ���Ͱ� WorldDynamic�̸� �ʿ� �� �߰�/����

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

    //        // ��/���� üũ �ʿ��ϸ� ���⼭ �߰� (��: MC->IsDead() ��)
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

    // �����ϸ� Pawn��
    FCollisionObjectQueryParams ObjQ;
    ObjQ.AddObjectTypesToQuery(ECC_Pawn);
    // ObjQ.AddObjectTypesToQuery(ECC_WorldDynamic); // ����/��Ʈ�ڽ� ���� ������ �ߺ��� �� �ʿ��� ����

    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams Q(TEXT("CountAttackingAllies"), /*bTraceComplex=*/false, SelfPawn);

    int32 Count = 0;
    const FVector CenterLoc = Center->GetActorLocation();

    if (World->OverlapMultiByObjectType(Hits, CenterLoc, FQuat::Identity, ObjQ, Sphere, Q))
    {
        // ���� ���� �ߺ� ����
        TSet<const AMonsterCharacter*> Seen;

        for (int32 i = 0; i < Hits.Num(); ++i)
        {
            AActor* A = Hits[i].GetActor();
            if (!A || A == SelfPawn) continue;

            AMonsterCharacter* MC = Cast<AMonsterCharacter>(A);
            if (!MC) continue;

            if (Seen.Contains(MC)) continue;  // �̹� �� ����
            Seen.Add(MC);

            // ��/���� üũ �ʿ�� �߰� (��: if (MC->IsDead()) continue;)
            if (MC->IsAttacking())
            {
                ++Count;
            }
        }
    }
    return Count;
}