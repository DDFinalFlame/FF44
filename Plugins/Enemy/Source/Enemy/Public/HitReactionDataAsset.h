// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyDefine.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "HitReactionDataAsset.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FEnemyHitReactRow : public FTableRowBase
{
	GENERATED_BODY()

	// Enemy Ÿ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEnemyType EnemyType;

	// ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EHitDirection Direction;

	// �ش� ��Ȳ���� ����� ��Ÿ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* Montage;
};

USTRUCT(BlueprintType)
struct FEnemyDeathRow : public FTableRowBase
{
	GENERATED_BODY()

	// Enemy Ÿ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEnemyType EnemyType;

	// �ش� ��Ȳ���� ����� ��Ÿ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* Montage;
};

USTRUCT(BlueprintType)
struct FEnemyAttackRow : public FTableRowBase
{
	GENERATED_BODY()

	// Enemy Ÿ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEnemyType EnemyType;

	// Attack Ÿ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag GameplayTag;

	// �ش� ��Ȳ���� ����� ��Ÿ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* Montage;
};

UCLASS()
class ENEMY_API UHitReactionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* HitReactTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* DeathTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* AttackTable;

	// Hit Montage Helper �Լ�
	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetHitMontage(EEnemyType EnemyType, EHitDirection Direction) const
	{
		if (!HitReactTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("HitReactTable is null"));
			return nullptr;
		}

		static const FString Context(TEXT("HitReactLookup"));

		// DataTable�� ��ϵ� ��� Row �̸��� ��ȸ
		for (const FName& RowName : HitReactTable->GetRowNames())
		{
			const FEnemyHitReactRow* Row = HitReactTable->FindRow<FEnemyHitReactRow>(RowName, Context);
			if (!Row) continue;

			// ���� �˻�
			if (Row->EnemyType == EnemyType && Row->Direction == Direction)
			{
				return Row->Montage;
			}
		}
		return nullptr;
	}

	// Death Montage Helper �Լ�
	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetDieMontage(EEnemyType EnemyType) const
	{
		if (!DeathTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("MontageTable is null"));
			return nullptr;
		}

		static const FString Context(TEXT("DieReactLookup"));

		// DataTable�� ��ϵ� ��� Row �̸��� ��ȸ
		for (const FName& RowName : DeathTable->GetRowNames())
		{
			const FEnemyDeathRow* Row = DeathTable->FindRow<FEnemyDeathRow>(RowName, Context);
			if (!Row) continue;

			// ���� �˻�
			if (Row->EnemyType == EnemyType)
			{
				return Row->Montage;
			}
		}
		return nullptr;
	}

	// Attack Montage Helper �Լ�
	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetAttackMontage(EEnemyType EnemyType, FGameplayTagContainer AttackTag) const
	{
		if (!AttackTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("Table is null"));
			return nullptr;
		}

		static const FString Context(TEXT("AttackLookup"));

		// DataTable�� ��ϵ� ��� Row �̸��� ��ȸ
		for (const FName& RowName : AttackTable->GetRowNames())
		{
			const FEnemyAttackRow* Row = AttackTable->FindRow<FEnemyAttackRow>(RowName, Context);
			if (!Row) continue;

			// ���� �˻�
			if (Row->EnemyType == EnemyType && AttackTag.HasTagExact(Row->GameplayTag))
			{
				return Row->Montage;
			}
		}
		return nullptr;
	}
};
