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

UCLASS()
class ENEMY_API UHitReactionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UDataTable* HitReactTable;

    // Helper 함수
    UFUNCTION(BlueprintCallable)
    UAnimMontage* GetHitMontage(EEnemyType EnemyType, EHitDirection Direction) const
    {
        //if (!HitReactTable) return nullptr;

        //// 모든 Row 가져오기
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

        //// 없으면 None 확인해서 None으로 리턴
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

        // DataTable에 등록된 모든 Row 이름들 순회
        for (const FName& RowName : HitReactTable->GetRowNames())
        {
            const FEnemyHitReactRow* Row = HitReactTable->FindRow<FEnemyHitReactRow>(RowName, Context);
            if (!Row) continue;

            // Debug 출력 (현재 Row 값 확인)
            UE_LOG(LogTemp, Log, TEXT("RowName: %s, EnemyType: %d, Direction: %d"),
                *RowName.ToString(),
                static_cast<int32>(Row->EnemyType),
                static_cast<int32>(Row->Direction));

            // 조건 검사
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
