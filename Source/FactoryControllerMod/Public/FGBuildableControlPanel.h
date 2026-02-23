#pragma once

#include "CoreMinimal.h"
#include "FGBuildable.h"
#include "FGBuildableControlPanel.generated.h"

// Просто объявляем классы, не включая заголовки
class UFGPowerConnectionComponent;
class UFGInteractWidget;
class AFGCharacterPlayer;
class AFGBuildableFactory;
class UFactorySettingsData;

DECLARE_LOG_CATEGORY_EXTERN(FactoryControllerMod, Verbose, All);

UCLASS()
class FACTORYCONTROLLERMOD_API AFGBuildableControlPanel : public AFGBuildable
{
    GENERATED_BODY()

public:
    AFGBuildableControlPanel();

    // Основной меш панели управления
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
    UStaticMeshComponent* MainMesh;

    // Два соединения: вход и выход
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UFGPowerConnectionComponent* InputConnection;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UFGPowerConnectionComponent* OutputConnection;

    // Для взаимодействия используем стандартный механизм Unreal
    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    //class UBoxComponent* InteractionBox;

    // Переопределяем методы взаимодействия из AFGBuildable
    virtual void OnUse_Implementation(class AFGCharacterPlayer* byCharacter, const FUseState& state) override;
    virtual FText GetLookAtDecription_Implementation(class AFGCharacterPlayer* byCharacter, const FUseState& state) const override;


    // Получить все заводы, которые находятся "после" этой панели
    UFUNCTION(BlueprintCallable, Category = "Control Panel")
    TArray<AFGBuildableFactory*> GetControlledFactories();

    // Применить настройки ко всем управляемым заводам
    UFUNCTION(BlueprintCallable, Category = "Control Panel")
    void ApplySettingsToControlledFactories(UObject* Settings);

    // Функция взаимодействия (открыть UI)
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnInteract(AFGCharacterPlayer* interactingCharacter);

protected:
    virtual void BeginPlay() override;
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

private:
    // Рекурсивный поиск всех заводов от указанной точки соединения
    void FindAllFactoriesFromConnection(
        UFGPowerConnectionComponent* StartConnection,
        TArray<AFGBuildableFactory*>& OutFactories,
        TSet<UFGPowerConnectionComponent*>& VisitedConnections,
        int32 Depth = 0);

    UPROPERTY()
    TArray<AFGBuildableFactory*> CachedControlledFactories;
};