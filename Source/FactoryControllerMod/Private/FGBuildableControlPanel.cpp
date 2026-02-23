#include "FGBuildableControlPanel.h"
#include "FGPlayerController.h"
#include "FGGameState.h"
#include "FGPowerCircuit.h"
#include "FGBuildableWire.h"
#include "FGPowerConnectionComponent.h"
#include "FGTrainPlatformConnection.h"

// Определяем категорию логирования
DEFINE_LOG_CATEGORY(FactoryControllerMod);

AFGBuildableControlPanel::AFGBuildableControlPanel()
{
    // Создаем Mesh компонент
    MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMesh"));
    MainMesh->SetupAttachment(RootComponent);

    //// Настраиваем коллизию для взаимодействия
    //MainMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    //MainMesh->SetCollisionResponseToAllChannels(ECR_Block);
    //MainMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    //MainMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);

    // Создаем Input Connection (питание)
    InputConnection = CreateDefaultSubobject<UFGPowerConnectionComponent>(TEXT("InputConnection"));
    InputConnection->SetupAttachment(RootComponent);

    // Создаем Output Connection (питание)
    OutputConnection = CreateDefaultSubobject<UFGPowerConnectionComponent>(TEXT("OutputConnection"));
    OutputConnection->SetupAttachment(RootComponent);

    // Настройки для корректной работы
    bReplicates = true;
}

void AFGBuildableControlPanel::OnUse_Implementation(AFGCharacterPlayer* byCharacter, const FUseState& state)
{
    Super::OnUse_Implementation(byCharacter, state);

    if (!byCharacter) return;

    UE_LOG(FactoryControllerMod, Display, TEXT("========== CONTROL PANEL INTERACTION =========="));
    UE_LOG(FactoryControllerMod, Display, TEXT("OnUse called by: %s"), *byCharacter->GetName());

    // Получаем список заводов
    TArray<AFGBuildableFactory*> Factories = GetControlledFactories();

    if (Factories.Num() == 0)
    {
        UE_LOG(FactoryControllerMod, Display, TEXT("Control Panel found 0 factories under its control"));
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
            TEXT("No factories connected!"));
    }
    else
    {
        UE_LOG(FactoryControllerMod, Display, TEXT("Control Panel found %d factories under its control"), Factories.Num());
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
            FString::Printf(TEXT("Found %d factories!"), Factories.Num()));

        // Выводим каждый завод
        for (int32 i = 0; i < Factories.Num(); i++)
        {
            if (Factories[i])
            {
                FString FactoryName = Factories[i]->GetName();
                UE_LOG(FactoryControllerMod, Display, TEXT("  %d. %s"), i + 1, *FactoryName);
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
                    FString::Printf(TEXT("  %d. %s"), i + 1, *FactoryName));
            }
        }
    }

    UE_LOG(FactoryControllerMod, Display, TEXT("================================================"));

    // Визуальная обратная связь - мигание
    if (MainMesh)
    {
        MainMesh->SetVisibility(false);
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, [this]() {
            if (MainMesh) MainMesh->SetVisibility(true);
            }, 0.1f, false);
    }
}

FText AFGBuildableControlPanel::GetLookAtDecription_Implementation(AFGCharacterPlayer* byCharacter, const FUseState& state) const
{
    return FText::FromString(TEXT("Open Factory Control Panel"));
}

