#include "main.hpp"
#include "al/area/AreaObj.h"
#include "al/area/AreaObjGroup.h"
#include "al/camera/CameraDirector.h"
#include "al/camera/CameraPoser.h"
#include "al/util.hpp"
#include "al/util/ControllerUtil.h"
#include "al/util/LiveActorUtil.h"
#include "al/util/StringUtil.h"
#include "cameras/CameraPoserCustom.h"
#include "container/seadListImpl.h"
#include "container/seadSafeArray.h"
#include "debugMenu.hpp"
#include "game/Actors/Shine.h"
#include "game/GameData/GameDataFile.h"
#include "game/GameData/GameDataFunction.h"
#include "gfx/seadFrameBuffer.h"
#include "keeper.hpp"
#include "rs/util.hpp"
#include "types.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

static bool showMenu = true;

static bool isInGame = false;

DebugWarpPoint warpPoints[40];

int listCount = 0;

int curWarpPoint = 0;

void drawBackground(agl::DrawContext *context)
{

    sead::Vector3<float> p1; // top left
    p1.x = -1.0;
    p1.y = 0.3;
    p1.z = 0.0;
    sead::Vector3<float> p2; // top right
    p2.x = -0.2;
    p2.y = 0.3;
    p2.z = 0.0;
    sead::Vector3<float> p3; // bottom left
    p3.x = -1.0;
    p3.y = -1.0;
    p3.z = 0.0;
    sead::Vector3<float> p4; // bottom right
    p4.x = -0.2;
    p4.y = -1.0;
    p4.z = 0.0;

    sead::Color4f c;
    c.r = 0.1;
    c.g = 0.1;
    c.b = 0.1;
    c.a = 0.9;

    agl::utl::DevTools::beginDrawImm(context, sead::Matrix34<float>::ident, sead::Matrix44<float>::ident);
    agl::utl::DevTools::drawTriangleImm(context, p1, p2, p3, c);
    agl::utl::DevTools::drawTriangleImm(context, p3, p4, p2, c);
}

// ------------- Hooks -------------

// Do Nothing ATM
al::StageInfo *initDebugListHook(const al::Scene *curScene)
{

    // hook that gets all objects put in DebugList and adds their coordinates to a warp point array

    al::StageInfo *info = al::getStageInfoMap(curScene, 0);

    al::PlacementInfo rootInfo = al::PlacementInfo();

    al::tryGetPlacementInfoAndCount(&rootInfo, &listCount, info, "DebugList");

    if (listCount > 0)
    {
        for (size_t i = 0; i < listCount; i++)
        {
            al::PlacementInfo objInfo = al::PlacementInfo();

            al::getPlacementInfoByIndex(&objInfo, rootInfo, i);

            const char *displayName = "";
            al::tryGetDisplayName(&displayName, objInfo);

            strcpy(warpPoints[i].pointName, displayName);

            al::tryGetTrans(&warpPoints[i].warpPos, objInfo);
        }
    }

    return info;
}

void drawMainHook(HakoniwaSequence *curSequence, sead::Viewport *viewport, sead::DrawContext *drawContext)
{

    if (!showMenu)
    {
        al::executeDraw(curSequence->mLytKit, "２Ｄバック（メイン画面）");
        return;
    }

    int dispWidth = al::getLayoutDisplayWidth();
    int dispHeight = al::getLayoutDisplayHeight();

    gTextWriter->mViewport = viewport;

    gTextWriter->mColor = sead::Color4f(
        1.f,
        1.f,
        1.f,
        0.4f);

    al::Scene *curScene = curSequence->curScene;

    if (curScene && isInGame)
    {

        drawBackground((agl::DrawContext *)drawContext);

        gTextWriter->beginDraw();

        gTextWriter->setCursorFromTopLeft(sead::Vector2f(10.f, (dispHeight / 3) + 30.f));

        gTextWriter->setScaleFromFontHeight(20.f);

        gTextWriter->printf("Seed: %d", getKeeper().origSeed);


        // if (getKeeper().msg) {
        //     gTextWriter->printf("Shine Name: %s, %d, %d", getKeeper().msg, getKeeper().param_3, getKeeper().param_4);
        // }

        // if (getKeeper().seed_arr[0][0] != 0) {
        //     gTextWriter->printf("Next Num: %d\n", getKeeper().next_num);
        //         for (int i = 0; i < 20; ++i) {
        //             gTextWriter->printf("%d, ", getKeeper().seed_arr[getKeeper().stageNum][i]);
        //         }
        // }

        isInGame = false;
    }

    gTextWriter->endDraw();

    al::executeDraw(curSequence->mLytKit, "２Ｄバック（メイン画面）");
}

