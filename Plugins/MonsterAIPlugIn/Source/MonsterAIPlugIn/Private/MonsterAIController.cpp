#include "MonsterAIController.h"
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
// �� �⺻ Subobject "PathFollowingComponent"�� Crowd�� ��ü
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
    PrimaryActorTick.bCanEverTick = true;

    // === Perception ���� ===
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

    if (SightConfig)
    {
        SightConfig->SightRadius = SightRadius;             // �� ������(��: 1500.f)
        SightConfig->LoseSightRadius = LoseSightRadius;     // ��: 1800.f
        SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle; // ��: 90.f
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = false; // ����� �� true��
    }
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->LoSHearingRange = LoSHearingRange;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = false; // ����� �� true��
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

    // ������ ����: ��(Hostile)�� ó��
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

/** Perception�� ��ġ�� ������Ʈ�� ��(����: ���� ���� ���� ����) */
void AMonsterAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    // �ʿ� �� �α�/����� �Ͻðų�, ���� ����� Ÿ�� �缱�� �� Ȯ�� ���� �߰� ����
}

void AMonsterAIController::UpdateBlackboardKeys()
{
    // Perception ��� ���������� �ʼ� �ƴ�.
    // �ʿ��ϸ� ���⼭ �߰����� BB ����ȭ�� �����ϼ���.
}

ETeamAttitude::Type AMonsterAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
    // 1) Other�� ��Ʈ�ѷ����� Ȯ��
    const AController* OtherCtrl = Cast<const AController>(&Other);

    // 2) �ƴϸ� Pawn->Controller ���󰡱�
    if (!OtherCtrl)
    {
        const APawn* OtherPawn = Cast<const APawn>(&Other);
        OtherCtrl = OtherPawn ? OtherPawn->GetController() : nullptr;
    }

    // 3) ��Ʈ�ѷ����� �� �������̽� ȹ��, ���� �� Actor���� �õ�
    const IGenericTeamAgentInterface* TeamAgent =
        OtherCtrl ? Cast<const IGenericTeamAgentInterface>(OtherCtrl)
        : Cast<const IGenericTeamAgentInterface>(&Other);

    if (!TeamAgent) return ETeamAttitude::Neutral;

    const FGenericTeamId OtherId = TeamAgent->GetGenericTeamId();
    if (OtherId == FGenericTeamId(1)) return ETeamAttitude::Friendly; // ����
    if (OtherId == FGenericTeamId(0)) return ETeamAttitude::Hostile;  // �÷��̾�
    return ETeamAttitude::Neutral;
}