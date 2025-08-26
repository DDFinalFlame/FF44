#include "MonsterAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

AMonsterAIController::AMonsterAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // === Perception 구성 ===
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

    // 시야 설정
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle;

        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
    }

    // 청각 설정
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->LoSHearingRange = LoSHearingRange;

        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = false;
    }

    if (PerceptionComp)
    {
        PerceptionComp->ConfigureSense(*SightConfig);
        PerceptionComp->ConfigureSense(*HearingConfig);

        PerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());

        PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AMonsterAIController::OnTargetPerceptionUpdated);
        PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AMonsterAIController::OnPerceptionUpdated);
    }

    SetGenericTeamId(TeamId);
}

void AMonsterAIController::BeginPlay()
{
    Super::BeginPlay();
}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BehaviorTreeAsset)
    {
        UBlackboardComponent* BBComp = nullptr;
        UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BBComp);
        BlackboardComponent = BBComp;

        const bool bOK = RunBehaviorTree(BehaviorTreeAsset);
        if (!bOK)
        {
            UE_LOG(LogTemp, Warning, TEXT("RunBehaviorTree failed on %s"), *GetName());
        }
    }
}

void AMonsterAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!Actor || !BB) return;

    // 팀으로 필터: 적(Hostile)만 처리
    if (GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Hostile)
        return;

    if (Stimulus.WasSuccessfullySensed())
    {
        BB->SetValueAsObject(BBKey_TargetActor, Actor);
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
            BB->SetValueAsBool(BBKey_HasLineOfSight, true);

        BB->SetValueAsVector(BBKey_LastKnownLocation, Stimulus.StimulusLocation);
    }
    else
    {
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
            BB->SetValueAsBool(BBKey_HasLineOfSight, false);
    }
}

/** Perception이 배치로 업데이트될 때(참조: 여러 액터 동시 갱신) */
void AMonsterAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    // 필요 시 로깅/디버그 하시거나, 가장 가까운 타겟 재선정 등 확장 로직 추가 가능
}

void AMonsterAIController::UpdateBlackboardKeys()
{
    // Perception 기반 구조에서는 필수 아님.
    // 필요하면 여기서 추가적인 BB 동기화를 수행하세요.
}

ETeamAttitude::Type AMonsterAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
    // 1) Other가 컨트롤러인지 확인
    const AController* OtherCtrl = Cast<const AController>(&Other);

    // 2) 아니면 Pawn->Controller 따라가기
    if (!OtherCtrl)
    {
        const APawn* OtherPawn = Cast<const APawn>(&Other);
        OtherCtrl = OtherPawn ? OtherPawn->GetController() : nullptr;
    }

    // 3) 컨트롤러에서 팀 인터페이스 획득, 실패 시 Actor에도 시도
    const IGenericTeamAgentInterface* TeamAgent =
        OtherCtrl ? Cast<const IGenericTeamAgentInterface>(OtherCtrl)
        : Cast<const IGenericTeamAgentInterface>(&Other);

    if (!TeamAgent) return ETeamAttitude::Neutral;

    const FGenericTeamId OtherId = TeamAgent->GetGenericTeamId();
    if (OtherId == FGenericTeamId(1)) return ETeamAttitude::Friendly; // 몬스터
    if (OtherId == FGenericTeamId(0)) return ETeamAttitude::Hostile;  // 플레이어
    return ETeamAttitude::Neutral;
}