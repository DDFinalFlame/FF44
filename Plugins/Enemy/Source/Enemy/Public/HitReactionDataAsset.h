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

	// Enemy 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEnemyType EnemyType;

	// 맞은 방향
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EHitDirection Direction;

	// 해당 상황에서 재생할 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* Montage;
};

USTRUCT(BlueprintType)
struct FEnemyDeathRow : public FTableRowBase
{
	GENERATED_BODY()

	// Enemy 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEnemyType EnemyType;

	// 해당 상황에서 재생할 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* Montage;
};

USTRUCT(BlueprintType)
struct FEnemyAttackRow : public FTableRowBase
{
	GENERATED_BODY()

	// Enemy 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEnemyType EnemyType;

	// Attack 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag GameplayTag;

	// 해당 상황에서 재생할 몽타주
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

	// Hit Montage Helper 함수
	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetHitMontage(EEnemyType EnemyType, EHitDirection Direction) const
	{
		if (!HitReactTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("HitReactTable is null"));
			return nullptr;
		}

		static const FString Context(TEXT("HitReactLookup"));

		// DataTable에 등록된 모든 Row 이름들 순회
		for (const FName& RowName : HitReactTable->GetRowNames())
		{
			const FEnemyHitReactRow* Row = HitReactTable->FindRow<FEnemyHitReactRow>(RowName, Context);
			if (!Row) continue;

			// 조건 검사
			if (Row->EnemyType == EnemyType && Row->Direction == Direction)
			{
				return Row->Montage;
			}
		}
		return nullptr;
	}

	// Death Montage Helper 함수
	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetDieMontage(EEnemyType EnemyType) const
	{
		if (!DeathTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("MontageTable is null"));
			return nullptr;
		}

		static const FString Context(TEXT("DieReactLookup"));

		// DataTable에 등록된 모든 Row 이름들 순회
		for (const FName& RowName : DeathTable->GetRowNames())
		{
			const FEnemyDeathRow* Row = DeathTable->FindRow<FEnemyDeathRow>(RowName, Context);
			if (!Row) continue;

			// 조건 검사
			if (Row->EnemyType == EnemyType)
			{
				return Row->Montage;
			}
		}
		return nullptr;
	}

	// Attack Montage Helper 함수
	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetAttackMontage(EEnemyType EnemyType, FGameplayTagContainer AttackTag) const
	{
		if (!AttackTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("Table is null"));
			return nullptr;
		}

		static const FString Context(TEXT("AttackLookup"));

		// DataTable에 등록된 모든 Row 이름들 순회
		for (const FName& RowName : AttackTable->GetRowNames())
		{
			const FEnemyAttackRow* Row = AttackTable->FindRow<FEnemyAttackRow>(RowName, Context);
			if (!Row) continue;

			// 조건 검사
			if (Row->EnemyType == EnemyType && AttackTag.HasTagExact(Row->GameplayTag))
			{
				return Row->Montage;
			}
		}
		return nullptr;
	}
};
