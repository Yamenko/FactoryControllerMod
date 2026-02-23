#include "FactoryTypeFilter.h"

// Классы для проверки
#include "FGBuildableGenerator.h"
#include "FGBuildableFactory.h"
#include "FGBuildablePowerPole.h"

// Реализация статических методов
bool UFactoryTypeFilter::CanConnectToInput(AActor* Actor)
{
    // К INPUT можно подключать:
    // - Генераторы (источники питания)
    // - Столбы (для построения цепочек)
    return IsPowerGenerator(Actor) || IsPowerDistributor(Actor);
}

bool UFactoryTypeFilter::CanConnectToOutput(AActor* Actor)
{
    // К OUTPUT можно подключать:
    // - Заводы (которые будем контролировать)
    // - Столбы (для распределения на много заводов)
    return IsControllableFactory(Actor) || IsPowerDistributor(Actor);
}

bool UFactoryTypeFilter::IsPowerGenerator(AActor* Actor)
{
    // Проверяем, является ли актер генератором
    // Все генераторы в игре наследуются от AFGBuildableGenerator
    if (!Actor) return false;

    return Actor->IsA(AFGBuildableGenerator::StaticClass());
}

bool UFactoryTypeFilter::IsControllableFactory(AActor* Actor)
{
    // Проверяем, является ли актер заводом, которым можно управлять
    // Все производственные здания наследуются от AFGBuildableFactory
    if (!Actor) return false;

    return Actor->IsA(AFGBuildableFactory::StaticClass());
}

bool UFactoryTypeFilter::IsPowerDistributor(AActor* Actor)
{
    // Проверяем, является ли актер столбом электропередач
    if (!Actor) return false;

    return Actor->IsA(AFGBuildablePowerPole::StaticClass());
}

FString UFactoryTypeFilter::GetActorTypeName(AActor* Actor)
{
    // Вспомогательная функция для отладки
    if (!Actor) return TEXT("Null");

    if (IsPowerGenerator(Actor)) return TEXT("Generator");
    if (IsControllableFactory(Actor)) return TEXT("Factory");
    if (IsPowerDistributor(Actor)) return TEXT("PowerPole");

    return TEXT("Unknown");
}