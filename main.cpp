#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT   600
#define MAX_HEALTH 3
#define TRANSPARENT 0x00000000
#define IMG_NOT_LOADED 1
#define SHOOT_COOLDOWN 1
#define ENEMY_SHOOT_COOLDOWN 1
#define PLAYER_SPEED 220
#define MAP_MAX_RIGHT -300
#define MAP_MAX_LEFT 1100
#define MAP_MAX_UP 795
#define MAP_MAX_DOWN -185
#define IMMORTAL_TIME 1
#define MENU 0
#define GAME_OVER 1
#define LEVEL_ONE 2
#define LEVEL_TWO 3
#define LEVEL_THREE 4
#define BULLET_SPEED 500
#define E 1
#define W 2
#define SPECIAL_ATTACK_COOLDOWN 5
#define PI 3,14
#define NUMBER_OF_ORBITAL_BULLETS 8
#define ENEMY_SPEED 100
#define ENEMY_BULLET_SPEED_A 250
#define ENEMY_BULLET_SPEED_B 180
#define ANIMATION_LENGTH 0.15
#define MAX_BULLET_SPEED 150
#define BASIC 1
#define SPECIAL 2


typedef struct
{
	SDL_Surface* screen;
	SDL_Surface* charset;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	char text[128];
}Display;

typedef struct
{
	SDL_Surface* graphic;
	int health;
	float posX;
	float posY;
	float speedX;
	float speedY;
	int direction;
	int error;
	float shootCooldown;
	float immortal;
	int shootReady;
	int currentAttack;
	float time;
	float specialAttack;
	int animation;
	float animationTimer;
}object;


typedef struct
{
	int black;
	int green;
	int red;
	int blue;
	int white;
	int blank;
	int aqua;
	int yellow;
}colorPalette;

typedef struct
{
	double delta;
	double worldTime;
	double fpsTimer;
	double fps;
	int frames;
	int t1;
	int t2;
	int quit;
	int numberOfEnemyBullets;
	int nPlayerBullet;
	int enemySpecial;
}parameters;

//basic functionalities
void timerUpdate(parameters& parameters);
void fpsUpdate(parameters& parameters);
void endPrograme(Display visualization);
int Game(Display visualization, parameters& parameters, colorPalette colors, int level);
void handleMove(parameters parameters, object& player, object& map, object& enemy, object bullets[], object& playerBullet);
void enemyShootBasic(object bullets[], object& enemy, parameters& parameters, Display visualization);
void hitDetection(object& player, object bullets[], parameters& parameters, object& enemy, object playerBullet);
void playerShoot(object& player, object& playerBullet, parameters& parameters, Display visualization);
void playerAnimation(object& player, parameters parameters);
void playerShootCooldownTimer(object& player, parameters parameters);
void orbitalBulletMovement(object enemy, object orbitalBullet[], parameters parameters);
void orbitalBulletHitDetection(object& player, object bullets[]);
void enemyMovement(object& enemy, object player, parameters parameters);
void enemyAnimation(object& enemy, parameters parameters);
void homingShotMovement(object player, object bullet[], parameters parameters);
void enemyHomingShot(object& enemy, object bullets[], parameters& parameters, Display visualization, int type);

//initialization
object etiInit(Display visualization);
colorPalette colorInit(SDL_Surface* screen);
parameters parametersInit();
object playerInit(Display visualization);
object mapInit(Display visualization, int level);
object enemyInit(Display visualization, int level);
object bulletInit(Display visualization, object enemy, int size);

//elemental printing
void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset);
void DrawSurface(SDL_Surface* screen, object object);
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color);
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color);
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor);

