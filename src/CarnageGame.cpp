#include "stdafx.h"
#include "CarnageGame.h"
#include "RenderingManager.h"
#include "SpriteManager.h"
#include "ConsoleWindow.h"
#include "GameCheatsWindow.h"
#include "PhysicsManager.h"
#include "Pedestrian.h"
#include "MemoryManager.h"
#include "TimeManager.h"
#include "TrafficManager.h"
#include "AiManager.h"
#include "GameTextsManager.h"
#include "BroadcastEventsManager.h"
#include "AudioManager.h"
#include "cvars.h"
#include "ParticleEffectsManager.h"
#include "WeatherManager.h"

//////////////////////////////////////////////////////////////////////////

static const char* InputsConfigPath = "config/inputs.json";

//////////////////////////////////////////////////////////////////////////
// cvars
//////////////////////////////////////////////////////////////////////////

CvarString gCvarMapname("g_mapname", "", "Current map name", CvarFlags_Init);
CvarString gCvarCurrentBaseDir("g_basedir", "", "Current gta data location", CvarFlags_Init);
CvarEnum<eGtaGameVersion> gCvarGameVersion("g_gamever", eGtaGameVersion_Unknown, "Current gta game version", CvarFlags_Init);
CvarString gCvarGameLanguage("g_gamelang", "en", "Current game language", CvarFlags_Init);
CvarInt gCvarNumPlayers("g_numplayers", 1, "Number of players in split screen mode", CvarFlags_Init);

//////////////////////////////////////////////////////////////////////////

CarnageGame gCarnageGame;

//////////////////////////////////////////////////////////////////////////

bool CarnageGame::Initialize()
{
    debug_assert(mCurrentStateID == eGameStateID_Initial);

    // init randomizer
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    mGameRand.set_seed((unsigned int) ms.count());

    gGameParams.SetToDefaults();

    if (!DetectGameVersion())
    {
        gConsole.LogMessage(eLogMessage_Debug, "Fail to detect game version");
    }

    ::memset(mHumanPlayers, 0, sizeof(mHumanPlayers));

    gGameCheatsWindow.mWindowShown = true; // show by default

    if (gCvarMapname.mValue.empty())
    {
        // try load first found map
        if (!gFiles.mGameMapsList.empty())
        {
            gCvarMapname.mValue = gFiles.mGameMapsList[0];
        }
    }
    gCvarMapname.ClearModified();

    // init texts
    if (!gCvarGameLanguage.mValue.empty())
    {
        gConsole.LogMessage(eLogMessage_Debug, "Set game language: '%s'", gCvarGameLanguage.mValue.c_str());
    }

    std::string textsFilename = GetTextsLanguageFileName(gCvarGameLanguage.mValue);
    gConsole.LogMessage(eLogMessage_Debug, "Loading game texts from '%s'", textsFilename.c_str());

    gGameTexts.Initialize();
    if (!gGameTexts.LoadTexts(textsFilename))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Fail to load game texts for current language");
        gGameTexts.Deinit();
    }

    // init scenario
    if (!StartScenario(gCvarMapname.mValue))
    {
        ShutdownCurrentScenario();
        gConsole.LogMessage(eLogMessage_Warning, "Fail to start game"); 

        mCurrentStateID = eGameStateID_Error;
    }

    gDebugConsoleWindow.mWindowShown = IsErrorGameState();
    return true;
}

void CarnageGame::Deinit()
{
    ShutdownCurrentScenario();

    gGameTexts.Deinit();

    mCurrentStateID = eGameStateID_Initial;
}

bool CarnageGame::IsMenuGameState() const
{
    return mCurrentStateID == eGameStateID_MainMenu;
}

bool CarnageGame::IsInGameState() const
{
    return mCurrentStateID == eGameStateID_InGame;
}

bool CarnageGame::IsErrorGameState() const
{
    return mCurrentStateID == eGameStateID_Error;
}

void CarnageGame::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;

    // advance game state
    if (IsInGameState())
    {
        gSpriteManager.UpdateBlocksAnimations(deltaTime);
        gPhysics.UpdateFrame();
        gGameObjectsManager.UpdateFrame();
        gWeatherManager.UpdateFrame();
        gParticleManager.UpdateFrame();

        for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
        {
            if (mHumanPlayers[ihuman] == nullptr)
                continue;

            mHumanPlayers[ihuman]->UpdateFrame();
        }

        gTrafficManager.UpdateFrame();
        gAiManager.UpdateFrame();
        gBroadcastEvents.UpdateFrame();
    }
}

