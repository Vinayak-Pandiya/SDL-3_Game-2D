#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>
#include<SDL3_image/SDL_image.h>
#include<array>
#include<string>
#include<glm/glm.hpp>
#include<iostream>
#include<format>

#include"GameObject.h"


using namespace std;


const size_t LAYER_INDEX_LEVEL=0;
const size_t LAYER_INDEX_CHARACTERS=1;
const int maprows=5;
const int mapcols=50;
const int tilesize= 32;


struct SDLState
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int width,height,logw,logh;
    const bool *keys;
    SDLState():keys(SDL_GetKeyboardState((nullptr)))
    {

    }
};

struct gamestate
{
    array< vector<GameObject>,2> layers;
    vector<GameObject>backgroundtiles;
    vector<GameObject>foregroundtiles;

    vector<GameObject>bullets;

    int palyerIndex;
    SDL_FRect mapViewport;
    bool debugmode;
    float bg2scroll,bg3scroll, bg4scroll;


    gamestate(SDLState &state)
    {
        mapViewport=SDL_FRect{
            .x=0,.y=0,
            .w=(float)state.logw,.h=(float)state.logh
        };
        palyerIndex=-1;
        bg2scroll=bg3scroll=bg4scroll=0;
        debugmode=false;
    }

    GameObject &player() {return layers[LAYER_INDEX_CHARACTERS][palyerIndex];}
};


struct Resources
{
    const int ANIM_PLAYER_IDLE = 0;
    const int ANIM_PLAYER_RUN = 1;
    const int ANIM_PLAYER_SLIDE = 2;
	const int ANIM_PLAYER_SHOOT = 3;
	const int ANIM_PLAYER_SLIDE_SHOOT = 4;
    vector<Animation> playerAnims;
    const int ANIM_BULLET_MOVING=0;
    const int ANIM_BULLET_HIT=1;
    vector<Animation> bulletAnims;


    vector<SDL_Texture *> textures;
    SDL_Texture *texIdle, *texRun, *texbrick, *texgrass, *texground, *texpanel, *texbg1,
    *texbg2, *texbg3 , *texbg4 , *texbullet,*texbulletHit,*texShoot,*texRunShoot,*texSlideShoot,*texSlide;

    SDL_Texture *loadTexture( SDL_Renderer *renderer,const string &filepath)
    {
        //Loading Data In gpu
        SDL_Texture *Tex = IMG_LoadTexture(renderer,filepath.c_str());
        SDL_SetTextureScaleMode(Tex,SDL_SCALEMODE_NEAREST);
        textures.push_back(Tex);
        return Tex;
    }

    void load(SDLState &state)
    {
        playerAnims.resize(5);
        // playerAnims[ANIM_PLAYER_IDLE]= Animation(5,0.5f);
        // playerAnims[ANIM_PLAYER_RUN]=Animation(8,0.8f);
        playerAnims[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);
		playerAnims[ANIM_PLAYER_RUN] = Animation(4, 0.5f);

        playerAnims[ANIM_PLAYER_SLIDE] = Animation(1, 1.0f);
		playerAnims[ANIM_PLAYER_SHOOT] = Animation(4, 0.5f);
		playerAnims[ANIM_PLAYER_SLIDE_SHOOT] = Animation(4, 0.5f);

        bulletAnims.resize(2);
        bulletAnims[ANIM_BULLET_MOVING]=Animation(4,0.05f);
        bulletAnims[ANIM_BULLET_HIT]=Animation(4,0.15f);

        texIdle = loadTexture(state.renderer, "./data/data/idle.png");
		texRun = loadTexture(state.renderer, "./data/data/run.png");
		texbrick = loadTexture(state.renderer, ".\\data\\data\\tiles\\brick.png");
		texgrass = loadTexture(state.renderer, ".\\data\\data\\tiles\\grass.png");
		texground = loadTexture(state.renderer, ".\\data\\data\\tiles\\ground.png");
		texpanel = loadTexture(state.renderer, ".\\data\\data\\tiles\\panel.png");
		texbg1 = loadTexture(state.renderer, ".\\data\\data\\bg\\bg_layer1.png");
		texbg2 = loadTexture(state.renderer, ".\\data\\data\\bg\\bg_layer2.png");
		texbg3 = loadTexture(state.renderer, ".\\data\\data\\bg\\bg_layer3.png");
		texbg4 = loadTexture(state.renderer, ".\\data\\data\\bg\\bg_layer4.png");

		texbullet = loadTexture(state.renderer, ".\\data\\data\\bullet.png");
		texbulletHit = loadTexture(state.renderer, ".\\data\\data\\bullet_hit.png");
		texShoot = loadTexture(state.renderer, ".\\data\\data\\shoot.png");
		texRunShoot = loadTexture(state.renderer, ".\\data\\data\\shoot_run.png");
		texSlideShoot = loadTexture(state.renderer, ".\\data\\data\\slide_shoot.png");

        texSlide= loadTexture(state.renderer, ".\\data\\data\\slide.png");
		// texEnemy = loadTexture(state.renderer, ".\\data\\data\\enemy.png");
		// texEnemyHit = loadTexture(state.renderer, ".\\data\\data\\enemy_hit.png");
		// texEnemyDie = loadTexture(state.renderer, ".\\data\\data\\enemy_die.png");
    }

