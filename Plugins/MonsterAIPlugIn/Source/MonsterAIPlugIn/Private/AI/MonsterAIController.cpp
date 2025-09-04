#include "AI/MonsterAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
//#include "Navigation/CrowdFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h" 

AMonsterAIController::AMonsterAIController(const FObjectInitializer& ObjectInitializer)
// �⺻ Subobject "PathFollowingComponent"�� Crowd�� ��ü (����) 
// : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
// �⺻ PathFollowing ���(����)
    : Super(ObjectInitializer /* .SetDefaultSubobjectClass<UPathFollowingComponent>(TEXT("PathFollowingComponent")) ���� ���� */)
{
    PrimaryActorTick.bCanEverTick = true;

    // === Perception ���� ===
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

    if (SightConfig)
    {
        SightConfig->SightRadius = SightRadius;
        SightConfig->LoseSightRadius = LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
    }
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->LoSHearingRange = LoSHearingRange; // (UE5.6���� deprecated ���) �� �����ϸ� �����ϰ� HearingRange�� ��� ����
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


void AMonsterAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    // ����/�ߴ� �� ���ڸ� ���� ����: ��� ����
    if (Result.Code != EPathFollowingResult::Success)
    {
        if (ACharacter* C = Cast<ACharacter>(GetPawn()))
            if (auto* M = C->GetCharacterMovement())
                M->StopMovementImmediately();
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
        // �� ����: ���� ���
        if (!Actor || GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Hostile)
            return;

        if (Stimulus.WasSuccessfullySensed())
        {
            // Ÿ�� ����
            BB->SetValueAsObject(BBKey_TargetActor, Actor);
            BB->SetValueAsBool(BBKey_HasLineOfSight, true);

            // ������ �˷��� ��ġ ����(���� �������� ������ ���)
            BB->SetValueAsVector(BBKey_LastKnownLocation, Stimulus.StimulusLocation);
        }
        else
        {
            // �þ߰� ���� �͸� ǥ��(Ÿ���� �����ؼ� �ڷ� ���� �Ҿ������ ����)
            //BB->SetValueAsBool(BBKey_HasLineOfSight, false);
        }
        return;
    }

    // -------- HEARING: �Ҹ� ��ġ�� ����(���� �̵�) --------
    if (Stimulus.Type == HearingId)
    {
        if (!Stimulus.WasSuccessfullySensed())
            return;

        // (����) �� ����: �Ҹ��� instigator�� ���� ���� ����
        // Actor�� null�� ���� �־� ReportNoiseEvent�� ���� �ٸ�
       /* if (Actor && GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Hostile)
            return;*/

        const FVector HeardLoc = Stimulus.StimulusLocation;

        // û�� ���� Ű ����
        BB->SetValueAsVector(TEXT("LastHeardLocation"), HeardLoc);
        BB->SetValueAsBool(TEXT("HeardNoise"), true);
        BB->SetValueAsFloat(TEXT("HeardNoiseTime"), GetWorld()->GetTimeSeconds());

        // ����: TargetActor�� ���⼭ "����" �ǵ帮�� ����.
        // (���ϸ� TargetActor ����� ���� �⵵�� ���Ǻη� ���� ���� ����)
        // if (!BB->GetValueAsObject(BBKey_TargetActor)) { BB->SetValueAsObject(BBKey_TargetActor, Actor); }
        // ���� �ð��� ���ܵ� LastKnownLocation�� �����ǹǷ�, ���� ���� �� �ǵ���� ��.
        return;
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