#include "ProceduralEditorModule.h"
#include "GenerationWindow.h"

#include "Modules/ModuleManager.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FProceduralEditorModule"

class FProceduralEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        UGenerationWindow::RegisterTabSpawner();

        UToolMenus::RegisterStartupCallback(
            FSimpleMulticastDelegate::FDelegate::CreateRaw(
                this,
                &FProceduralEditorModule::RegisterMenus
            )
        );
    }

    virtual void ShutdownModule() override
    {
        UGenerationWindow::UnregisterTabSpawner();
        UToolMenus::UnRegisterStartupCallback(this);
    }

private:
    void RegisterMenus()
    {
        UToolMenu* Menu =
            UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");

        FToolMenuSection& Section =
            Menu->AddSection("Procedural", LOCTEXT("Procedural", "Procedural"));

        FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
            "OpenProceduralGenerationTab",
            LOCTEXT("OpenProceduralGenerationTab", "Procedural Generation"),
            LOCTEXT("OpenProceduralGenerationTab_Tooltip", "Open Procedural Generation Tab"),
            FSlateIcon(),
            FUIAction(
                FExecuteAction::CreateLambda([]()
                    {
                        FGlobalTabmanager::Get()->TryInvokeTab(UGenerationWindow::TabName);
                    })
            )
        );

        Section.AddEntry(Entry);
    }
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FProceduralEditorModule, ProceduralEditorModule)