void AFGBuildableControlPanel::RecursiveFindFactories(UFGCircuitConnectionComponent* StartConnection,
    TArray<AFGBuildableFactory*>& OutFactories,
    TSet<AActor*>& Visited)
{
    if (!StartConnection) return;

    AActor* Owner = StartConnection->GetOwner();
    if (!Owner || Visited.Contains(Owner)) return;

    Visited.Add(Owner);

    UE_LOG(FactoryControllerMod, VeryVerbose, TEXT("    Visiting: %s"), *Owner->GetName());

    // Проверяем, завод ли это
    AFGBuildableFactory* Factory = Cast<AFGBuildableFactory>(Owner);
    if (Factory)
    {
        UE_LOG(FactoryControllerMod, VeryVerbose, TEXT("      Found factory: %s"), *Factory->GetName());
        OutFactories.Add(Factory);
    }

    // Получаем все проводные подключения через GetConnections
    TArray<UFGCircuitConnectionComponent*> Connections;
    StartConnection->GetConnections(Connections);  // Правильный метод!

    UE_LOG(FactoryControllerMod, VeryVerbose, TEXT("      Found %d wired connections"), Connections.Num());

    for (UFGCircuitConnectionComponent* Conn : Connections)
    {
        if (Conn && Conn->GetOwner() != Owner)
        {
            RecursiveFindFactories(Conn, OutFactories, Visited);
        }
    }

    // Также проверяем скрытые подключения (через стены/столбы)
    TArray<UFGCircuitConnectionComponent*> HiddenConnections;
    StartConnection->GetHiddenConnections(HiddenConnections);  // Правильный метод!

    UE_LOG(FactoryControllerMod, VeryVerbose, TEXT("      Found %d hidden connections"), HiddenConnections.Num());

    for (UFGCircuitConnectionComponent* Conn : HiddenConnections)
    {
        if (Conn && Conn->GetOwner() != Owner)
        {
            RecursiveFindFactories(Conn, OutFactories, Visited);
        }
    }
}

TArray<AFGBuildableFactory*> AFGBuildableControlPanel::GetControlledFactories()
{
    TArray<AFGBuildableFactory*> Factories;
    TSet<AActor*> Visited;

    UE_LOG(FactoryControllerMod, Display, TEXT("GetControlledFactories called"));

    if (!OutputConnection)
    {
        UE_LOG(FactoryControllerMod, Error, TEXT("OutputConnection is NULL!"));
        return Factories;
    }

    // Проверяем прямые подключения через GetConnections
    TArray<UFGCircuitConnectionComponent*> DirectConnections;
    OutputConnection->GetConnections(DirectConnections);

    UE_LOG(FactoryControllerMod, Display, TEXT("Direct connections from Output: %d"), DirectConnections.Num());

    // Логируем информацию о OutputConnection
    UE_LOG(FactoryControllerMod, Display, TEXT("OutputConnection - Max connections: %d"), OutputConnection->GetMaxNumConnections());
    UE_LOG(FactoryControllerMod, Display, TEXT("OutputConnection - Num connections: %d"), OutputConnection->GetNumConnections());
    UE_LOG(FactoryControllerMod, Display, TEXT("OutputConnection - Circuit ID: %d"), OutputConnection->GetCircuitID());

    // Добавляем себя в посещенные, чтобы не зациклиться
    Visited.Add(this);

    // Запускаем рекурсивный обход от каждого прямого подключения
    for (UFGCircuitConnectionComponent* Conn : DirectConnections)
    {
        if (Conn)
        {
            AActor* ConnOwner = Conn->GetOwner();
            UE_LOG(FactoryControllerMod, Display, TEXT("  Direct connection to: %s"),
                ConnOwner ? *ConnOwner->GetName() : TEXT("NULL"));
            RecursiveFindFactories(Conn, Factories, Visited);
        }
    }

    // Также проверяем скрытые подключения от Output
    TArray<UFGCircuitConnectionComponent*> HiddenConnections;
    OutputConnection->GetHiddenConnections(HiddenConnections);

    UE_LOG(FactoryControllerMod, Display, TEXT("Hidden connections from Output: %d"), HiddenConnections.Num());

    for (UFGCircuitConnectionComponent* Conn : HiddenConnections)
    {
        if (Conn)
        {
            AActor* ConnOwner = Conn->GetOwner();
            UE_LOG(FactoryControllerMod, Display, TEXT("  Hidden connection to: %s"),
                ConnOwner ? *ConnOwner->GetName() : TEXT("NULL"));
            RecursiveFindFactories(Conn, Factories, Visited);
        }
    }

    // Убираем дубликаты
    TSet<AFGBuildableFactory*> UniqueFactories;
    for (AFGBuildableFactory* Factory : Factories)
    {
        if (Factory)
        {
            UniqueFactories.Add(Factory);
        }
    }

    Factories = UniqueFactories.Array();

    UE_LOG(FactoryControllerMod, Display, TEXT("Total unique factories found: %d"), Factories.Num());

    return Factories;
}

void AFGBuildableControlPanel::ApplySettingsToControlledFactories(UObject* Settings)
{
    // TODO: Implement settings application
    UE_LOG(FactoryControllerMod, Display, TEXT("ApplySettingsToControlledFactories called"));
}