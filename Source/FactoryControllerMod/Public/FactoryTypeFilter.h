#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FactoryTypeFilter.generated.h"

// Forward declarations
class AActor;

/**
 * Класс для фильтрации типов зданий в Satisfactory
 * Определяет, какие здания можно подключать к Input/Output панели управления
 */
UCLASS(BlueprintType, Blueprintable)
class FACTORYCONTROLLERMOD_API UFactoryTypeFilter : public UObject
{
    GENERATED_BODY()

public:
    // Можно ли подключать этот объект к INPUT панели?
    UFUNCTION(BlueprintPure, Category = "Factory Control|Filter")
    static bool CanConnectToInput(AActor* Actor);

    // Можно ли подключать этот объект к OUTPUT панели?
    UFUNCTION(BlueprintPure, Category = "Factory Control|Filter")
    static bool CanConnectToOutput(AActor* Actor);

    // Является ли объект генератором (источником питания)?
    UFUNCTION(BlueprintPure, Category = "Factory Control|Filter")
    static bool IsPowerGenerator(AActor* Actor);

    // Является ли объект заводом (которым можно управлять)?
    UFUNCTION(BlueprintPure, Category = "Factory Control|Filter")
    static bool IsControllableFactory(AActor* Actor);

    // Является ли объект распределителем (столбом)?
    UFUNCTION(BlueprintPure, Category = "Factory Control|Filter")
    static bool IsPowerDistributor(AActor* Actor);

    // Получить отображаемое имя типа здания для отладки
    UFUNCTION(BlueprintPure, Category = "Factory Control|Filter")
    static FString GetActorTypeName(AActor* Actor);
};