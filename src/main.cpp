#include "main.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"
#include "UnityEngine/GameObject.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "HMUI/Touchable.hpp"
#include "questui/shared/QuestUI.hpp"
#include "config-utils/shared/config-utils.hpp"
#include "ModConfig.hpp"
#include "GlobalNamespace/NoteJump.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "System/Action.hpp"

using namespace QuestUI;
using namespace UnityEngine;
using namespace GlobalNamespace;

float i = 0.0f;

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    config.Load();
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

MAKE_HOOK_MATCH(NoteControllerInit, &NoteController::Init, void, NoteController* self, NoteData* noteData, float worldRotation, Vector3 moveStartPos, Vector3 moveEndPos, Vector3 jumpEndPos, float moveDuration, float jumpDuration, float jumpGravity, float endRotation, float uniformScale, bool rotatesTowardsPlayer, bool useRandomRotation) {
    if(getModConfig().Active.GetValue()){
        moveEndPos = {moveEndPos.x, 0, moveEndPos.z};
        moveStartPos = {moveStartPos.x, 0, moveStartPos.z};
        jumpEndPos = {jumpEndPos.x, 0, jumpEndPos.z};
        jumpGravity = 0.0f;
    }
	NoteControllerInit(self, noteData, worldRotation, moveStartPos, moveEndPos, jumpEndPos, moveDuration, jumpDuration, jumpGravity, endRotation, uniformScale, rotatesTowardsPlayer, useRandomRotation);

}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = "feetsaber";
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load(); // Load the config file
    getLogger().info("Completed setup!");
}

void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling){
    if(firstActivation){
        GameObject* container = BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

        BeatSaberUI::CreateToggle(container->get_transform(), "Active", getModConfig().Active.GetValue(),
            [](bool value) { 
                getModConfig().Active.SetValue(value);
            });
    }
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();
    getModConfig().Init(modInfo);

    LoggerContextObject logger = getLogger().WithContext("load");

    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController(modInfo, DidActivate);
    QuestUI::Register::RegisterMainMenuModSettingsViewController(modInfo, DidActivate);
    getLogger().info("Successfully installed Settings UI!");

    getLogger().info("Installing hooks...");
    //INSTALL_HOOK(logger, NoteJump_Init);
    INSTALL_HOOK(logger, NoteControllerInit);
    getLogger().info("Installed all hooks!");
}
