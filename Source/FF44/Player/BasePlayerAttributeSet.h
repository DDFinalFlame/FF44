#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BasePlayerAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UENUM(BlueprintType)
enum class EAttributeType : uint8
{
    Health		UMETA(DisplayName = "Health"),
	Stamina	    UMETA(DisplayName = "Stamina"),
	AttackPower UMETA(DisplayName = "AttackPower"),
	DefencePoint UMETA(DisplayName = "DefencePoint"),
};

UCLASS()
class FF44_API UBasePlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UBasePlayerAttributeSet();

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_CurrentHP)
    FGameplayAttributeData CurrentHP;
    ATTRIBUTE_ACCESSORS(UBasePlayerAttributeSet, CurrentHP)

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_MaxHP)
    FGameplayAttributeData MaxHP;
    ATTRIBUTE_ACCESSORS(UBasePlayerAttributeSet, MaxHP)

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_CurrentStamina)
    FGameplayAttributeData CurrentStamina;
    ATTRIBUTE_ACCESSORS(UBasePlayerAttributeSet, CurrentStamina)

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_MaxStamina)
    FGameplayAttributeData MaxStamina;
    ATTRIBUTE_ACCESSORS(UBasePlayerAttributeSet, MaxStamina)

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_RegenRateStamina)
    FGameplayAttributeData RegenRateStamina;
    ATTRIBUTE_ACCESSORS(UBasePlayerAttributeSet, RegenRateStamina)

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_AttackPower)
    FGameplayAttributeData AttackPower;
    ATTRIBUTE_ACCESSORS(UBasePlayerAttributeSet, AttackPower)

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_DefencePoint)
    FGameplayAttributeData DefencePoint;
    ATTRIBUTE_ACCESSORS(UBasePlayerAttributeSet, DefencePoint)

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_WalkSpeed)
    FGameplayAttributeData WalkSpeed;
    ATTRIBUTE_ACCESSORS(UBasePlayerAttributeSet, WalkSpeed)

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_RunSpeed)
    FGameplayAttributeData RunSpeed;
    ATTRIBUTE_ACCESSORS(UBasePlayerAttributeSet, RunSpeed)

    UFUNCTION()
    void OnRep_CurrentHP(const FGameplayAttributeData& _OldValue);

    UFUNCTION()
    void OnRep_MaxHP(const FGameplayAttributeData& _OldValue);

    UFUNCTION()
    void OnRep_CurrentStamina(const FGameplayAttributeData& _OldValue);

    UFUNCTION()
    void OnRep_MaxStamina(const FGameplayAttributeData& _OldValue);

    UFUNCTION()
    void OnRep_RegenRateStamina(const FGameplayAttributeData& _OldValue);

    UFUNCTION()
    void OnRep_AttackPower(const FGameplayAttributeData& _OldValue);

    UFUNCTION()
	void OnRep_DefencePoint(const FGameplayAttributeData& _OldValue);

    UFUNCTION()
    void OnRep_WalkSpeed(const FGameplayAttributeData& _OldValue);

    UFUNCTION()
    void OnRep_RunSpeed(const FGameplayAttributeData& _OldValue);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& _Data) override;
};
