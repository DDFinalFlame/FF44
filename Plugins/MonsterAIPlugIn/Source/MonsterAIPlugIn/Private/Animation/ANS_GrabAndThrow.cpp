// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/ANS_GrabAndThrow.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

ACharacter* UANS_GrabAndThrow::ResolveVictimFromBlackboard(ACharacter* Boss) const
{
    if (!Boss) return nullptr;

    AAIController* AI = Cast<AAIController>(Boss->GetController());
    if (!AI) return nullptr;

    UBlackboardComponent* BB = AI->GetBlackboardComponent();
    if (!BB || BB_TargetActorKey.IsNone()) return nullptr;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(BB_TargetActorKey));
    ACharacter* Victim = Cast<ACharacter>(Target);

    if (Victim && VictimClassFilter)
    {
        if (!Victim->IsA(VictimClassFilter)) return nullptr; 
    }
    return Victim;
}

void UANS_GrabAndThrow::AlignVictimRootToBossSocket(ACharacter* Boss, ACharacter* Victim) const
{
    if (!Boss || !Victim) return;

    USkeletalMeshComponent* BossMesh = Boss->GetMesh();
    USceneComponent* VictimRoot = Victim->GetRootComponent();
    USkeletalMeshComponent* VictimMesh = Victim->GetMesh();
    if (!BossMesh || !VictimRoot || !VictimMesh) return;

    // 1) ���� ���� ���� ���� Ʈ������
    const FTransform BossSocketWS = BossMesh->GetSocketTransform(BossSocketName, RTS_World);

    // 2) ����� ���� ���� Ʈ������
    //    - ������ �������� ������ ����� ��Ʈ(ĸ��) �������� ����
    FTransform VictimSocketWS;
    if (VictimAttachSocketName.IsNone())
    {
        // ����� ��Ʈ ��ü�� ���Ϸ� ���� (��ƮWS == ����WS)
        VictimSocketWS = VictimRoot->GetComponentTransform();
    }
    else
    {
        // ����� '�޽���' ���� ���� Ʈ������
        VictimSocketWS = VictimMesh->GetSocketTransform(VictimAttachSocketName, RTS_World);
    }

    // 3) ����� ��Ʈ ���� ����
    const FTransform VictimRootWS = VictimRoot->GetComponentTransform();

    // 4) VictimSocketWS = T * VictimRootWS  ==>  VictimRootWS' = T^{-1} * BossSocketWS
    const FTransform T = VictimSocketWS.GetRelativeTransform(VictimRootWS);
    const FTransform NewVictimRootWS = T.Inverse() * BossSocketWS;

    // 5) ����� ��Ʈ�� ���� ���ϰ� ���ĵǵ��� �̵�(���� �ڷ���Ʈ)
    VictimRoot->SetWorldTransform(NewVictimRootWS, false, nullptr, ETeleportType::TeleportPhysics);
}



void UANS_GrabAndThrow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;
    ACharacter* Boss = Cast<ACharacter>(MeshComp->GetOwner());
    if (!Boss || !Boss->HasAuthority()) return;

    ACharacter* Victim = ResolveVictimFromBlackboard(Boss); // �� �÷��̾� ������
    if (!Victim) return;

    if (VictimStartEventTag.IsValid())
    {
        FGameplayEventData Data;
        Data.EventTag = VictimStartEventTag;
        Data.Instigator = Boss;
        Data.Target = Victim;
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
            Victim, VictimStartEventTag, Data);
    }

    USkeletalMeshComponent* BossMesh = Boss->GetMesh();
    USceneComponent* VictimRoot = Victim->GetRootComponent();
    if (!BossMesh || !VictimRoot) return;

    if (UCharacterMovementComponent* Move = Victim->GetCharacterMovement())
    {
        Move->StopMovementImmediately();
    }

    //AlignVictimRootToBossSocket(Boss, Victim);

    //const FAttachmentTransformRules Rules = bSnapToBossSocket
    //    ? FAttachmentTransformRules::KeepWorldTransform // �̹� ���������� ���� ����
    //    : FAttachmentTransformRules::KeepWorldTransform;

    //VictimRoot->AttachToComponent(BossMesh, Rules, BossSocketName);
}

void UANS_GrabAndThrow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;
    ACharacter* Boss = Cast<ACharacter>(MeshComp->GetOwner());
    if (!Boss || !Boss->HasAuthority()) return; 

    ACharacter* Victim = ResolveVictimFromBlackboard(Boss); 
    if (!Victim) return;

   // Victim->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    if (VictimEndEventTag.IsValid())
    {
        FGameplayEventData Data;
        Data.EventTag = VictimEndEventTag;
        Data.Instigator = Boss;
        Data.Target = Victim;
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
            Victim, VictimEndEventTag, Data);
    }

    const FVector Fwd = Boss->GetActorForwardVector();
    const FVector Launch = Fwd * LaunchSpeed + FVector(0, 0, LaunchUpward);
   // Victim->LaunchCharacter(Launch, true, true);
}