void CarnageGame::InputEventLost()
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanPlayers[ihuman] == nullptr)
            continue;

        mHumanPlayers[ihuman]->InputEventLost();
    }
}

void CarnageGame::InputEvent(KeyInputEvent& inputEvent)
{
    if (inputEvent.HasPressed(eKeycode_TILDE)) // show debug console
    {
        gDebugConsoleWindow.mWindowShown = !gDebugConsoleWindow.mWindowShown;
        return;
    }
    if (inputEvent.HasPressed(eKeycode_F3))
    {
        gRenderManager.ReloadRenderPrograms();
        return;
    }

    if (inputEvent.HasPressed(eKeycode_ESCAPE))
    {
        gSystem.QuitRequest();
        return;
    }

    if (inputEvent.HasPressed(eKeycode_C))
    {
        gGameCheatsWindow.mWindowShown = !gGameCheatsWindow.mWindowShown;
        return;
    }

    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanPlayers[ihuman] == nullptr)
            continue;

        mHumanPlayers[ihuman]->InputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(MouseButtonInputEvent& inputEvent)
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanPlayers[ihuman] == nullptr)
            continue;

        mHumanPlayers[ihuman]->InputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(MouseMovedInputEvent& inputEvent)
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanPlayers[ihuman] == nullptr)
            continue;

        mHumanPlayers[ihuman]->InputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(MouseScrollInputEvent& inputEvent)
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanPlayers[ihuman] == nullptr)
            continue;

        mHumanPlayers[ihuman]->InputEvent(inputEvent);
    }
}

void CarnageGame::InputEvent(KeyCharEvent& inputEvent)
{
}

void CarnageGame::InputEvent(GamepadInputEvent& inputEvent)
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        if (mHumanPlayers[ihuman] == nullptr)
            continue;

        mHumanPlayers[ihuman]->InputEvent(inputEvent);
    }
}

bool CarnageGame::SetInputActionsFromConfig()
{
    // force default mapping for first player
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        HumanPlayer* currentPlayer = mHumanPlayers[ihuman];
        if (currentPlayer == nullptr)
            continue;

        currentPlayer->mActionsMapping.Clear();
        if (ihuman == 0) 
        {
            currentPlayer->mActionsMapping.SetDefaults();
        }  
    }

    // open config document
    cxx::json_document configDocument;
    if (!gFiles.ReadConfig(InputsConfigPath, configDocument))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load inputs config from '%s'", InputsConfigPath);
        return false;
    }

    std::string tempString;
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        HumanPlayer* currentPlayer = mHumanPlayers[ihuman];
        if (currentPlayer == nullptr)
            continue;

        tempString = cxx::va("player%d", ihuman + 1);

        cxx::json_document_node rootNode = configDocument.get_root_node();
        cxx::json_document_node configNode = rootNode[tempString];
        currentPlayer->mActionsMapping.LoadConfig(configNode);
    }

    return true;
}

void CarnageGame::SetupHumanPlayer(int humanIndex, Pedestrian* pedestrian)
{
    if (mHumanPlayers[humanIndex])
    {
        debug_assert(false);
        return;
    }

    debug_assert(humanIndex < GAME_MAX_PLAYERS);
    debug_assert(pedestrian);

    HumanPlayer* humanPlayer = new HumanPlayer(pedestrian);
    mHumanPlayers[humanIndex] = humanPlayer;

    humanPlayer->mSpawnPosition = pedestrian->GetPosition();
    humanPlayer->mPlayerView.mFollowCameraController.SetFollowTarget(pedestrian);
    humanPlayer->mPlayerView.mHUD.SetupHUD(humanPlayer);
}

void CarnageGame::DeleteHumanPlayer(int playerIndex)
{
    debug_assert(playerIndex < GAME_MAX_PLAYERS);

    if (mHumanPlayers[playerIndex])
    {
        gRenderManager.DetachRenderView(&mHumanPlayers[playerIndex]->mPlayerView);
        mHumanPlayers[playerIndex]->mPlayerView.SetCameraController(nullptr);

        SafeDelete(mHumanPlayers[playerIndex]);
    }
}

