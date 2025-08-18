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

UCLASS()
class FF44_API UBasePlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UBasePlayerAttributeSet();

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_Health)
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UBasePlayerAttributeSet, Health)

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_Stamina)
    FGameplayAttributeData Stamina;
    ATTRIBUTE_ACCESSORS(UBasePlayerAttributeSet, Stamina)

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_AttackPower)
    FGameplayAttributeData AttackPower;
    ATTRIBUTE_ACCESSORS(UBasePlayerAttributeSet, AttackPower)

    UFUNCTION()
    void OnRep_Health(const FGameplayAttributeData& OldHealth);

    UFUNCTION()
    void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

    UFUNCTION()
    void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