//complex printing
void printMenu(Display visualization, colorPalette colors, parameters parameters, object eti);
void printGame(Display visualization, object player, colorPalette colors, parameters parameters, object map, object enemy, object bullets[], object playerBullet);
void printHealth(Display visualization, object player);
void printGameOver(Display visualization, colorPalette colors, parameters parameters, object eti);
void printOrbitalBullets(object orbitalBullet[], Display visualization);

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int rc;
	SDL_Event event;
	colorPalette colors;
	Display visualization;
	parameters parameters;
	object eti;
	int level=MENU;

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) 
	{
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	//rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &visualization.window, &visualization.renderer);  // tryb pe³noekranowy / fullscreen mode

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,&visualization.window, &visualization.renderer);

	if(rc != 0)
	{
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(visualization.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(visualization.renderer, 0, 0, 0, 255);
	SDL_SetWindowTitle(visualization.window, "Szablon do zdania drugiego 2017");
	visualization.screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	visualization.scrtex = SDL_CreateTexture(visualization.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_ShowCursor(SDL_DISABLE); // wy³¹czenie widocznoœci kursora myszy
	colors = colorInit(visualization.screen);
	visualization.charset = SDL_LoadBMP("./cs8x8.bmp"); // wczytanie obrazka cs8x8.bmp
	if(visualization.charset == NULL) 
	{
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		endPrograme(visualization);
		return IMG_NOT_LOADED;
	};

    SDL_SetColorKey(visualization.charset, true, TRANSPARENT);
	parameters = parametersInit();
	eti = etiInit(visualization);
	if (eti.error == IMG_NOT_LOADED) return IMG_NOT_LOADED;
	

	while(!parameters.quit) 
	{
		timerUpdate(parameters);
		eti.posX += eti.speedX * parameters.delta;
		if (eti.posX >= SCREEN_WIDTH && eti.speedX > 0 || eti.posX<=0 && eti.speedX < 0) eti.speedX=-eti.speedX;

		SDL_FillRect(visualization.screen, NULL, colors.black);

		fpsUpdate(parameters);
		
		if (level == MENU)printMenu(visualization, colors, parameters, eti);
		else if (level == GAME_OVER) printGameOver(visualization, colors, parameters, eti);
		else if (level == LEVEL_ONE) { parameters.worldTime = 0; level = Game(visualization, parameters, colors, LEVEL_ONE); }
		else if (level == LEVEL_TWO) level = Game(visualization, parameters, colors, LEVEL_TWO);
		else if (level == LEVEL_THREE) level = Game(visualization, parameters, colors, LEVEL_THREE);

		SDL_UpdateTexture(visualization.scrtex, NULL, visualization.screen->pixels, visualization.screen->pitch);
		//SDL_RenderClear(visualization.renderer);
		SDL_RenderCopy(visualization.renderer, visualization.scrtex, NULL, NULL);
		SDL_RenderPresent(visualization.renderer);

		// obs³uga zdarzeñ (o ile jakieœ zasz³y) / handling of events (if there were any)
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) parameters.quit = 1;
					else if (event.key.keysym.sym == SDLK_UP) eti.speedX = eti.speedX * 2;
					else if (event.key.keysym.sym == SDLK_DOWN) eti.speedX = eti.speedX / 2;
					else if (event.key.keysym.sym == SDLK_n) level = LEVEL_THREE;
					else if (event.key.keysym.sym == SDLK_TAB);
					break;
				case SDL_KEYUP:
					break;
				case SDL_QUIT:
					parameters.quit = 1;
					break;
				};
			};
		parameters.frames++;
	};
	endPrograme(visualization);// zwolnienie powierzchni / freeing all surfaces
	return 0;
	};



int Game(Display visualization, parameters& parameters, colorPalette colors,int level)
{
	object player, map, enemy;
	object bullets[17]; //last array place is reserved for special attacks
	object orbitalBullet[NUMBER_OF_ORBITAL_BULLETS];
	object playerBullet;
	SDL_Event event;
	player = playerInit(visualization);
	map = mapInit(visualization, level);
	enemy = enemyInit(visualization, level);
	if (level == LEVEL_TWO) for (int i = 0; i < NUMBER_OF_ORBITAL_BULLETS; i++) orbitalBullet[i] = bulletInit(visualization, enemy, 1);
	parameters.nPlayerBullet = 0;
	parameters.numberOfEnemyBullets = 0;
	parameters.enemySpecial = 0;
	playerBullet.error = 0;
	while (!parameters.quit)
	{
		timerUpdate(parameters);
		fpsUpdate(parameters);
		SDL_FillRect(visualization.screen, NULL, colors.black);

		playerShootCooldownTimer(player, parameters);
		playerAnimation(player, parameters);
		handleMove(parameters, player, map, enemy, bullets, playerBullet);
		if(level==LEVEL_ONE)enemyShootBasic(bullets, enemy, parameters, visualization);
		hitDetection(player, bullets, parameters, enemy, playerBullet);
		

		if (player.health == 0){ return GAME_OVER; }
		else if (enemy.health == 0) {return level + 1; }
		printGame(visualization, player, colors, parameters, map, enemy, bullets, playerBullet);
		if (level == LEVEL_TWO)
		{
			enemyMovement(enemy, player, parameters);
			enemyAnimation(enemy, parameters);
			orbitalBulletMovement(enemy, orbitalBullet, parameters);
			if (player.immortal == 0) orbitalBulletHitDetection(player, orbitalBullet);
			printOrbitalBullets(orbitalBullet, visualization);
		}
		else if (level == LEVEL_THREE)
		{
			enemyHomingShot(enemy, bullets, parameters, visualization, BASIC);
			enemyHomingShot(enemy, bullets, parameters, visualization, SPECIAL);
			homingShotMovement(player, bullets, parameters);
			
		}
		DrawRectangle(visualization.screen, 0, SCREEN_HEIGHT - 50, SCREEN_WIDTH, 50, colors.yellow, colors.aqua);
		sprintf(visualization.text, "Time : %.1lf s      %.0lf FPS", parameters.worldTime, parameters.fps);
		DrawString(visualization.screen, visualization.screen->w / 4 * 3 - strlen(visualization.text) * 5, SCREEN_HEIGHT - 30, visualization.text, visualization.charset);
		printHealth(visualization, player);

		SDL_UpdateTexture(visualization.scrtex, NULL, visualization.screen->pixels, visualization.screen->pitch);
		//SDL_RenderClear(visualization.renderer);
		SDL_RenderCopy(visualization.renderer, visualization.scrtex, NULL, NULL);
		SDL_RenderPresent(visualization.renderer);

		// obs³uga zdarzeñ (o ile jakieœ zasz³y) / handling of events (if there were any)
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) return MENU;
				else if (event.key.keysym.sym == SDLK_UP)    player.speedY = -PLAYER_SPEED;
				else if (event.key.keysym.sym == SDLK_DOWN)  player.speedY = PLAYER_SPEED;
				else if (event.key.keysym.sym == SDLK_LEFT)  player.speedX = -PLAYER_SPEED;
				else if (event.key.keysym.sym == SDLK_RIGHT) player.speedX= PLAYER_SPEED;
				else if (event.key.keysym.sym == SDLK_SPACE) playerShoot(player, playerBullet, parameters, visualization);
				break;
			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_UP)        { if (player.speedY == -PLAYER_SPEED)player.speedY = 0; }
				else if (event.key.keysym.sym == SDLK_DOWN) { if (player.speedY == PLAYER_SPEED) player.speedY = 0; }
				else if (event.key.keysym.sym == SDLK_LEFT) { if (player.speedX == -PLAYER_SPEED)player.speedX = 0; }
				else if (event.key.keysym.sym == SDLK_RIGHT){ if (player.speedX == PLAYER_SPEED) player.speedX = 0; }
				break;
			case SDL_QUIT:
				parameters.quit = 1;
				break;
			};
		};
		parameters.frames++;
	};
}