    void unload()
    {
        for(SDL_Texture *tex : textures)
        {
            SDL_DestroyTexture(tex);
        }
    }

};

void cleanup(SDLState &state);
bool initialize(SDLState &state);
void drawObject(const SDLState &state, gamestate &gs , GameObject &obj,float width,float height, float &deltat);
void update(const SDLState &state,gamestate &gs, Resources &res, GameObject &obj, float deltat);
void createTiles(const SDLState &state, gamestate &gs, const Resources &res );
void checkColllison(const SDLState &state, gamestate &gs, const Resources &res,GameObject &a, GameObject &b, float deltat);
void handleKeyInput(const SDLState &state, gamestate &gs, GameObject &obj, SDL_Scancode key, bool keyDown);
void drawParalaxBackground(SDL_Renderer *renderer, SDL_Texture *texture, float xVelocity, float &scrollpos,float const scrollfact, float deltat, float offset);

int main(int argc, char *argv[]){

    SDLState state;
    state.width=1600;
    state.height=900;
    state.logw=640;
    state.logh=320;
    
    if(!initialize(state)){
        return 1;
    }
    //Load data
    Resources res;
    res.load(state);
    

    //game data
    gamestate gs(state);
    createTiles(state, gs, res );

    uint64_t previoustime =SDL_GetTicks();


    //GAME Loop
    bool running=true;
    

    while(running)
    {
        uint64_t nowtime =SDL_GetTicks();
        float deltat =(nowtime-previoustime)/1000.0f ;
        SDL_Event event{0};
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_EVENT_QUIT:
                {
                    running=false;
                    break;
                }
                case SDL_EVENT_WINDOW_RESIZED:
                {
                    state.width=event.window.data1;
                    state.height=event.window.data2;
                }
                case SDL_EVENT_KEY_DOWN:
                {
                    handleKeyInput(state,gs,gs.player(),event.key.scancode,true);
                    break;
                }
                case SDL_EVENT_KEY_UP:
                {
                    handleKeyInput(state,gs,gs.player(),event.key.scancode,false);
                    if(event.key.scancode ==  SDL_SCANCODE_F12)
                    {
                        gs.debugmode=!gs.debugmode;
                    }
                    break;
                }
            }
        }
        
        for(auto &layer :gs.layers)
        {
            for(GameObject &obj : layer)
            {
                if(obj.dynamic)
                {
                    update(state, gs, res, obj, deltat);
                
                    if(obj.currentAnimation!=-1)
                    {
                        obj.animations[obj.currentAnimation].step(deltat);
                    }
                }
            }
        }
        for(GameObject &bullet:gs.bullets)
        {
            update(state, gs, res, bullet, deltat);
                
                    if(bullet.currentAnimation!=-1)
                    {
                        bullet.animations[bullet.currentAnimation].step(deltat);
                    }
        }
        gs.mapViewport.x=(gs.player().position.x+tilesize/2)-gs.mapViewport.w/2;
        
        //Drawing in the window
        SDL_SetRenderDrawColor(state.renderer,10,10,30,255);
        SDL_RenderClear(state.renderer);

        //background
        SDL_RenderTexture(state.renderer,res.texbg1,nullptr,nullptr);
        drawParalaxBackground(state.renderer,res.texbg2,gs.player().velocity.x,gs.bg2scroll, 0.075f ,deltat,0);
        drawParalaxBackground(state.renderer,res.texbg3,gs.player().velocity.x,gs.bg3scroll, 0.15f ,deltat,200);
        drawParalaxBackground(state.renderer,res.texbg4,gs.player().velocity.x,gs.bg4scroll, 0.3f ,deltat,520);

        //draaw all object
        
        //background
        for(GameObject &obj:gs.backgroundtiles){
            SDL_FRect dst
            {
                .x=obj.position.x - gs.mapViewport.x,
                .y=obj.position.y,
                .w=static_cast<float>(obj.texture->w),
                .h=static_cast<float>(obj.texture->h)
            };
            SDL_RenderTexture(state.renderer,obj.texture,nullptr,&dst);
        }
        //layers
        for(auto &layer :gs.layers)
        {
            for(GameObject &obj : layer)
            {
                drawObject(state,gs,obj,tilesize,tilesize, deltat);
            }
        }
        //bullets
        for(GameObject &bullet:gs.bullets){
            
            drawObject(state,gs,bullet,bullet.collider.w,bullet.collider.h, deltat);
        }
        //foregrounds
        for(GameObject &obj:gs.foregroundtiles){
            SDL_FRect dst
            {
                .x=obj.position.x - gs.mapViewport.x,
                .y=obj.position.y,
                .w=static_cast<float>(obj.texture->w),
                .h=static_cast<float>(obj.texture->h)
            };
            SDL_RenderTexture(state.renderer,obj.texture,nullptr,&dst);
        }

        if(gs.debugmode)
        {
            SDL_SetRenderDrawColor(state.renderer,255,255,255,255);
            SDL_RenderDebugText(state.renderer,10,10,format("State: {},Bullet: {},G: {}",
                 static_cast<int>(gs.player().data.player.state),gs.bullets.size(),gs.player().grounded).c_str());
        }

        SDL_RenderPresent(state.renderer);
        previoustime=nowtime;
    }


    res.unload();
    cleanup(state);

    return 0;
}

