#pragma once

#include "CoreMinimal.h"
#include "FGBuildable.h"
#include "FGPowerConnectionComponent.h"
#include "FGCharacterPlayer.h"
#include "FGBuildableFactory.h"
#include "FGBuildableControlPanel.generated.h"

// Создаем категорию для логирования
DECLARE_LOG_CATEGORY_EXTERN(FactoryControllerMod, Log, All);

UCLASS()
class FACTORYCONTROLLERMOD_API AFGBuildableControlPanel : public AFGBuildable
{
    GENERATED_BODY()
public:
    AFGBuildableControlPanel();

    // Меш панели
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
    UStaticMeshComponent* MainMesh;

    // Компоненты подключения
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Power")
    UFGPowerConnectionComponent* InputConnection;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Power")
    UFGPowerConnectionComponent* OutputConnection;

    // Переопределяем методы взаимодействия
    virtual void OnUse_Implementation(class AFGCharacterPlayer* byCharacter, const FUseState& state) override;
    virtual FText GetLookAtDecription_Implementation(class AFGCharacterPlayer* byCharacter, const FUseState& state) const override;

    // Получить все заводы под управлением
    UFUNCTION(BlueprintCallable)
    TArray<AFGBuildableFactory*> GetControlledFactories();

    // Применить настройки ко всем заводам
    UFUNCTION(BlueprintCallable)
    void ApplySettingsToControlledFactories(UObject* Settings);

    // Вспомогательная функция для рекурсивного обхода
    void RecursiveFindFactories(UFGCircuitConnectionComponent* StartConnection,
        TArray<AFGBuildableFactory*>& OutFactories,
        TSet<AActor*>& Visited);

    // Функция для отладки
    void DebugPrintConnectionStats() const ;
};