void homingShotMovement(object player, object bullet[], parameters parameters)
{
	for (int i = 0; i < parameters.numberOfEnemyBullets; i++)
	{
		if (player.posX > bullet[i].posX && bullet[i].speedX < MAX_BULLET_SPEED) bullet[i].speedX = bullet[i].speedX + (MAX_BULLET_SPEED * parameters.delta / 2);
		else if (bullet[i].speedX > -MAX_BULLET_SPEED) bullet[i].speedX += MAX_BULLET_SPEED * parameters.delta / -2;
		if (player.posY > bullet[i].posY && bullet[i].speedY < MAX_BULLET_SPEED) bullet[i].speedY += MAX_BULLET_SPEED * parameters.delta / 2;
		else if(bullet[i].speedY > -MAX_BULLET_SPEED) bullet[i].speedY += MAX_BULLET_SPEED * parameters.delta / -2;

		bullet[i].posX += parameters.delta * bullet[i].speedX;
		bullet[i].posY += parameters.delta * bullet[i].speedY;
	}
	if (player.posX > bullet[16].posX && bullet[16].speedX < MAX_BULLET_SPEED) bullet[16].speedX = bullet[16].speedX + (MAX_BULLET_SPEED * parameters.delta);
	else if (bullet[16].speedX > -MAX_BULLET_SPEED) bullet[16].speedX += MAX_BULLET_SPEED * parameters.delta * -1;
	if (player.posY > bullet[16].posY && bullet[16].speedY < MAX_BULLET_SPEED) bullet[16].speedY += MAX_BULLET_SPEED * parameters.delta;
	else if (bullet[16].speedY > -MAX_BULLET_SPEED) bullet[16].speedY += MAX_BULLET_SPEED * parameters.delta * -1;

	bullet[16].posX += parameters.delta * bullet[16].speedX;
	bullet[16].posY += parameters.delta * bullet[16].speedY;

}

void enemyHomingShot(object& enemy, object bullets[], parameters& parameters, Display visualization, int type)
{
	if (type == BASIC)
	{
		if (enemy.shootCooldown <= 0)
		{
			enemy.shootReady = 1;
		}
		if (enemy.shootReady == 0)
		{
			enemy.shootCooldown = enemy.shootCooldown - parameters.delta;
		}
		else
		{
			int help = enemy.currentAttack;
			bullets[help] = bulletInit(visualization, enemy, 1);
			if (enemy.currentAttack < 15) enemy.currentAttack++;
			else if (enemy.currentAttack == 15) enemy.currentAttack = 0;
			if (parameters.numberOfEnemyBullets < 16)parameters.numberOfEnemyBullets++;
			enemy.shootReady = 0;
			enemy.shootCooldown = ENEMY_SHOOT_COOLDOWN;
		}
	}
	else if (type == SPECIAL)
	{
		if (enemy.specialAttack <= 0)
		{
			bullets[16] = bulletInit(visualization, enemy, SPECIAL);
			enemy.specialAttack = SPECIAL_ATTACK_COOLDOWN;
			parameters.enemySpecial = 1;
		}
		else enemy.specialAttack = enemy.specialAttack - parameters.delta;
	}
		
		
}

void enemyAnimation(object& enemy, parameters parameters)
{
	if (enemy.animationTimer <= 0)
	{
		if (enemy.animation == 0)
		{
			enemy.graphic = SDL_LoadBMP("./FROG1.bmp");
			enemy.animation = 1;
		}
		else if (enemy.animation == 1)
		{
			enemy.graphic = SDL_LoadBMP("./FROG2.bmp");
			enemy.animation = 2;
		}
		else if (enemy.animation == 2)
		{
			enemy.graphic = SDL_LoadBMP("./FROG3.bmp");
			enemy.animation = 3;
		}
		else if (enemy.animation == 3)
		{
			enemy.graphic = SDL_LoadBMP("./FROG4.bmp");
			enemy.animation = 0;
		}
		enemy.animationTimer = ANIMATION_LENGTH;
	}
	else
	{
		enemy.animationTimer = enemy.animationTimer - parameters.delta;
	}
}

void enemyMovement(object& enemy, object player, parameters parameters)
{
	if (player.posX > enemy.posX) enemy.posX += ENEMY_SPEED * parameters.delta;
	else enemy.posX += ENEMY_SPEED * parameters.delta * (-1);
	if (player.posY > enemy.posY) enemy.posY += ENEMY_SPEED * parameters.delta;
	else enemy.posY += ENEMY_SPEED * parameters.delta * (-1);
}

