#include "graphics.h"

/*g++ graphicaltime.cpp 
-lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -static-libstdc++ -std=c++11 
-Wl,-rpath,./
-o graphicaltime

g++ graphicaltime.cpp -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -static-libstdc++ -std=c++11 -o graphicaltime
*/

const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

/*
	ASSETS:
	---------------------------------------------
	Monster/000.png
	Monster/001.png
	Monster/002.png
	Monster/003.png
	Monster/004.png
	Monster/005.png
	Monster/006.png
	
	sprites/background.png
	sprites/selection.png
	sprites/commands.png
	sprites/star.png
	sprites/talkbubble.png
	sprites/stickerstar.png
	sprites/turnHouser.png
	
	fonts/Roman.ttf
	
	OST/Sound/arrow.wav
	OST/Sound/sound_effect1.wav
	OST/Sound/sound_effect2.wav
	OST/Sound/sound_effect3.wav
	
*/

// holds info for displaying characters in combat 
// icon - full sprite
// turn - turn counter dimensions to render
// party - party member icon dimensions to render

void printFrontEnemies()
{
	int r2 = 3;
				
	inBattle[r2].icon.render(50,330);
	inBattle[r2].icon.render(230,360);
	inBattle[r2].icon.render(390,330);

}

void printBackEnemies() // printing enemies in the backrow
{
	int r1 = 2;
	
	inBattle[r1].icon.render(40,180);
	inBattle[r1].icon.render(220,220);
	inBattle[r1].icon.render(390,170);

}

void turnOrder()
{
	image back = image("sprites/turnHouser.png");
	image arr = image("sprites/nextarrow.png");
	back.render(960-(128*2),150,NULL,2);
	arr.render(630,170);
	
}

double angel = 0.0; // used for background animations
bool down = false;
image stary = image("sprites/stickerstar.png");

void background( bool lowHealth)
{
	if(lowHealth)
		main_window.setBackColor(angel,0,0,0);
	else
		main_window.setBackColor(100,0,(int) angel,0);
	stary.setColor(200,200,255);
	stary.renderRotate(300,100,NULL,angel,5);
	stary.renderRotate(300,0,NULL,angel,5);
	stary.renderRotate(300,-100,NULL,angel,5);
		
	stary.setColor(255,255,255);
	stary.renderRotate(700,200,NULL,angel,5);
	stary.renderRotate(700,100,NULL,angel,5);
	stary.renderRotate(700,0,NULL,angel,5);
	
	
	if(angel >= 255)
		down = true;
	if(angel <= 0)
		down = false;
	if(down == false)
		angel+=.1;
	else
		angel-=.1;
}


int main()
{
	//SetDllDirectory(".");
	srand((unsigned)time(NULL));
	
	//HWND windowHandle = GetConsoleWindow();
	//ShowWindow(windowHandle,SW_HIDE);
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	
	handler input = handler();
	block men = block(5,3);
	
	menus playerr = menus("sprites/commands.png",4,255,200);

	image star = image("sprites/star.png",4);
	
	bool exit = false;
	bool arroww = false; // is the arrow being used to select a target?
	bool lowHealth = false; // low health background animation
	bool sele = false;
	bool statInc = false;
	int counter;
	
	int health = 0;
	
	double angle = 0;
	int backr = 100;
	
	SDL_Rect rend;
	rend.x=0;
	rend.y=10;
	rend.w=37;
	rend.h=20;
	
	arrow ar = arrow(6);
	
	image back = image("sprites/background.png");
	
	negotiation talk;
	
	declareEnemy();
	
	image ani[3]; 
	ani[0] = image("sprites/plusAgi.png");
	ani[1] = image("sprites/plusDef.png");
	ani[2] = image("sprites/plusAtt.png");
	double val = 0;
	
	int valu;
	
	while(exit == false)
	{
		if(talk.active == false)
			main_window.playMusic(1);
		else
			main_window.playMusic(2);
		
		if(counter == 3)
			counter = 0;
		
		if(talk.pickingEnemy == true || talk.active == true) // if the arrow is used to select a target, it handles the input
		{		
			talk.handle(input.getEvents());
		}
		else if(ar.active)
		{
			ar.handle(input.getEvents());
		}
		else
		{
			switch(input.getEvents())
			{
				case -1:
				exit = true;
				break;
			
				//  up
				case 1:
				statInc = true;
				valu = rand()%3;
				val = 0;
				//playerr.increaseOption();
				break;
			
				// down
				case 2:
				playerr.decreaseOption();
				break;
			
				// select
				case 3:
				if(playerr.getOption() == 0)
					ar.active = true;
				else if(playerr.getOption() == 1)
					talk = negotiation(6);
				break;
			
				case 4:
				if(lowHealth)
					lowHealth = false;
				else
					lowHealth = true;
				break;
			}
		}
		if(statInc)
		{
			if(val >= 20)
				statInc = false;
		}
		
		// clear display
		main_window.clearDisplay();
		
		// background
		background(lowHealth);
	
		if(lowHealth)
			back.setColor((int)(angle)%255+100,0,0);
		else
			back.setColor(255,255,255);
		back.render(0,0);
		
		printBackEnemies();
		printFrontEnemies();
		
		// print arrow when selecting target
		ar.renderArrow();
		
		// turn order
		//enemies[0].render(800,200,&rend,2);
		
		// spinning star
		star.renderRotate(700,500,NULL,angle);
		
		// main battle selection menu
		playerr.render(500,SCREEN_HEIGHT-200);
		
		// print party stat
		
		if(statInc)
		{
			ani[valu].setAlpha(val*5);
			ani[valu].render(100,100-(((int)val)*5));
			val+=.15;
		}
		
		// for if the player is negotiating 
		talk.render();
		
		turnOrder();
		
		main_window.display();
		counter++;
		if(sele)
			angle+=.02;
		else
			angle+=.05;
	}
	dele:
	main_window.close();
}