void CarnageGame::SetupScreenLayout()
{   
    const int MaxCols = 2;
    const int playersCount = GetHumanPlayersCount();

    debug_assert(playersCount > 0);

    Rect fullViewport = gGraphicsDevice.mViewportRect;

    int numRows = (playersCount + MaxCols - 1) / MaxCols;
    debug_assert(numRows > 0);

    int frameSizePerH = fullViewport.h / numRows;

    for (int icurr = 0; icurr < playersCount; ++icurr)
    {
        debug_assert(mHumanPlayers[icurr]);

        int currRow = icurr / MaxCols;
        int currCol = icurr % MaxCols;

        int colsOnCurrentRow = glm::clamp(playersCount - (currRow * MaxCols), 1, MaxCols);
        debug_assert(colsOnCurrentRow);
        int frameSizePerW = fullViewport.w / colsOnCurrentRow;
        
        HumanPlayerView& currView = mHumanPlayers[icurr]->mPlayerView;
        currView.mCamera.mViewportRect.h = frameSizePerH;
        currView.mCamera.mViewportRect.x = currCol * (frameSizePerW + 1);
        currView.mCamera.mViewportRect.y = (numRows - currRow - 1) * (frameSizePerH + 1);
        currView.mCamera.mViewportRect.w = frameSizePerW;
 
        currView.SetCameraController(&currView.mFollowCameraController);
        gRenderManager.AttachRenderView(&currView);
    }
}

int CarnageGame::GetHumanPlayerIndex(const HumanPlayer* controller) const
{
    if (controller == nullptr)
    {
        debug_assert(false);
        return -1;
    }
    for (int icurr = 0; icurr < GAME_MAX_PLAYERS; ++icurr)
    {
        if (mHumanPlayers[icurr] == controller)
            return icurr;
    }
    return -1;
}

bool CarnageGame::StartScenario(const std::string& mapName)
{
    ShutdownCurrentScenario();

    if (mapName.empty())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Map name is not specified");
        return false;
    }

    if (!gGameMap.LoadFromFile(mapName))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load map '%s'", mapName.c_str());
        return false;
    }
    if (!gAudioManager.LoadLevelSounds())
    {
        // ignore
    }
    gSpriteManager.Cleanup();
    gRenderManager.mMapRenderer.BuildMapMesh();
    if (!gSpriteManager.InitLevelSprites())
    {
        debug_assert(false);
    }
    //gSpriteManager.DumpSpriteDeltas("D:/Temp/gta1_deltas");
    //gSpriteCache.DumpBlocksTexture("D:/Temp/gta1_blocks");
    //gSpriteManager.DumpSpriteTextures("D:/Temp/gta1_sprites");
    //gSpriteManager.DumpCarsTextures("D:/Temp/gta_cars");

    gPhysics.EnterWorld();
    gParticleManager.EnterWorld();
    gGameObjectsManager.EnterWorld();
    // temporary
    //glm::vec3 pos { 108.0f, 2.0f, 25.0f };
    //glm::vec3 pos { 14.0, 2.0f, 38.0f };
    //glm::vec3 pos { 91.0f, 2.0f, 236.0f };
    //glm::vec3 pos { 121.0f, 2.0f, 200.0f };
    //glm::vec3 pos { 174.0f, 2.0f, 230.0f };

    gCvarNumPlayers.mValue = glm::clamp(gCvarNumPlayers.mValue, 1, GAME_MAX_PLAYERS);
    gConsole.LogMessage(eLogMessage_Info, "Num players: %d", gCvarNumPlayers.mValue);

    glm::vec3 pos[GAME_MAX_PLAYERS];

    // choose spawn point
    // it is temporary!
    int currFindPosIter = 0;
    for (int yBlock = 10; yBlock < 20 && currFindPosIter < gCvarNumPlayers.mValue; ++yBlock)
    {
        for (int xBlock = 10; xBlock < 20 && currFindPosIter < gCvarNumPlayers.mValue; ++xBlock)
        {
            for (int zBlock = MAP_LAYERS_COUNT - 1; zBlock > -1; --zBlock)
            {
                const MapBlockInfo* currBlock = gGameMap.GetBlockInfo(xBlock, yBlock, zBlock);
                if (currBlock->mGroundType == eGroundType_Field ||
                    currBlock->mGroundType == eGroundType_Pawement ||
                    currBlock->mGroundType == eGroundType_Road)
                {
                    pos[currFindPosIter++] = Convert::MapUnitsToMeters(glm::vec3(xBlock + 0.5f, zBlock, yBlock + 0.5f));
                    break;
                }
            }
        }
    }

    ePedestrianType playerPedTypes[GAME_MAX_PLAYERS] =
    {
        ePedestrianType_Player1,
        ePedestrianType_Player2,
        ePedestrianType_Player3,
        ePedestrianType_Player4,
    };

    for (int icurr = 0; icurr < gCvarNumPlayers.mValue; ++icurr)
    {
        cxx::angle_t pedestrianHeading { 360.0f * gCarnageGame.mGameRand.generate_float(), cxx::angle_t::units::degrees };

        Pedestrian* pedestrian = gGameObjectsManager.CreatePedestrian(pos[icurr], pedestrianHeading, playerPedTypes[icurr]);
        SetupHumanPlayer(icurr, pedestrian);
    }

    SetInputActionsFromConfig();
    SetupScreenLayout();

    gTrafficManager.StartupTraffic();
    gWeatherManager.EnterWorld();

    mCurrentStateID = eGameStateID_InGame;
    return true;
}

