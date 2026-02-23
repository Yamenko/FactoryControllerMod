#include "FGBuildableControlPanel.h"
#include "FGPowerConnectionComponent.h"
#include "FGBuildableFactory.h"
#include "FGBuildablePowerPole.h"
#include "Components/BoxComponent.h"
#include "FGCharacterPlayer.h"
#include "Engine/World.h"
#include "FGBuildableControlPanel.h"
#include "FGPlayerController.h"
#include "FGGameState.h"
#include "FGCharacterPlayer.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY(FactoryControllerMod);

// Временный класс для FactorySettingsData
class UFactorySettingsData : public UObject
{
public:
    void ApplyToFactory(AFGBuildableFactory* Factory)
    {
        // Заглушка
    }
};

AFGBuildableControlPanel::AFGBuildableControlPanel()
{
	// Входное соединение
    InputConnection = CreateDefaultSubobject<UFGPowerConnectionComponent>(TEXT("InputConnection"));
    InputConnection->SetupAttachment(RootComponent);  // <-- ВАЖНО: прикрепляем к Root

     // Выходное соединение
    OutputConnection = CreateDefaultSubobject<UFGPowerConnectionComponent>(TEXT("OutputConnection"));
    OutputConnection->SetupAttachment(RootComponent);  // <-- прикрепляем к Root

    // Интерактивная коробка
    //InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    //InteractionBox->SetupAttachment(RootComponent);  // <-- прикрепляем к Root

    // Если есть Mesh
    MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    MainMesh->SetupAttachment(RootComponent);

    // Box компонент для взаимодействия (вместо FGInteractComponent)
    //InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    //InteractionBox->SetupAttachment(RootComponent);
    //InteractionBox->SetBoxExtent(FVector(100, 100, 100));
    //InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    //InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    //InteractionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    bReplicates = true;
    bAlwaysRelevant = true;
}

void AFGBuildableControlPanel::OnUse_Implementation(AFGCharacterPlayer* byCharacter, const FUseState& state)
{
    // Сначала вызываем родительский метод
    Super::OnUse_Implementation(byCharacter, state);

    // Проверяем, что персонаж существует
    if (!byCharacter)
    {
        UE_LOG(FactoryControllerMod, Warning, TEXT("Control Panel: byCharacter is NULL"));
        return;
    }

    // Логируем вызов
    UE_LOG(FactoryControllerMod, Display, TEXT("========== CONTROL PANEL INTERACTION =========="));
    UE_LOG(FactoryControllerMod, Display, TEXT("OnUse called by: %s"), *byCharacter->GetName());

    // Получаем список заводов
    TArray<AFGBuildableFactory*> Factories = GetControlledFactories();

    // Выводим количество
    UE_LOG(FactoryControllerMod, Display, TEXT("Found %d controlled factories:"), Factories.Num());

    // Выводим на экран
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
        FString::Printf(TEXT("Control Panel: %d factories found"), Factories.Num()));

    // Выводим каждый завод
    for (AFGBuildableFactory* Factory : Factories)
    {
        if (Factory)
        {
            FString FactoryName = Factory->GetName();
            UE_LOG(FactoryControllerMod, Display, TEXT("  - %s"), *FactoryName);
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
                FString::Printf(TEXT("  • %s"), *FactoryName));
        }
    }

    UE_LOG(FactoryControllerMod, Display, TEXT("================================================"));

    // Простая визуальная обратная связь - мигание
    if (MainMesh)
    {
        // Временно отключаем видимость
        MainMesh->SetVisibility(false);

        // Возвращаем через 0.1 секунды
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, [this]() {
            if (MainMesh)
            {
                MainMesh->SetVisibility(true);
            }
            }, 0.1f, false);
    }
}

FText AFGBuildableControlPanel::GetLookAtDecription_Implementation(AFGCharacterPlayer* byCharacter, const FUseState& state) const
{
    // Текст, который появляется при наведении
    return FText::FromString(TEXT("Open Control Panel"));
}

void AFGBuildableControlPanel::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        UE_LOG(FactoryControllerMod, Log, TEXT("Control Panel initialized at %s"), *GetActorLocation().ToString());
    }
}

