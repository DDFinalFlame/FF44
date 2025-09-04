#include "AI/MonsterAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

AMonsterAIController::AMonsterAIController(const FObjectInitializer& ObjectInitializer)
// ▼ 기본 Subobject "PathFollowingComponent"를 Crowd로 교체
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
    PrimaryActorTick.bCanEverTick = true;

    // === Perception 구성 ===
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

    if (SightConfig)
    {
        SightConfig->SightRadius = SightRadius;             // 값 적당히(예: 1500.f)
        SightConfig->LoseSightRadius = LoseSightRadius;     // 예: 1800.f
        SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle; // 예: 90.f
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = false; // 디버깅 시 true로
    }
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->LoSHearingRange = LoSHearingRange;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = false; // 디버깅 시 true로
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

    const FAISenseID SightId = UAISense::GetSenseID<UAISense_Sight>();
    const FAISenseID HearingId = UAISense::GetSenseID<UAISense_Hearing>();

    if (Stimulus.Type == SightId)
    {
        // 팀 필터: 적만 대상
        if (!Actor || GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Hostile)
            return;

        if (Stimulus.WasSuccessfullySensed())
        {
            // 타겟 고정
            BB->SetValueAsObject(BBKey_TargetActor, Actor);
            BB->SetValueAsBool(BBKey_HasLineOfSight, true);

            // 마지막 알려진 위치 갱신(추후 가려져도 추적에 사용)
            BB->SetValueAsVector(BBKey_LastKnownLocation, Stimulus.StimulusLocation);
        }
        else
        {
            // 시야가 끊긴 것만 표시(타겟은 유지해서 뒤로 가도 잃어버리지 않음)
            //BB->SetValueAsBool(BBKey_HasLineOfSight, false);
        }
        return;
    }

    // -------- HEARING: 소리 위치만 저장(조사 이동) --------
    if (Stimulus.Type == HearingId)
    {
        if (!Stimulus.WasSuccessfullySensed())
            return;

        // (선택) 팀 필터: 소리의 instigator가 적일 때만 반응
        // Actor가 null일 수도 있어 ReportNoiseEvent에 따라 다름
       /* if (Actor && GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Hostile)
            return;*/

        const FVector HeardLoc = Stimulus.StimulusLocation;

        // 청각 전용 키 갱신
        BB->SetValueAsVector(TEXT("LastHeardLocation"), HeardLoc);
        BB->SetValueAsBool(TEXT("HeardNoise"), true);
        BB->SetValueAsFloat(TEXT("HeardNoiseTime"), GetWorld()->GetTimeSeconds());

        // 참고: TargetActor는 여기서 "절대" 건드리지 않음.
        // (원하면 TargetActor 비었을 때만 잡도록 조건부로 넣을 수도 있음)
        // if (!BB->GetValueAsObject(BBKey_TargetActor)) { BB->SetValueAsObject(BBKey_TargetActor, Actor); }
        // 또한 시각이 끊겨도 LastKnownLocation은 유지되므로, 여긴 굳이 안 건드려도 됨.
        return;
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