void orbitalBulletHitDetection(object& player, object bullets[])
{
	for (int i = 0; i < NUMBER_OF_ORBITAL_BULLETS; i++)
	{
		if (player.posX > bullets[i].posX - 30 && player.posX<bullets[i].posX + 30 && player.posY>bullets[i].posY - 32 && player.posY < bullets[i].posY + 32 && player.immortal == 0)
		{
			player.health--;
			player.immortal = IMMORTAL_TIME;
		}
	}
}

void printOrbitalBullets(object orbitalBullet[], Display visualization)
{
	for (int i = 0; i < NUMBER_OF_ORBITAL_BULLETS; i++)
	DrawSurface(visualization.screen, orbitalBullet[i]);
}

void orbitalBulletMovement(object enemy, object orbitalBullet[], parameters parameters)
{
	float trygonometry1 = (parameters.worldTime * 4) + (PI);
	float trygonometry2 = (parameters.worldTime) + (PI);
	
	orbitalBullet[0].posX = enemy.posX + sin(parameters.worldTime*4) * 100;    orbitalBullet[0].posY = enemy.posY + cos(parameters.worldTime*4) * 100;
	orbitalBullet[1].posX = enemy.posX + sin(parameters.worldTime*4) * -100;   orbitalBullet[1].posY = enemy.posY + cos(parameters.worldTime*4) * -100;
	orbitalBullet[2].posX = enemy.posX + sin(trygonometry1) * -100;            orbitalBullet[2].posY = enemy.posY + cos(trygonometry1) * -100;
	orbitalBullet[3].posX = enemy.posX + sin(trygonometry1) * 100;             orbitalBullet[3].posY = enemy.posY + cos(trygonometry1) * 100;
	orbitalBullet[4].posX = enemy.posX + sin(parameters.worldTime) * 300;      orbitalBullet[4].posY = enemy.posY + cos(parameters.worldTime) * 300;
	orbitalBullet[5].posX = enemy.posX + sin(parameters.worldTime) * -300;     orbitalBullet[5].posY = enemy.posY + cos(parameters.worldTime) * -300;
	orbitalBullet[6].posX = enemy.posX + sin(trygonometry2) * -300;            orbitalBullet[6].posY = enemy.posY + cos(trygonometry2) * -300;
	orbitalBullet[7].posX = enemy.posX + sin(trygonometry2) * 300;             orbitalBullet[7].posY = enemy.posY + cos(trygonometry2) * 300;
}

void playerShootCooldownTimer(object& player, parameters parameters)
{
	if (player.shootCooldown <= 0)
	{
		player.shootReady = 1;
	}
	if (player.shootReady == 0)
	{
		player.shootCooldown = player.shootCooldown - parameters.delta;
	}
}

void playerAnimation(object& player, parameters parameters)
{
	if (player.speedX < 0 || (player.speedY!=0 && player.direction == W && player.speedX==0))
	{
		if (player.direction == E)
		{
			player.direction = W;
			player.animationTimer = 0;
		}
		if (player.animationTimer <= 0)
		{
			if (player.animation == 0)
			{
				player.graphic = SDL_LoadBMP("./playerLEFTMOVE1.bmp");
				player.animation = 1;
			}
			else if (player.animation == 1)
			{
				player.graphic = SDL_LoadBMP("./playerLEFTMOVE2.bmp");
				player.animation = 0;
			}
			player.animationTimer = ANIMATION_LENGTH;
		}
		else player.animationTimer = player.animationTimer - parameters.delta;
	}
	else if (player.speedX > 0 || (player.speedY != 0 && player.direction == E && player.speedX == 0))
	{
		if (player.direction == W)
		{
			player.direction = E;
			player.animationTimer = 0;
		}
		if (player.animationTimer <= 0)
		{
			if (player.animation == 0)
			{
				player.graphic = SDL_LoadBMP("./playerRIGHTMOVE1.bmp");
				player.animation = 1;
			}
			else if (player.animation == 1)
			{
				player.graphic = SDL_LoadBMP("./playerRIGHTMOVE2.bmp");
				player.animation = 0;
			}
			player.animationTimer = ANIMATION_LENGTH;
		}
		else player.animationTimer = player.animationTimer - parameters.delta;
	}
	else if (player.speedX == 0 && player.direction == E && player.speedY==0)
	{
		player.graphic = SDL_LoadBMP("./playerRIGHT.bmp");
	}
	else if (player.speedX == 0 && player.direction == W && player.speedY == 0)
	{
		player.graphic = SDL_LoadBMP("./playerLEFT.bmp");
	}
}

void playerShoot(object& player, object& playerBullet, parameters& parameters, Display visualization)
{
	if (player.shootReady == 1) 
	{
		playerBullet = bulletInit(visualization, player, 1);
		if (player.direction == W)       
		{
			playerBullet.speedX = -BULLET_SPEED;
			playerBullet.speedY = 0; 
			playerBullet.graphic = SDL_LoadBMP("./PBLEFT.bmp");
		}
		else if (player.direction == E)  
		{
			playerBullet.speedX = BULLET_SPEED;
			playerBullet.speedY = 0;
			playerBullet.graphic = SDL_LoadBMP("./PBRIGHT.bmp");
		}
		player.shootReady = 0;
		player.shootCooldown = SHOOT_COOLDOWN;
		parameters.nPlayerBullet = 1;
	}
}

