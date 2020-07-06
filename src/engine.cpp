#include "common.h"
#include "reflection.h"
#include "game_state.h"

#include "linear_allocator.cpp"
#include "shader.cpp"
#include "shader_manager.cpp"
#include "game.cpp"

#define DLLEXPORT extern "C" __declspec(dllexport)

struct Introspection
{
    LinearAllocator allocator;
    reflection::TypeDB types;
    uint8_t allocMemory[kilobytes(64)];

    Introspection()
        : allocator{kilobytes(64), allocMemory},
          types{&allocator}
    {
    }
};

struct EngineData
{
    Introspection meta;
    GameState state;
};
    
static void mutateFromIntrospectionInfo(void *oldObject, void *newObject, reflection::Type *oldType, reflection::Type *newType) {
    if (newType->fieldCount == 0)
    {
        memcpy(newObject, oldObject, newType->size);
        return;
    }

    reflection::Field *newFields = newType->fields;
    for (int i = 0; i < newType->fieldCount; i++)
    {
        if(newFields[i].isPointer) {
            continue;
        }

        reflection::Field *field = oldType->findField(newFields[i].name);
        if (field)
        {
            mutateFromIntrospectionInfo((uint8_t *)oldObject + field->offset,
                        (uint8_t *)newObject + newFields[i].offset,
                        field->type,
                        newFields[i].type);
            continue;
        }

        newFields[i].type->constructor((uint8_t *)newObject + newFields[i].offset);
    }
}

DLLEXPORT void onLoad(bool isInit, GameMemory *gameMemory)
{
    //TODO: This has to be here for now because the game doesn't know when it is reloaded.
    gladLoadGL();

    int prevDataIndex = gameMemory->currentDataIndex;
    if (!isInit) {
        gameMemory->currentDataIndex = 1 - prevDataIndex;    //Swap memory blocks
    }

    //! This constructor initializes everything, even fields that will be replaced during mutation.
    //TODO: Find a way to only initialize new fields with default values, leaving old fields intact.
    EngineData *data = new (gameMemory->data[gameMemory->currentDataIndex]) EngineData{};
    reflection::Type *newType = data->meta.types.create<GameState>();

    if (isInit)
    {
        init(&data->state);
    }
    else
    {
        EngineData *prevData = (EngineData *)gameMemory->data[prevDataIndex];
        reflection::Type *oldType = prevData->meta.types.get<GameState>();
        mutateFromIntrospectionInfo(&prevData->state, &data->state, oldType, newType);
        prevData->meta.allocator.clear();

        data->state.reloadablesAlloc.clear();
    }
}

DLLEXPORT void update(GameMemory *gameMemory, PlayerInput* input, float dt, float time)
{
    GameState *state = (GameState*)&((EngineData *)gameMemory->data[gameMemory->currentDataIndex])->state;
    update(state, input, dt, time);
}

DLLEXPORT void render(GameMemory *gameMemory, float deltaInterpolation)
{
    GameState *state = (GameState*)&((EngineData *)gameMemory->data[gameMemory->currentDataIndex])->state;
    render(state, deltaInterpolation);
}