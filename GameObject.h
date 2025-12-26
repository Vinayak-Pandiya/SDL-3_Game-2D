#include<glm/glm.hpp>
#include<vector>
#include<SDL3/SDL.h>

#include"animation.h"

enum class PlayerState
{
    idle,running,jumping, death, defending, hurt,attack
};

enum class BulletState
{
    moving,colliding,inactive
};

struct PlayerData
{
    PlayerState state;
    timer weaponTimer;
    PlayerData():weaponTimer(0.1f)
    {
        state=PlayerState::idle;
    };
};

struct Leveldata{};
struct Enimydata{};
struct Bulletdata
{
    BulletState state;
    Bulletdata():state(BulletState::moving)
    {
    }
};

union objecttype
{
    PlayerData player;
    Leveldata level;
    Enimydata enimey;
    Bulletdata bullet;
};


enum class ObjectType
{
    player,level,enemy,bullet
};




struct GameObject
{
    ObjectType type;
    objecttype data;
    glm::vec2 position,velocity, accelaration;
    float direction;
    float maxSpeedX;
    std::vector<Animation> animations;
    int currentAnimation;
    SDL_Texture *texture;
    bool dynamic;
    bool grounded;
    SDL_FRect collider;


    GameObject():data{.level=Leveldata()},collider{0}
    {
        type = ObjectType::level;
        direction = 1;
        maxSpeedX=0;
        position=velocity=accelaration=glm::vec2(0);
        currentAnimation=-1;
        texture=nullptr;
        dynamic=false;
        grounded=false;
    }
};