void hitDetection(object& player, object bullets[], parameters& parameters, object& enemy, object playerBullet)
{
	if (player.immortal > 0)
	{
		player.immortal = player.immortal - parameters.delta;
		if (player.immortal < 0) player.immortal = 0;
	}

	if (enemy.immortal > 0)
	{
		enemy.immortal = enemy.immortal - parameters.delta;
		if (enemy.immortal < 0) enemy.immortal = 0;
	}

	if (parameters.nPlayerBullet == 1)
	{
		if (enemy.posX > playerBullet.posX - 20 && enemy.posX<playerBullet.posX + 20 && enemy.posY>playerBullet.posY - 32 && enemy.posY < playerBullet.posY + 32 && enemy.immortal == 0)
		{
			enemy.health--;
			enemy.immortal = IMMORTAL_TIME;
			parameters.nPlayerBullet = 0;
		}
	}

	for (int i = 0; i < parameters.numberOfEnemyBullets; i++)
	{
		if (player.posX > bullets[i].posX - 30 && player.posX<bullets[i].posX + 30 && player.posY>bullets[i].posY - 32 && player.posY < bullets[i].posY + 32 && player.immortal==0)
		{
			player.health--;
			player.immortal = IMMORTAL_TIME;
		}
	}
}

void handleMove(parameters parameters, object& player, object& map, object& enemy, object bullets[], object& playerBullet)
{
	for (int i = 0; i < parameters.numberOfEnemyBullets; i++)
	{
		bullets[i].posX += bullets[i].speedX * parameters.delta;
		bullets[i].posY += bullets[i].speedY * parameters.delta;
	}

	if (parameters.nPlayerBullet == 1)
	{
		playerBullet.posX += playerBullet.speedX * parameters.delta;
		playerBullet.posY += playerBullet.speedY * parameters.delta;
	}

	if (parameters.enemySpecial == 1)
	{
		bullets[16].posX += bullets[16].speedX * parameters.delta;
		bullets[16].posY += bullets[16].speedY * parameters.delta;
	}

	if (((player.posX > SCREEN_WIDTH / 3) || (player.posX <= SCREEN_WIDTH/3 && player.speedX > 0)) && ((player.posX < SCREEN_WIDTH / 3 * 2) || (player.posX >= SCREEN_WIDTH/3*2 && player.speedX < 0)))
	{
		player.posX += player.speedX * parameters.delta;
	}
	else if ((map.posX > MAP_MAX_RIGHT && map.posX < MAP_MAX_LEFT) || (map.posX >= MAP_MAX_LEFT && player.speedX > 0) || (map.posX <= MAP_MAX_RIGHT && player.speedX < 0))
	{
		map.posX += player.speedX * parameters.delta * (-1);
		enemy.posX += player.speedX * parameters.delta * (-1);
		for (int i = 0; i < parameters.numberOfEnemyBullets; i++)
		{
			bullets[i].posX += player.speedX * parameters.delta * (-1);
		}
		if (parameters.nPlayerBullet == 1) playerBullet.posX += player.speedX * parameters.delta * (-1);
		if (parameters.enemySpecial == 1) bullets[16].posX += player.speedX * parameters.delta * (-1);
	}

	if (((player.posY > SCREEN_HEIGHT / 3) || (player.posY <= SCREEN_HEIGHT /3 && player.speedY > 0)) && ((player.posY < SCREEN_HEIGHT / 3 * 2) || (player.posY >= SCREEN_HEIGHT /3*2 && player.speedY < 0)))
	{
		player.posY += player.speedY * parameters.delta;
	}
	else if ((map.posY < MAP_MAX_UP && map.posY > MAP_MAX_DOWN) || (map.posY <= MAP_MAX_DOWN && player.speedY < 0) || (map.posY >= MAP_MAX_UP && player.speedY > 0))
	{
		map.posY += player.speedY * parameters.delta * (-1);
		enemy.posY += player.speedY * parameters.delta * (-1);
		for (int i = 0; i < parameters.numberOfEnemyBullets; i++)
		{
			bullets[i].posY += player.speedY * parameters.delta * (-1);
		}
		if (parameters.nPlayerBullet == 1)playerBullet.posY += player.speedY * parameters.delta * (-1);
		if (parameters.enemySpecial == 1) bullets[16].posY += player.speedY * parameters.delta * (-1);
	}
}