bool initialize(SDLState &state)
{
    bool initSucess=true;
    if(!SDL_Init(SDL_INIT_VIDEO)){

        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Error Initializing sdl3", nullptr);
        initSucess=false;
    }

    state.window = SDL_CreateWindow("Shinobi Ninja",state.width, state.height,SDL_WINDOW_RESIZABLE);
    
    if(!state.window){
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Error Creating Window",nullptr);
        cleanup(state);
        initSucess=false;
    }

    //Renderer
    state.renderer=SDL_CreateRenderer(state.window,nullptr);
    if(!state.renderer){
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Error Loading renderer",state.window);
        cleanup(state);
    }
    SDL_SetRenderVSync(state.renderer,1);
    //render presentation
    SDL_SetRenderLogicalPresentation(state.renderer,state.logw,state.logh,SDL_LOGICAL_PRESENTATION_LETTERBOX);
    
    return initSucess;
}

void drawObject(const SDLState &state, gamestate &gs , GameObject &obj,float width,float height, float &deltat)
{
    float srcx= obj.currentAnimation != -1 ? obj.animations[obj.currentAnimation].currentFrame()*width : 0.0f;
    SDL_FRect src{
        .x=srcx,.y=0,
        .w=width,.h=height
    };
    SDL_FRect dst{
        .x=obj.position.x - gs.mapViewport.x,.y=obj.position.y,
        .w= width,.h= height
    };
    SDL_FlipMode flipmode=obj.direction== -1? SDL_FLIP_HORIZONTAL:SDL_FLIP_NONE;
    SDL_RenderTextureRotated(state.renderer,obj.texture, &src, &dst, 0, nullptr, flipmode);


    if(gs.debugmode)
    {
        SDL_FRect rectA{
        .x= obj.position.x + obj.collider.x-gs.mapViewport.x,
        .y= obj.position.y + obj.collider.y,
        .w=obj.collider.w, .h=obj.collider.h};

        SDL_SetRenderDrawBlendMode(state.renderer,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(state.renderer,255,0,0,150);
        SDL_RenderFillRect(state.renderer,&rectA);
        SDL_SetRenderDrawBlendMode(state.renderer,SDL_BLENDMODE_NONE);
    }
}

// void update(const SDLState &state,gamestate &gs, Resources &res, GameObject &obj, float deltat)
// {
//     if (obj.currentAnimation != -1)
// 	{
// 		obj.animations[obj.currentAnimation].step(deltat);
// 	}

//     if(obj.dynamic && !obj.grounded){
//         obj.velocity += glm::vec2(0,500)*deltat;
//     }

//     float currentDirection=0;
//     if(obj.type== ObjectType::player)
//     {
//         if(state.keys[SDL_SCANCODE_A])
//         {
//             currentDirection += -1;
//         }
//         if(state.keys[SDL_SCANCODE_D])
//         {
//             currentDirection += 1;
//         }
//         if(currentDirection)
//         {
//             obj.direction = currentDirection;
//         }

//         timer &weaponTimer = obj.data.player.weaponTimer;
// 		weaponTimer.step(deltat);

// 		const auto handleShooting = [&state, &gs, &res, &obj, &weaponTimer](
// 			SDL_Texture *tex, SDL_Texture *shootTex, int animIndex, int shootAnimIndex)
// 		{
// 			if (state.keys[SDL_SCANCODE_E])
// 			{
// 				// set shooting tex/anim
// 				obj.texture = shootTex;
// 				obj.currentAnimation = shootAnimIndex;

// 				if (weaponTimer.isTimeout())
// 				{
// 					weaponTimer.reset();
// 					// spawn some bullets
// 					GameObject bullet;
// 					bullet.data.bullet = Bulletdata();
// 					bullet.type = ObjectType::bullet;
// 					bullet.direction = gs.player().direction;
// 					bullet.texture = res.texbullet;
// 					bullet.currentAnimation = res.ANIM_BULLET_MOVING;
// 					bullet.collider = SDL_FRect{
// 						.x = 0, .y = 0,
// 						.w = static_cast<float>(res.texbullet->h),
// 						.h = static_cast<float>(res.texbullet->h)
// 					};
// 					const int yVariation = 40;
// 					const float yVelocity = SDL_rand(yVariation) - yVariation / 2.0f;
// 					bullet.velocity = glm::vec2(
// 						obj.velocity.x + 600.0f * obj.direction,
// 						yVelocity
// 					);
// 					bullet.maxSpeedX = 1000.0f;
// 					bullet.animations = res.bulletAnims;

// 					// adjust bullet start position
// 					const float left = 4;
// 					const float right = 24;
// 					const float t = (obj.direction + 1) / 2.0f; // results in a value of 0..1
// 					const float xOffset = left + right * t; // LERP between left and right based on direction
// 					bullet.position = glm::vec2(
// 						obj.position.x + xOffset,
// 						obj.position.y + tilesize / 2 + 1
// 					);

// 					// look for an inactive slot and overwrite the bullet
// 					bool foundInactive = false;
// 					for (int i = 0; i < gs.bullets.size() && !foundInactive; i++)
// 					{
// 						if (gs.bullets[i].data.bullet.state == BulletState::inactive)
// 						{
// 							foundInactive = true;
// 							gs.bullets[i] = bullet;
// 						}
// 					}
// 					// if no inactive slot was found, push a new bullet
// 					if (!foundInactive)
// 					{
// 						gs.bullets.push_back(bullet);
// 					}
// 				}
// 			}
// 			else
// 			{
// 				obj.texture = tex;
// 				obj.currentAnimation = animIndex;
// 			}
// 		};

//         switch (obj.data.player.state)
// 		{
// 			case PlayerState::idle:
// 			{
// 				// switching to running state
// 				if (currentDirection)
// 				{
// 					obj.data.player.state = PlayerState::running;
// 				}
// 				else
// 				{
// 					// decelerate
// 					if (obj.velocity.x)
// 					{
// 						const float factor = obj.velocity.x > 0 ? -1.5f : 1.5f;
// 						float amount = factor * obj.accelaration.x * deltat;
// 						if (std::abs(obj.velocity.x) < std::abs(amount))
// 						{
// 							obj.velocity.x = 0;
// 						}
// 						else
// 						{
// 							obj.velocity.x += amount;
// 						}
// 					}
// 				}
// 				handleShooting(res.texIdle, res.texShoot, res.ANIM_PLAYER_IDLE, res.ANIM_PLAYER_SHOOT);
// 				break;
// 			}
// 			case PlayerState::running:
// 			{
// 				// switching to idle state
// 				if (!currentDirection)
// 				{
// 					obj.data.player.state = PlayerState::idle;
// 				}

// 				// moving in opposite direction of velocity, sliding!
// 				if (obj.velocity.x * obj.direction < 0 && obj.grounded)
// 				{
// 					handleShooting(res.texslide, res.texSlideShoot, res.ANIM_PLAYER_SLIDE, res.ANIM_PLAYER_SLIDE_SHOOT);
// 				}
// 				else
// 				{
// 					handleShooting(res.texRun, res.texRunShoot, res.ANIM_PLAYER_RUN, res.ANIM_PLAYER_RUN);
// 				}
// 				break;
// 			}
// 			case PlayerState::jumping:
// 			{
// 				handleShooting(res.texRun, res.texRunShoot, res.ANIM_PLAYER_RUN, res.ANIM_PLAYER_RUN);
// 				break;
// 			}
// 		}
        

//     }
//     obj.position +=obj.velocity*deltat;

//     bool foundGround = false;
//     for(auto &layer:gs.layers)
//     {
//         for(GameObject &objb:layer)
//         {
//             if(&obj != &objb)
//             {
//                 checkColllison(state,gs,res,obj,objb,deltat);
//                 if (obj.type == ObjectType::level)
//                 {
//                     SDL_FRect sensor{
//                         .x=obj.position.x+obj.collider.x+8,
//                         .y=obj.position.y+obj.collider.y+obj.collider.h,
//                         .w=28, .h=2
//                     };
//                     SDL_FRect rectB{
//                         .x=objb.position.x + objb.collider.x,
//                         .y = objb.position.y+ objb.collider.y,
//                         .w=objb.collider.w, .h=obj.collider.h
//                     };
//                     SDL_FRect rectC{0};
//                     if(SDL_GetRectIntersectionFloat(&sensor,&rectB,&rectC))
//                     {
//                         foundGround = true;
//                     }
//                 }
//             }
//         }
//         if(obj.type == ObjectType::player)
//         {
//             obj.grounded = foundGround;
//             // If the player is in the air and has now landed, switch to idle/running
//             if (foundGround && obj.data.player.state == PlayerState::jumping)
//             {
//                 if (state.keys[SDL_SCANCODE_A] || state.keys[SDL_SCANCODE_D]) {
//                     obj.data.player.state = PlayerState::running;
//                 } else {
//                     obj.data.player.state = PlayerState::idle;
//                 }
//             }
//         }
//     }    
// };
void update(const SDLState &state, gamestate &gs, Resources &res, GameObject &obj, float deltaTime)
{
	// update the animation
	if (obj.currentAnimation != -1)
	{
		obj.animations[obj.currentAnimation].step(deltaTime);
	}

	if (obj.dynamic && !obj.grounded)
	{
		// apply some gravity
		obj.velocity += glm::vec2(0, 500) * deltaTime;
	}

	float currentDirection = 0;
	if (obj.type == ObjectType::player)
	{
		if (state.keys[SDL_SCANCODE_A])
		{
			currentDirection += -1;
		}
		if (state.keys[SDL_SCANCODE_D])
		{
			currentDirection += 1;
		}
		timer &weaponTimer = obj.data.player.weaponTimer;
		weaponTimer.step(deltaTime);

		const auto handleShooting = [&state, &gs, &res, &obj, &weaponTimer](
			SDL_Texture *tex, SDL_Texture *shootTex, int animIndex, int shootAnimIndex)
		{
			if (state.keys[SDL_SCANCODE_J])
			{
				// set shooting tex/anim
				obj.texture = shootTex;
				obj.currentAnimation = shootAnimIndex;

				if (weaponTimer.isTimeout())
				{
					weaponTimer.reset();
					// spawn some bullets
					GameObject bullet;
					bullet.data.bullet = Bulletdata();
					bullet.type = ObjectType::bullet;
					bullet.direction = gs.player().direction;
					bullet.texture = res.texbullet;
					bullet.currentAnimation = res.ANIM_BULLET_MOVING;
					bullet.collider = SDL_FRect{
						.x = 0, .y = 0,
						.w = static_cast<float>(res.texbullet->h),
						.h = static_cast<float>(res.texbullet->h)
					};
					const int yVariation = 40;
					const float yVelocity = SDL_rand(yVariation) - yVariation / 2.0f;
					bullet.velocity = glm::vec2(
						obj.velocity.x + 600.0f * obj.direction,
						yVelocity
					);
					bullet.maxSpeedX = 1000.0f;
					bullet.animations = res.bulletAnims;

					// adjust bullet start position
					const float left = 4;
					const float right = 24;
					const float t = (obj.direction + 1) / 2.0f; // results in a value of 0..1
					const float xOffset = left + right * t; // LERP between left and right based on direction
					bullet.position = glm::vec2(
						obj.position.x + xOffset,
						obj.position.y + tilesize / 2 + 1
					);

					// look for an inactive slot and overwrite the bullet
					bool foundInactive = false;
					for (int i = 0; i < gs.bullets.size() && !foundInactive; i++)
					{
						if (gs.bullets[i].data.bullet.state == BulletState::inactive)
						{
							foundInactive = true;
							gs.bullets[i] = bullet;
						}
					}
					// if no inactive slot was found, push a new bullet
					if (!foundInactive)
					{
						gs.bullets.push_back(bullet);
					}

				}
			}
			else
			{
				obj.texture = tex;
				obj.currentAnimation = animIndex;
			}
		};

		switch (obj.data.player.state)
		{
			case PlayerState::idle:
			{
				// switching to running state
				if (currentDirection)
				{
					obj.data.player.state = PlayerState::running;
				}
				else
				{
					// decelerate
					if (obj.velocity.x)
					{
						const float factor = obj.velocity.x > 0 ? -1.5f : 1.5f;
						float amount = factor * obj.accelaration.x * deltaTime;
						if (std::abs(obj.velocity.x) < std::abs(amount))
						{
							obj.velocity.x = 0;
						}
						else
						{
							obj.velocity.x += amount;
						}
					}
				}
				handleShooting(res.texIdle, res.texShoot, res.ANIM_PLAYER_IDLE, res.ANIM_PLAYER_SHOOT);
				break;
			}
			case PlayerState::running:
			{
				// switching to idle state
				if (!currentDirection)
				{
					obj.data.player.state = PlayerState::idle;
				}

				// moving in opposite direction of velocity, sliding!
				if (obj.velocity.x * obj.direction < 0 && obj.grounded)
				{
					handleShooting(res.texSlide, res.texSlideShoot, res.ANIM_PLAYER_SLIDE, res.ANIM_PLAYER_SLIDE_SHOOT);
				}
				else
				{
					handleShooting(res.texRun, res.texRunShoot, res.ANIM_PLAYER_RUN, res.ANIM_PLAYER_RUN);
				}
				break;
			}
			case PlayerState::jumping:
			{
				handleShooting(res.texRun, res.texRunShoot, res.ANIM_PLAYER_RUN, res.ANIM_PLAYER_RUN);
				break;
			}
		}
	}
	else if (obj.type == ObjectType::bullet)
	{
		switch (obj.data.bullet.state)
		{
			case BulletState::moving:
			{
				if (obj.position.x - gs.mapViewport.x < 0 || // left edge
					obj.position.x - gs.mapViewport.x > state.logw|| // right edge
					obj.position.y - gs.mapViewport.y < 0 || // top edge
					obj.position.y - gs.mapViewport.y > state.logh) // bottom edge
				{
					obj.data.bullet.state = BulletState::inactive;
				}
				break;
			}
			// case BulletState::colliding:
			// {
			// 	if (obj.animations[obj.currentAnimation].isDone())
			// 	{
			// 		obj.data.bullet.state = BulletState::inactive;
			// 	}
			// 	break;x
			// }
		}
	}

	if (currentDirection)
	{
		obj.direction = currentDirection;
	}
	// add acceleration to velocity
	obj.velocity += currentDirection * obj.accelaration * deltaTime;
	if (std::abs(obj.velocity.x) > obj.maxSpeedX)
	{
		obj.velocity.x = currentDirection * obj.maxSpeedX;
	}

	// add velocity to position
	obj.position += obj.velocity * deltaTime;

	// handle collision detection
	bool wasGrounded = obj.grounded;
	obj.grounded = false;
	for (auto &layer : gs.layers)
	{
		for (GameObject &objB : layer)
		{
			if (&obj != &objB)
			{
				checkColllison(state, gs, res, obj, objB, deltaTime);
			}
		}
	}
	// collision response updates obj.grounded to new state
	if (obj.grounded && !wasGrounded)
	{
		if (obj.grounded && obj.type == ObjectType::player)
		{
			obj.data.player.state = PlayerState::running;
		}
	}
}
void collisionResponse(const SDLState &state, gamestate &gs, const Resources &res,SDL_FRect &rectA,SDL_FRect &rectB, SDL_FRect &rectC ,GameObject &obja, GameObject &objb, float deltat)
{
    if(obja.type==ObjectType::player)
    {
        switch(objb.type)
        {
            case ObjectType::level:
            {
                if(rectC.w<rectC.h)
                {
                    if(obja.velocity.x>0){
                        obja.position.x -= rectC.w;}
                    else if(obja.velocity.x<0)
                    {
                        obja.position.x += rectC.w + 0.1f;
                    }
                    obja.velocity.x = 0;
                }
                else
                    if(obja.velocity.y>0){
                        obja.position.y -= rectC.h;}
                    else if(obja.velocity.y < 0)
                    {
                        obja.position.y += rectC.h + 0.05f;
                    }
                    obja.velocity.y = 0;
                    obja.grounded=true;
                {

                }
                break;
            }
        }
    }
    else if(obja.type == ObjectType::bullet)
    {
        switch(obja.data.bullet.state)
        {
            case BulletState::moving:
            {
                obja.velocity*=0;
                break;
            }
        }
    }
};

void checkColllison(const SDLState &state, gamestate &gs, const Resources &res,GameObject &a, GameObject &b, float deltat)
{
    SDL_FRect rectA{
        .x= a.position.x + a.collider.x,
        .y= a.position.y + a.collider.y,
        .w=a.collider.w, .h=a.collider.h
    };

    SDL_FRect rectB{
        .x= b.position.x + b.collider.x,
        .y= b.position.y + b.collider.y,
        .w=b.collider.w, .h=b.collider.h
    };

    SDL_FRect rectC{ 0 };

    if(SDL_GetRectIntersectionFloat(&rectA , &rectB, &rectC))
    {
        collisionResponse(state,gs,res ,rectA,rectB,rectC,a,b,deltat);
    }
};


void createTiles(const SDLState &state, gamestate &gs, const Resources &res )
{
    /*
    1.ground
    2.pannel
    3.enemy
    4.player
    5.grass
    6.brick
    */
    short map[maprows][mapcols] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 4, 0, 0, 0, 0, 0, 2, 2, 0, 3, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 3, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 2, 0, 0, 2, 2, 2, 2, 0, 2, 2, 2, 0, 0, 3, 2, 2, 2, 2, 0, 0, 2, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 3, 0, 0, 3, 0, 2, 2, 2, 2, 2, 0, 0, 2, 2, 0, 3, 0, 0, 3, 0, 2, 3, 3, 3, 0, 2, 0, 3, 3, 0, 0, 3, 0, 3, 0, 3, 0, 0, 0, 3,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};
    
    short background[maprows][mapcols] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};
	short foreground[maprows][mapcols] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

    const auto loadMap= [&state,&gs,&res](short layer[maprows][mapcols])
    {
        const auto createObject= [&state](int r, int c, SDL_Texture *tex, ObjectType type)
        {
            GameObject o;
            o.type= type;
            o.position =glm::vec2(c*tilesize, state.logh-(maprows-r)*tilesize );
            o.texture=tex;
            o.collider = {.x=0,.y=0,.w=tilesize,.h=tilesize};
            return o;
        };

        for(int r=0; r < maprows;r++)
        {
            for(int c=0;c < mapcols;c++)
            {
                switch(layer[r][c])
                {
                    case(1):    //ground
                    {
                        GameObject o=createObject(r,c, res.texground,ObjectType::level);
                        gs.layers[LAYER_INDEX_LEVEL].push_back(o);
                        break;
                    }
                    case(2):    //panel
                    {
                        GameObject o=createObject(r,c, res.texpanel ,ObjectType::level);
                        gs.layers[LAYER_INDEX_LEVEL].push_back(o);
                        break;
                    }
                    case(3):    //enemy
                    {
                        break;
                    }
                    case(4):    //player
                    {
                        GameObject player=createObject(r,c, res.texIdle, ObjectType::player);
                        player.data.player=PlayerData();
                        player.animations = res.playerAnims;
                        player.accelaration=glm::vec2(1500,0);
                        player.maxSpeedX= 550;
                        player.dynamic=true;
                        player.currentAnimation = res.ANIM_PLAYER_IDLE;
                        player.collider ={
                            .x = 11, .y = 6,
                            .w = 10, .h = 26
                        };

                        gs.layers[LAYER_INDEX_CHARACTERS].push_back(player);
                        gs.palyerIndex=gs.layers[LAYER_INDEX_CHARACTERS].size()-1;
                        break;
                    }
                    case(5):    //grass
                    {   
                        GameObject o =createObject(r,c,res.texgrass,ObjectType::level);
                        gs.foregroundtiles.push_back(o);
                        break;
                    }
                    case(6):    //brick
                    {
                        GameObject o =createObject(r,c,res.texbrick,ObjectType::level);
                        gs.foregroundtiles.push_back(o);
                        break;
                    }

                }
            }
        }
    };
    loadMap(map);
	loadMap(background);
	loadMap(foreground);
	assert(gs.palyerIndex!= -1);
}

