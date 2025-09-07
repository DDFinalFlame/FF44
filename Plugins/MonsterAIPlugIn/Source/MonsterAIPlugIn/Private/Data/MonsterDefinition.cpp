#include "Data/MonsterDefinition.h"
#include "Animation/AnimMontage.h"

static void CollectSectionsFromMontage(UAnimMontage* _montage, const FName _prefix, TArray<FName>& _out)
{
    _out.Reset();
    if (!_montage) return;

    const bool bUsePrefix = (_prefix != NAME_None);
    for (int32 i = 0; i < _montage->CompositeSections.Num(); ++i)
    {
        const FCompositeSection& Sec = _montage->CompositeSections[i];
        if (!bUsePrefix)
        {
            _out.Add(Sec.SectionName);
        }
        else
        {
            const FString P = _prefix.ToString();
            const FString S = Sec.SectionName.ToString();
            if (S.StartsWith(P))
            {
                _out.Add(Sec.SectionName);
            }
        }
    }
}

bool UMonsterDefinition::FindAttackByKey(FName _key, UAnimMontage*& _outMontage, FName& _outSection)
{
    _outMontage = nullptr;
    _outSection = NAME_None;

    if (_key.IsNone()) return false;

    for (auto& E : AttackList)
    {
        if (E.Key == _key)
        {
            if (!E.Montage.IsValid())
            {
                E.Montage.LoadSynchronous();
            }
            UAnimMontage* M = E.Montage.Get();
            if (!M) return false;

            // 섹션 후보 계산
            TArray<FName> Candidates = E.Sections;
            if (Candidates.Num() == 0)
            {
                CollectSectionsFromMontage(M, E.SectionPrefix, Candidates);
            }

            _outMontage = M;
            if (Candidates.Num() > 0)
            {
                const int32 R = FMath::RandRange(0, Candidates.Num() - 1);
                _outSection = Candidates[R];
            }
            return true;
        }
    }
    return false;
}

bool UMonsterDefinition::PickRandomAttack(UAnimMontage*& _outMontage, FName& _outSection)
{
    _outMontage = nullptr;
    _outSection = NAME_None;

    if (AttackList.Num() == 0) return false;

    // 가중치 합
    int32 Total = 0;
    for (auto& E : AttackList)
    {
        int32 W = E.Weight;
        if (W < 1) W = 1;
        Total += W;
    }

    int32 Pick = FMath::RandRange(1, Total);
    const FAttackMontageEntry* Chosen = nullptr;
    int32 Acc = 0;
    for (auto& E : AttackList)
    {
        int32 W = E.Weight;
        if (W < 1) W = 1;
        Acc += W;
        if (Pick <= Acc)
        {
            Chosen = &E;
            break;
        }
    }
    if (!Chosen) return false;

    if (!Chosen->Montage.IsValid())
    {
        const_cast<TSoftObjectPtr<UAnimMontage>&>(Chosen->Montage).LoadSynchronous();
    }
    UAnimMontage* M = Chosen->Montage.Get();
    if (!M) return false;

    // 섹션 후보
    TArray<FName> Candidates = Chosen->Sections;
    if (Candidates.Num() == 0)
    {
        CollectSectionsFromMontage(M, Chosen->SectionPrefix, Candidates);
    }

    _outMontage = M;
    if (Candidates.Num() > 0)
    {
        const int32 R = FMath::RandRange(0, Candidates.Num() - 1);
        _outSection = Candidates[R];
    }
    return true;
}