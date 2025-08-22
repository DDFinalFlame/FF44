// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

#include "MonsterAttributeSet.generated.h"

UCLASS()
class MONSTERAIPLUGIN_API UMonsterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UMonsterAttributeSet();

	// 체력, 최대 체력
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UMonsterAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UMonsterAttributeSet, MaxHealth)


	// 공격력 이동속도
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPower, Category = "Attributes")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UMonsterAttributeSet, AttackPower)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed, Category = "Attributes")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UMonsterAttributeSet, MoveSpeed)

	UPROPERTY(BlueprintReadOnly, Category = "Attr", ReplicatedUsing = OnRep_Defense)
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UMonsterAttributeSet, Defense)

	// OnRep 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 어트리뷰트 값이 “설정되기 직전” 정규화(클램프 등) */
	virtual void PreAttributeChange(const FGameplayAttribute& _attr, float& _newValue) override;

	/** GE가 적용된 “직후” 후처리(Health 0 감지 등) */
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& _data) override;

	/** OnRep */
	UFUNCTION() void OnRep_Health(const FGameplayAttributeData& _old);
	UFUNCTION() void OnRep_MaxHealth(const FGameplayAttributeData& _old);
	UFUNCTION() void OnRep_AttackPower(const FGameplayAttributeData& _old);
	UFUNCTION() void OnRep_MoveSpeed(const FGameplayAttributeData& _old);
	UFUNCTION() void OnRep_Defense(const FGameplayAttributeData& _old);

};