void enemyShootBasic(object bullets[], object& enemy, parameters& parameters, Display visualization)
{
	if (enemy.shootCooldown <= 0)
	{
		enemy.shootReady = 1;
	}
	if (enemy.shootReady == 0)
	{
		enemy.shootCooldown = enemy.shootCooldown - parameters.delta;
	}
	else
	{
		if (enemy.currentAttack == 0)
		{
			bullets[0] = bulletInit(visualization, enemy, 1); bullets[0].speedX = ENEMY_BULLET_SPEED_A;  bullets[0].speedY = 0;
			bullets[1] = bulletInit(visualization, enemy, 1); bullets[1].speedX = -ENEMY_BULLET_SPEED_A; bullets[1].speedY = 0;
			bullets[2] = bulletInit(visualization, enemy, 1); bullets[2].speedX = 0;    bullets[2].speedY = -ENEMY_BULLET_SPEED_A;
			bullets[3] = bulletInit(visualization, enemy, 1); bullets[3].speedX = 0;    bullets[3].speedY = ENEMY_BULLET_SPEED_A;
			enemy.currentAttack = 1;
			if (parameters.numberOfEnemyBullets < 16) parameters.numberOfEnemyBullets = 4;
		}
		else if (enemy.currentAttack == 1)
		{
			bullets[4] = bulletInit(visualization, enemy, 1); bullets[4].speedX = ENEMY_BULLET_SPEED_B;  bullets[4].speedY = ENEMY_BULLET_SPEED_B;
			bullets[5] = bulletInit(visualization, enemy, 1); bullets[5].speedX = ENEMY_BULLET_SPEED_B;  bullets[5].speedY = -ENEMY_BULLET_SPEED_B;
			bullets[6] = bulletInit(visualization, enemy, 1); bullets[6].speedX = -ENEMY_BULLET_SPEED_B; bullets[6].speedY = -ENEMY_BULLET_SPEED_B;
			bullets[7] = bulletInit(visualization, enemy, 1); bullets[7].speedX = -ENEMY_BULLET_SPEED_B; bullets[7].speedY = ENEMY_BULLET_SPEED_B;
			enemy.currentAttack = 2;
			if (parameters.numberOfEnemyBullets < 16) parameters.numberOfEnemyBullets = 8;
		}
		else if (enemy.currentAttack == 2)
		{
			bullets[8] = bulletInit(visualization, enemy, 1);  bullets[8].speedX = ENEMY_BULLET_SPEED_A;   bullets[8].speedY = 0;
			bullets[9] = bulletInit(visualization, enemy, 1);  bullets[9].speedX = -ENEMY_BULLET_SPEED_A;  bullets[9].speedY = 0;
			bullets[10] = bulletInit(visualization, enemy, 1); bullets[10].speedX = 0;    bullets[10].speedY = -ENEMY_BULLET_SPEED_A;
			bullets[11] = bulletInit(visualization, enemy, 1); bullets[11].speedX = 0;    bullets[11].speedY = ENEMY_BULLET_SPEED_A;
			bullets[12] = bulletInit(visualization, enemy, 1); bullets[12].speedX = ENEMY_BULLET_SPEED_B;  bullets[12].speedY = ENEMY_BULLET_SPEED_B;
			bullets[13] = bulletInit(visualization, enemy, 1); bullets[13].speedX = ENEMY_BULLET_SPEED_B;  bullets[13].speedY = -ENEMY_BULLET_SPEED_B;
			bullets[14] = bulletInit(visualization, enemy, 1); bullets[14].speedX = -ENEMY_BULLET_SPEED_B; bullets[14].speedY = -ENEMY_BULLET_SPEED_B;
			bullets[15] = bulletInit(visualization, enemy, 1); bullets[15].speedX = -ENEMY_BULLET_SPEED_B; bullets[15].speedY = ENEMY_BULLET_SPEED_B;
			enemy.currentAttack = 0;
			if (parameters.numberOfEnemyBullets < 16) parameters.numberOfEnemyBullets = 16;
		}
		enemy.shootReady = 0;
		enemy.shootCooldown = ENEMY_SHOOT_COOLDOWN;
	}
}

object bulletInit(Display visualization, object character, int size)
{
	object bullet;
	bullet.error = 0;
	if (size==BASIC) bullet.graphic = SDL_LoadBMP("./bullet.bmp");
	else if (size==SPECIAL) bullet.graphic = SDL_LoadBMP("./BIGBULLET.bmp");
	if (bullet.graphic == NULL)
	{
		printf("SDL_LoadBMP(bullet.bmp) error: %s\n", SDL_GetError());
		endPrograme(visualization);
		bullet.error = IMG_NOT_LOADED;
	}
	SDL_SetColorKey(bullet.graphic, true, TRANSPARENT);
	bullet.posX = character.posX;
	bullet.posY = character.posY;
	bullet.speedX = 0;
	bullet.speedY = 0;
	return bullet;
}

void printHealth(Display visualization, object player)
{
	object fullHeart, emptyHeart;
	fullHeart.posY = SCREEN_HEIGHT - 20;
	emptyHeart.posY = SCREEN_HEIGHT - 20;
	fullHeart.graphic = SDL_LoadBMP("./heartfull.bmp");
	SDL_SetColorKey(player.graphic, true, TRANSPARENT);
	emptyHeart.graphic = SDL_LoadBMP("./heartempty.bmp");
	SDL_SetColorKey(player.graphic, true, TRANSPARENT);
	for (int i = 1; i <= MAX_HEALTH; i++)
	{
		if (player.health >= i)
		{
			fullHeart.posX = 35 * i;
			DrawSurface(visualization.screen, fullHeart);
		}
		else
		{
			emptyHeart.posX = 35 * i;
			DrawSurface(visualization.screen, emptyHeart);
		}
	}
}