void CarnageGame::ShutdownCurrentScenario()
{
    for (int ihuman = 0; ihuman < GAME_MAX_PLAYERS; ++ihuman)
    {
        DeleteHumanPlayer(ihuman);
    }
    gAiManager.ReleaseAiControllers();
    gTrafficManager.CleanupTraffic();
    gWeatherManager.ClearWorld();
    gGameObjectsManager.ClearWorld();
    gPhysics.ClearWorld();
    gGameMap.Cleanup();
    gBroadcastEvents.ClearEvents();
    gAudioManager.FreeLevelSounds();
    gParticleManager.ClearWorld();
    mCurrentStateID = eGameStateID_Initial;
}

int CarnageGame::GetHumanPlayersCount() const
{
    int playersCounter = 0;
    for (HumanPlayer* currPlayer: mHumanPlayers)
    {
        if (currPlayer)
        {
            ++playersCounter;
        }
    }
    return playersCounter;
}

bool CarnageGame::DetectGameVersion()
{
    bool useAutoDetection = (gCvarGameVersion.mValue == eGtaGameVersion_Unknown);
    if (useAutoDetection)
    {
        const int GameMapsCount = (int) gFiles.mGameMapsList.size();
        if (GameMapsCount == 0)
            return false;

        if (gFiles.IsFileExists("MISSUK.INI"))
        {
            gCvarGameVersion.mValue = eGtaGameVersion_MissionPack1_London69;
        }
        else if (gFiles.IsFileExists("missuke.ini"))
        {
            gCvarGameVersion.mValue = eGtaGameVersion_MissionPack2_London61;
        }
        else if (GameMapsCount < 3)
        {
            gCvarGameVersion.mValue = eGtaGameVersion_Demo;
        }
        else
        {
            gCvarGameVersion.mValue = eGtaGameVersion_Full;
        }
    }

    gConsole.LogMessage(eLogMessage_Debug, "Gta game version is '%s' (%s)", cxx::enum_to_string(gCvarGameVersion.mValue),
        useAutoDetection ? "autodetect" : "forced");
    
    return true;
}

std::string CarnageGame::GetTextsLanguageFileName(const std::string& languageID) const
{
    if ((gCvarGameVersion.mValue == eGtaGameVersion_Demo) || (gCvarGameVersion.mValue == eGtaGameVersion_Full))
    {
        if (cxx_stricmp(languageID.c_str(), "en") == 0)
        {
            return "ENGLISH.FXT";
        }

        if (cxx_stricmp(languageID.c_str(), "fr") == 0)
        {
            return "FRENCH.FXT";
        }

        if (cxx_stricmp(languageID.c_str(), "de") == 0)
        {
            return "GERMAN.FXT";
        }

        if (cxx_stricmp(languageID.c_str(), "it") == 0)
        {
            return "ITALIAN.FXT";
        }

        return "ENGLISH.FXT";
    }

    if (gCvarGameVersion.mValue == eGtaGameVersion_MissionPack1_London69)
    {
        if (cxx_stricmp(languageID.c_str(), "en") == 0)
        {
            return "ENGUK.FXT";
        }

        if (cxx_stricmp(languageID.c_str(), "fr") == 0)
        {
            return "FREUK.FXT";
        }

        if (cxx_stricmp(languageID.c_str(), "de") == 0)
        {
            return "GERUK.FXT";
        }

        if (cxx_stricmp(languageID.c_str(), "it") == 0)
        {
            return "ITAUK.FXT";
        }

        return "ENGUK.FXT";
    }

    if (gCvarGameVersion.mValue == eGtaGameVersion_MissionPack2_London61)
    {
        return "enguke.fxt";
    }
   
    return "ENGLISH.FXT";
}
