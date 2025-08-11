
#include "MonsterCharacter.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.h"
#include "GE_MonsterDefaultStat.h"
#include "GA_MonsterAttack.h"
#include "GameFramework/PlayerState.h" // OnRep_PlayerState ���� �ʿ�
#include "MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"  
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AMonsterCharacter::AMonsterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UMonsterAttributeSet>(TEXT("AttributeSet"));

	AIControllerClass = AMonsterAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;


}

// Called when the game starts or when spawned
void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
    // ASC ActorInfo ���� �ʱ�ȭ
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
    }

    // Definition �ε�
    if (!MonsterDefinition.IsValid())
        MonsterDefinition.LoadSynchronous();
    UMonsterDefinition* Def = MonsterDefinition.Get();
    if (!Def) { UE_LOG(LogTemp, Warning, TEXT("MonsterDefinition not set.")); return; }

    // Mesh / AnimBP ����
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        if (!Def->SkeletalMesh.IsValid()) Def->SkeletalMesh.LoadSynchronous();
        MeshComp->SetSkeletalMesh(Def->SkeletalMesh.Get());

        if (!Def->AnimBlueprint.IsValid()) Def->AnimBlueprint.LoadSynchronous();
        MeshComp->SetAnimInstanceClass(Def->AnimBlueprint.Get());
    }

    // Ability �ο�
    if (AbilitySystemComponent)
    {
        int32 AbilityIndex = 0;
        for (int32 i = 0; i < Def->AbilitiesToGrant.Num(); ++i)
        {
            TSubclassOf<UGameplayAbility> GA = Def->AbilitiesToGrant[i];
            if (GA)
            {
                AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GA, 1, AbilityIndex++));
            }
        }
    }

    // DT���� ���� �о� SetByCaller GE ����
    if (MonsterStatTable && Def->StatRowName.IsValid())
    {
        if (FMonsterStatRow* Row = MonsterStatTable->FindRow<FMonsterStatRow>(Def->StatRowName, TEXT("MonsterInit")))
        {
            ApplyInitStats(*Row, Def->InitStatGE_SetByCaller);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Row %s not found in MonsterStatTable."), *Def->StatRowName.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MonsterStatTable or StatRowName not set."));
    }

    // AI(BT/BB) ����
    if (AMonsterAIController* AIC = Cast<AMonsterAIController>(GetController()))
    {
        // ����Ʈ ���۷��� ���� �ε�
        if (!Def->BehaviorTree.IsValid())    Def->BehaviorTree.LoadSynchronous();
        if (!Def->BlackboardAsset.IsValid()) Def->BlackboardAsset.LoadSynchronous();

        UBlackboardComponent* BBComp = nullptr; // out �Ķ���͸� ���� lvalue �غ�
        const bool bBBInited = AIC->UseBlackboard(Def->BlackboardAsset.Get(), BBComp);
        if (bBBInited && BBComp)
        {
            // BT ����
            AIC->RunBehaviorTree(Def->BehaviorTree.Get());

            // �ʿ� �� �ʱ� BB �� ����
            if (MonsterStatTable && Def->StatRowName.IsValid())
            {
                if (FMonsterStatRow* Row2 = MonsterStatTable->FindRow<FMonsterStatRow>(Def->StatRowName, TEXT("MonsterInit")))
                {
                    BBComp->SetValueAsFloat(TEXT("DetectDistance"), Row2->DetectDistance);
                    BBComp->SetValueAsFloat(TEXT("AttackDistance"), Row2->AttackDistance);
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UseBlackboard failed or BBComp is null."));
        }
    }

	// ������ GE ���� ����
	//if (HasAuthority() && AbilitySystemComponent)
	//{
	//	// C++�� ���� GameplayEffect�� ����
	//	UGE_MonsterDefaultStat* DefaultStatEffect = NewObject<UGE_MonsterDefaultStat>();
	//	if (DefaultStatEffect)
	//	{
	//		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	//		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultStatEffect->GetClass(), 1.f, Context);

	//		if (SpecHandle.IsValid())
	//		{
	//			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	//		}
	//	}
	//	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UGA_MonsterAttack::StaticClass(), 1, 0));

	//}
}


// Called every frame
void AMonsterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Log�� ü��Ȯ��.
	//UE_LOG(LogTemp, Warning, TEXT("Health: %.1f, AttackPower: %.1f"),
	//	AttributeSet->GetHealth(), AttributeSet->GetAttackPower());

}


UAbilitySystemComponent* AMonsterCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AMonsterCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
    }
}

void AMonsterCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
    }
}

void AMonsterCharacter::Attack()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbilityByClass(UGA_MonsterAttack::StaticClass());
	}
}


void AMonsterCharacter::SetMonsterState(EMonsterState NewState)
{
	if (CurrentState == EMonsterState::Dead) return; // ������ ���� �� �ٲ�
	if (CurrentState == NewState) return;

	CurrentState = NewState;

	// ���� ���� �� �α�
	UE_LOG(LogTemp, Log, TEXT("[Monster] State changed to %d"), (uint8)NewState);

	// �ִϸ��̼�/AI ������ �ִٸ� ���⼭ ȣ��
	// ex) AnimInstance->SetState(NewState);
}


void AMonsterCharacter::ApplyInitStats(const FMonsterStatRow& Row, TSubclassOf<class UGameplayEffect> InitGE)
{
	if (!AbilitySystemComponent || !InitGE) return;

	FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(InitGE, 1.f, Ctx);
	if (!Spec.IsValid()) return;

	const FGameplayTag Tag_Health = FGameplayTag::RequestGameplayTag(FName("Data.Health"));
	const FGameplayTag Tag_Attack = FGameplayTag::RequestGameplayTag(FName("Data.AttackPower"));
	const FGameplayTag Tag_Move = FGameplayTag::RequestGameplayTag(FName("Data.MoveSpeed"));

	Spec.Data->SetSetByCallerMagnitude(Tag_Health, Row.MaxHealth);
	Spec.Data->SetSetByCallerMagnitude(Tag_Attack, Row.AttackPower);
	Spec.Data->SetSetByCallerMagnitude(Tag_Move, Row.MoveSpeed);

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

    if (GEngine && AttributeSet)
    {
        FString Msg = FString::Printf(TEXT("HP=%.1f  ATK=%.1f"),
            AttributeSet->GetHealth(),
            AttributeSet->GetAttackPower());
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Msg);
    }

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = Row.MoveSpeed;
	}
}