void handleKeyInput(const SDLState &state, gamestate &gs, GameObject &obj, SDL_Scancode key, bool keyDown)
{
    // This is the jump force you were using in your update function
    const float JUMP_Force =  -200.0f; 
    
    // This lambda contains the direct-action logic
    const auto jump = [&]()
	{
        // We only act on the KEY DOWN event, and only if grounded
		if (key == SDL_SCANCODE_SPACE && keyDown && obj.grounded) 
        {
            obj.data.player.state = PlayerState::jumping;
            obj.velocity.y = JUMP_Force; // Apply jump force directly
            obj.grounded = false;       // The player is now airborne
        }
	};

    // We only check for player input
    if(obj.type == ObjectType::player)
    {
        switch (obj.data.player.state)
        {
            case PlayerState::idle:
            {
                jump(); 
                break;
            }
            case PlayerState::running:
            {
                jump();
                break;
            }
            case PlayerState::jumping:
            {
                break;
            }
        }
    }
}

void drawParalaxBackground(SDL_Renderer *renderer, SDL_Texture *texture, float xVelocity, float& scrollpos,float scrollfact, float deltat,float offset)
{
    scrollpos -= scrollfact* xVelocity* deltat;

    if(scrollpos<= -texture->w)
    {
        scrollpos=0;
    }

    SDL_FRect dst{
        .x= scrollpos,.y=offset,
        .w = texture->w*2.0f,
        .h= static_cast<float>(texture->h)
    };
    SDL_RenderTexture(renderer,texture,nullptr,&dst);
};

void cleanup(SDLState &state)
{
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}