void printGame(Display visualization, object player, colorPalette colors, parameters parameters, object map, object enemy, object bullets[], object playerBullet)
{
	DrawSurface(visualization.screen, map);
	DrawSurface(visualization.screen, player);
	DrawSurface(visualization.screen, enemy);
	for (int i = 0; i < parameters.numberOfEnemyBullets; i++)
	{
		DrawSurface(visualization.screen, bullets[i]);
	}
	if (parameters.enemySpecial == 1) DrawSurface(visualization.screen, bullets[16]);
	if (parameters.nPlayerBullet == 1) DrawSurface(visualization.screen, playerBullet);
}

object playerInit(Display visualization)
{
	object player;
	player.error = 0;
	player.graphic = SDL_LoadBMP("./playerRIGHT.bmp");
	if (player.graphic == NULL)
	{
		printf("SDL_LoadBMP(playerLEFT.bmp) error: %s\n", SDL_GetError());
		endPrograme(visualization);
		player.error = IMG_NOT_LOADED;
	}
	SDL_SetColorKey(player.graphic, true, TRANSPARENT);
	player.posX = SCREEN_WIDTH/2;
	player.posY = SCREEN_HEIGHT/2;
	player.speedX = 0;
	player.speedY = 0;
	player.shootCooldown = SHOOT_COOLDOWN;
	player.shootReady = 1;
	player.health = MAX_HEALTH;
	player.immortal = 0;
	player.animation = 0;
	player.direction = E;
	return player;
}

object mapInit(Display visualization, int level)
{
	object map;
	map.error = 0;
	if (level==LEVEL_ONE) map.graphic = SDL_LoadBMP("./level1.bmp");
	else if (level==LEVEL_TWO) map.graphic = SDL_LoadBMP("./level2.bmp");
	else if (level==LEVEL_THREE) map.graphic = SDL_LoadBMP("./level3.bmp");
	if (map.graphic == NULL)
	{
		printf("SDL_LoadBMP(level2.bmp) error: %s\n", SDL_GetError());
		endPrograme(visualization);
		map.error = IMG_NOT_LOADED;
	}
	SDL_SetColorKey(map.graphic, true, TRANSPARENT);
	map.posX = MAP_MAX_LEFT;
	map.posY = MAP_MAX_DOWN;
	return map;
}

object enemyInit(Display visualization, int level)
{
	object enemy;
	enemy.error = 0;
	if (level==LEVEL_ONE) enemy.graphic = SDL_LoadBMP("./oponent.bmp");
	else if (level==LEVEL_TWO) enemy.graphic = SDL_LoadBMP("./FROG1.bmp");
	else if (level==LEVEL_THREE) enemy.graphic = SDL_LoadBMP("./BRAIN.bmp");
	if (enemy.graphic == NULL)
	{
		printf("SDL_LoadBMP(enemy) error: %s\n", SDL_GetError());
		endPrograme(visualization);
		enemy.error = IMG_NOT_LOADED;
	}
	SDL_SetColorKey(enemy.graphic, true, TRANSPARENT);
	enemy.shootCooldown = ENEMY_SHOOT_COOLDOWN;
	enemy.shootReady = 1;
	enemy.posX = SCREEN_WIDTH * 3 / 2;
	enemy.posY = -250;
	enemy.immortal = 0;
	enemy.health = MAX_HEALTH;
	enemy.specialAttack = SPECIAL_ATTACK_COOLDOWN;
	enemy.currentAttack = 0;
	enemy.animation = 0;
	enemy.animationTimer = ANIMATION_LENGTH;
	return enemy;
}

void timerUpdate(parameters& parameters)
{
	parameters.t2 = SDL_GetTicks();
	parameters.delta = (parameters.t2 - parameters.t1) * 0.001; // t2-t1 = czas w milisekundach od ostatniej klatki,
	parameters.t1 = parameters.t2;							   // delta to ten sam czas w sekundach
	parameters.worldTime += parameters.delta;
}

object etiInit(Display visualization)
{
	object eti;
	eti.error = 0;
	eti.graphic = SDL_LoadBMP("./eti.bmp");
	if (eti.graphic == NULL)
	{
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		endPrograme(visualization);
		eti.error = IMG_NOT_LOADED;
	};
	eti.posX = 0;
	eti.posY = 400;
	eti.speedX = 300;
	eti.speedY = 0;
	eti.health = NULL;
	return eti;
}

colorPalette colorInit(SDL_Surface* screen)
{
	colorPalette colorSet;
	colorSet.black = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	colorSet.green = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	colorSet.red = SDL_MapRGB(screen->format, 0x88, 0x11, 0x11);
	colorSet.blue = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	colorSet.white = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
	colorSet.yellow = SDL_MapRGB(screen->format, 0xFF, 0xCC, 0X00);
	colorSet.aqua = SDL_MapRGB(screen->format, 0x66, 0x99, 0X99);
	return colorSet;
}

parameters parametersInit()
{
	parameters parameters;
	parameters.delta = 0;
	parameters.worldTime = 0;
	parameters.fpsTimer = 0;
	parameters.fps = 0;
	parameters.frames = 0;
	parameters.t1 = SDL_GetTicks();
	parameters.t2 = 0;
	parameters.quit = 0;
	return parameters;
}

void fpsUpdate(parameters& parameters)
{
	parameters.fpsTimer += parameters.delta;
	if (parameters.fpsTimer > 0.5)
	{
		parameters.fps = parameters.frames * 2;
		parameters.frames = 0;
		parameters.fpsTimer -= 0.5;
	};
}