void stageInitHook(StageScene *initStageScene, al::SceneInitInfo *sceneInitInfo)
{
    __asm("MOV X19, X0");
    __asm("LDR X24, [X1, #0x18]");

    // place any code that needs to be ran during init here (creating actors for example)

    __asm("MOV X1, X24");
}

ulong threadInit()
{ // hook for initializing any threads we need
    __asm("STR X21, [X19,#0x208]");

    return 0x20;
}

void stageSceneHook()
{

    __asm("MOV X19, X0");

    StageScene *stageScene;
    __asm("MOV %[result], X0"
          : [result] "=r"(stageScene));

    al::PlayerHolder *pHolder = al::getScenePlayerHolder(stageScene);
    PlayerActorHakoniwa *p1 = (PlayerActorHakoniwa*)al::tryGetPlayerActor(pHolder, 0);

    isInGame = true;

    __asm("MOV X0, %[input]"
          : [input] "=r"(stageScene));
}

void seadPrintHook(const char *fmt, ...) // hook for replacing sead::system::print with our custom logger
{
    va_list args;
    va_start(args, fmt);

    gLogger->LOG(fmt, args);

    va_end(args);
}

int find(const char* seek) {
    static constexpr const char *stageNames[17] = {"CapWorldHomeStage", "WaterfallWorldHomeStage", "SandWorldHomeStage", "LakeWorldHomeStage", "ForestWorldHomeStage", "CloudWorldHomeStage", "ClashWorldHomeStage", "CityWorldHomeStage", "SnowWorldHomeStage", "SeaWorldHomeStage", "LavaWorldHomeStage", "BossRaidWorldHomeStage", "SkyWorldHomeStage", "MoonWorldHomeStage", "PeachWorldHomeStage", "Special1WorldHomeStage", "Special2WorldHomeStage"};
    for (int i = 0; i < 17; ++i) {
        if (strcmp(stageNames[i], seek) == 0) return i;
    }
    return -1;
}

std::array<std::array<int, 120>, 17> generate_2d_array(unsigned seed) {
    std::array<std::array<int, 120>, 17> array;    

    // Initialize the vector with values from 1 to num_cols
    std::array<int, 120> numbers;
    for (int i = 0; i < 120; ++i) {
        numbers[i] = i + 1;
    }

    // Set the seed for reproducibility
    std::mt19937 rng(seed);

    for (int i = 0; i < 17; ++i) {
        // Shuffle the numbers)
        std::shuffle(numbers.begin(), numbers.end(), rng);

        // Assign the shuffled numbers to the 2D array
        for (int j = 0; j < 120; ++j) {
            array[i][j] = numbers[j];
        }
    }

    return array;
}

bool hakoniwaSequenceHook(HakoniwaSequence *sequence)
{
	StageScene *stageScene = (StageScene *)sequence->curScene;

	bool isFirstStep = al::isFirstStep(sequence);

	al::PlayerHolder *pHolder = al::getScenePlayerHolder(stageScene);
	PlayerActorHakoniwa *p1 = (PlayerActorHakoniwa *)al::tryGetPlayerActor(pHolder, 0);

    getKeeper().stageSceneRef = stageScene;

    if (al::isPadHoldZL(-1) && al::isPadTriggerUp(-1)) {
        showMenu = !showMenu;
    }

    if (showMenu) {
        if (al::isPadTriggerRight(-1)) {
            getKeeper().origSeed++;
            getKeeper().seed_arr = generate_2d_array(getKeeper().origSeed);
        } else if (al::isPadTriggerLeft(-1)) {
            getKeeper().origSeed--;
            getKeeper().seed_arr = generate_2d_array(getKeeper().origSeed);
        }
    }

	return isFirstStep;
}

int getRandomHook(int zero, int max) {
    return getKeeper().next_num;
}

void tableHook(int* arr, int* num_avalible, al::LiveActor* poetter) {
    if (getKeeper().seed_arr[0][0] == 0) {
        // getKeeper().seed_arr = readNumbersFromFile("sd:/atmosphere/contents/0100000000010000/seed.txt");
        getKeeper().seed_arr = generate_2d_array(getKeeper().origSeed);
    }
    rs::calcShineIndexTableNameUnlockable(arr, num_avalible, poetter);

    getKeeper().tableArr = arr;
    getKeeper().tableNum = *num_avalible;

    int stage_id = find(GameDataFunction::getCurrentStageName(getKeeper().stageSceneRef->mHolder));
    getKeeper().stageNum = stage_id;

    for (int i = 0; i < 120; ++i) {
        for (int j = 0; j < *num_avalible; ++j) {
            if (arr[j] == getKeeper().seed_arr[stage_id][i]) {
                getKeeper().next_num = j;
                return;
            }
        }
    }

    return;
}