void AFGBuildableControlPanel::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (AFGCharacterPlayer* Character = Cast<AFGCharacterPlayer>(OtherActor))
    {
        OnInteract(Character);
    }
}

TArray<AFGBuildableFactory*> AFGBuildableControlPanel::GetControlledFactories()
{
    if (!HasAuthority() || !GetWorld())
    {
        return TArray<AFGBuildableFactory*>();
    }

    CachedControlledFactories.Empty();

    if (OutputConnection)
    {
        TSet<UFGPowerConnectionComponent*> VisitedConnections;
        FindAllFactoriesFromConnection(OutputConnection, CachedControlledFactories, VisitedConnections);
    }

    UE_LOG(FactoryControllerMod, Log, TEXT("Control Panel found %d factories under its control"),
        CachedControlledFactories.Num());

    return CachedControlledFactories;
}

void AFGBuildableControlPanel::FindAllFactoriesFromConnection(
    UFGPowerConnectionComponent* StartConnection,
    TArray<AFGBuildableFactory*>& OutFactories,
    TSet<UFGPowerConnectionComponent*>& VisitedConnections,
    int32 Depth)
{
    if (!StartConnection || VisitedConnections.Contains(StartConnection))
    {
        return;
    }

    if (Depth > 100)
    {
        UE_LOG(FactoryControllerMod, Warning, TEXT("Reached maximum recursion depth in power grid"));
        return;
    }

    VisitedConnections.Add(StartConnection);

    AActor* Owner = StartConnection->GetOwner();

    if (Owner)
    {
        if (AFGBuildableFactory* Factory = Cast<AFGBuildableFactory>(Owner))
        {
            if (Factory != Cast<AFGBuildableFactory>(this))
            {
                OutFactories.Add(Factory);
                UE_LOG(FactoryControllerMod, Verbose, TEXT("Depth %d: Found factory: %s"), Depth, *Factory->GetName());
            }
        }
        else if (AFGBuildablePowerPole* Pole = Cast<AFGBuildablePowerPole>(Owner))
        {
            UE_LOG(FactoryControllerMod, VeryVerbose, TEXT("Depth %d: Passing through pole: %s"), Depth, *Pole->GetName());
        }
    }

    TArray<UFGPowerConnectionComponent*> ConnectedConnections;
    if (Owner)
    {
        Owner->GetComponents<UFGPowerConnectionComponent>(ConnectedConnections);
    }

    for (UFGPowerConnectionComponent* Connected : ConnectedConnections)
    {
        if (Connected && !VisitedConnections.Contains(Connected))
        {
            bool bLeadsToInput = false;

            AActor* ConnectedOwner = Connected->GetOwner();
            if (ConnectedOwner == this && Connected == InputConnection)
            {
                bLeadsToInput = true;
            }

            if (!bLeadsToInput)
            {
                FindAllFactoriesFromConnection(Connected, OutFactories, VisitedConnections, Depth + 1);
            }
        }
    }
}

void AFGBuildableControlPanel::ApplySettingsToControlledFactories(UObject* SettingsObject)
{
    if (!SettingsObject || !HasAuthority())
    {
        return;
    }

    // Проверяем, что это нужный тип
    UFactorySettingsData* Settings = Cast<UFactorySettingsData>(SettingsObject);
    if (!Settings)
    {
        UE_LOG(FactoryControllerMod, Warning, TEXT("Invalid settings object"));
        return;
    }

    TArray<AFGBuildableFactory*> Controlled = GetControlledFactories();

    for (AFGBuildableFactory* Factory : Controlled)
    {
        Settings->ApplyToFactory(Factory);
    }

    UE_LOG(FactoryControllerMod, Log, TEXT("Applied settings to %d controlled factories"), Controlled.Num());
}

void AFGBuildableControlPanel::OnInteract(AFGCharacterPlayer* interactingCharacter)
{
    if (!interactingCharacter)
    {
        return;
    }

    int32 ControlledCount = GetControlledFactories().Num();

    UE_LOG(FactoryControllerMod, Warning, TEXT("=== CONTROL PANEL INFO ==="));
    UE_LOG(FactoryControllerMod, Warning, TEXT("Controlled factories: %d"), ControlledCount);

    // Здесь будем открывать UI
}