void printMenu(Display visualization, colorPalette colors, parameters parameters, object eti)
{
	DrawRectangle(visualization.screen, 4, 30, SCREEN_WIDTH - 8, 30, colors.green, colors.blue);
	DrawRectangle(visualization.screen, 4, 150, (SCREEN_WIDTH - 8)/2, 160, colors.green, colors.blue);
	DrawRectangle(visualization.screen, SCREEN_WIDTH-(SCREEN_WIDTH-8)/2, 150, (SCREEN_WIDTH - 8)/2, 160, colors.green, colors.blue);
	sprintf(visualization.text, "GRA BULLETHELL GENRE, czas trwania = %.1lf s  %.0lf FPS", parameters.worldTime, parameters.fps);
	DrawString(visualization.screen, visualization.screen->w / 2 - strlen(visualization.text) * 4, 42, visualization.text, visualization.charset);
	sprintf(visualization.text, "N - Start Game");
	DrawString(visualization.screen, visualization.screen->w / 4 - strlen(visualization.text) * 4, 190, visualization.text, visualization.charset);
	sprintf(visualization.text, "TAB - Scoreboard");
	DrawString(visualization.screen, visualization.screen->w / 4 - strlen(visualization.text) * 4, 230, visualization.text, visualization.charset);
	sprintf(visualization.text, "Esc - Exit");
	DrawString(visualization.screen, visualization.screen->w / 4 - strlen(visualization.text) * 4, 270, visualization.text, visualization.charset);
	sprintf(visualization.text, "Controls:");
	DrawString(visualization.screen, visualization.screen->w / 4*3 - strlen(visualization.text) * 4, 190, visualization.text, visualization.charset);
	sprintf(visualization.text, "Arrows \32 \30 \33 \31  - movement");
	DrawString(visualization.screen, visualization.screen->w / 4*3 - strlen(visualization.text) * 4, 230, visualization.text, visualization.charset);
	sprintf(visualization.text, "Space - shoot");
	DrawString(visualization.screen, visualization.screen->w / 4*3 - strlen(visualization.text) * 4, 270, visualization.text, visualization.charset);
	DrawSurface(visualization.screen, eti);
}

void printGameOver(Display visualization, colorPalette colors, parameters parameters, object eti)
{
	DrawRectangle(visualization.screen, 4, 30, SCREEN_WIDTH - 8, SCREEN_HEIGHT-60, colors.blue, colors.red);
	DrawRectangle(visualization.screen, 4, 150, (SCREEN_WIDTH - 8) / 2, 160, colors.green, colors.blue);
	DrawRectangle(visualization.screen, SCREEN_WIDTH - (SCREEN_WIDTH - 8) / 2, 150, (SCREEN_WIDTH - 8) / 2, 160, colors.green, colors.blue);
	sprintf(visualization.text, "GRA BULLETHELL GENRE, czas trwania = %.1lf s  %.0lf FPS", parameters.worldTime, parameters.fps);
	DrawString(visualization.screen, visualization.screen->w / 2 - strlen(visualization.text) * 4, 42, visualization.text, visualization.charset);
	sprintf(visualization.text, "GAME OVER");
	DrawString(visualization.screen, visualization.screen->w / 2 - strlen(visualization.text) * 4, 70, visualization.text, visualization.charset);
	sprintf(visualization.text, "N - RESTART");
	DrawString(visualization.screen, visualization.screen->w / 4 - strlen(visualization.text) * 4, 190, visualization.text, visualization.charset);
	sprintf(visualization.text, "TAB - Scoreboard");
	DrawString(visualization.screen, visualization.screen->w / 4 - strlen(visualization.text) * 4, 230, visualization.text, visualization.charset);
	sprintf(visualization.text, "Esc - Exit");
	DrawString(visualization.screen, visualization.screen->w / 4 - strlen(visualization.text) * 4, 270, visualization.text, visualization.charset);
	sprintf(visualization.text, "Controls:");
	DrawString(visualization.screen, visualization.screen->w / 4 * 3 - strlen(visualization.text) * 4, 190, visualization.text, visualization.charset);
	sprintf(visualization.text, "Arrows \32 \30 \33 \31  - movement");
	DrawString(visualization.screen, visualization.screen->w / 4 * 3 - strlen(visualization.text) * 4, 230, visualization.text, visualization.charset);
	sprintf(visualization.text, "Space - shoot");
	DrawString(visualization.screen, visualization.screen->w / 4 * 3 - strlen(visualization.text) * 4, 270, visualization.text, visualization.charset);
	DrawSurface(visualization.screen, eti);
}

void endPrograme(Display visualization)
{
	SDL_FreeSurface(visualization.screen);
	SDL_DestroyTexture(visualization.scrtex);
	SDL_DestroyWindow(visualization.window);
	SDL_DestroyRenderer(visualization.renderer);
	SDL_Quit();
}

// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset)
{
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text)
	{
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface* screen, object object)
{
	SDL_Rect dest;
	dest.x = object.posX - object.graphic->w / 2;
	dest.y = object.posY - object.graphic->h / 2;
	dest.w = object.graphic->w;
	dest.h = object.graphic->h;
	SDL_BlitSurface(object.graphic, NULL, screen, &dest);
};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color)
{
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color)
{
	for (int i = 0; i < l; i++)
	{
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};


// rysowanie prostok¹ta o d³ugoœci boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor)
{
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

