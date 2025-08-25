// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyDefine.h"
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

UCLASS()
class ENEMY_API UHitReactionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UDataTable* HitReactTable;

    // Helper �Լ�
    UFUNCTION(BlueprintCallable)
    UAnimMontage* GetHitMontage(EEnemyType EnemyType, EHitDirection Direction) const
    {
        //if (!HitReactTable) return nullptr;

        //// ��� Row ��������
        //static const FString Context(TEXT("HitReactLookup"));
        //TArray<FEnemyHitReactRow*> Rows;
        //HitReactTable->GetAllRows(Context, Rows);

        //for (FEnemyHitReactRow* Row : Rows)
        //{
        //    if (Row->EnemyType == EnemyType && Row->Direction == Direction)
        //    {
        //        return Row->Montage;
        //    }
        //}

        //// ������ None Ȯ���ؼ� None���� ����
        //for (FEnemyHitReactRow* Row : Rows)
        //{
        //    if (Row->EnemyType == EnemyType && Row->Direction == EHitDirection::None)
        //    {
        //        return Row->Montage;
        //    }
        //}

        //return nullptr;

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

            // Debug ��� (���� Row �� Ȯ��)
            UE_LOG(LogTemp, Log, TEXT("RowName: %s, EnemyType: %d, Direction: %d"),
                *RowName.ToString(),
                static_cast<int32>(Row->EnemyType),
                static_cast<int32>(Row->Direction));

            // ���� �˻�
            if (Row->EnemyType == EnemyType && Row->Direction == Direction)
            {
                UE_LOG(LogTemp, Log, TEXT("Match Found: EnemyType=%d, Direction=%d, Montage=%s"),
                    static_cast<int32>(EnemyType),
                    static_cast<int32>(Direction),
                    Row->Montage ? *Row->Montage->GetName() : TEXT("NULL"));

                return Row->Montage;
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("No matching HitReact found for EnemyType=%d, Direction=%d"),
            static_cast<int32>(EnemyType),
            static_cast<int32>(Direction));

        return nullptr;
    }
};
