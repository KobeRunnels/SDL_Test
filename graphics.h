#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include <cmath>
#include <cstdlib>
#include <ctime>

#include <string>

#include <random>
#include <iostream>
#include <vector>

using namespace std;

class image;
class handler;

//Screen dimension constants
const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 720;

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 8000;

//Game Controller 1 handler
SDL_Joystick* gGameController = NULL;

// handles overall window handling
class window
{
	public:
		window()
		{
			//Initialization flag
			
			//Initialize SDL
			if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0 )
			{
			}
			else
			{
				//Set texture filtering to linear
				if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
				{
					printf( "Warning: Linear texture filtering not enabled!" );
				}		

				//Check for joysticks
				if( SDL_NumJoysticks() < 1 )
				{
				}
				else
				{
					//Load joystick
					gGameController = SDL_JoystickOpen( 0 );
					if( gGameController == NULL )
					{
						printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() );
					}
				}

				//Create window
				gWindow = SDL_CreateWindow( "Graphics", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
				if( gWindow == NULL )
				{
					printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
					success = false;
				}
				else
				{
					//Create renderer for window
					gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
					if( gRenderer == NULL )
					{
						printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
						success = false;
					}
					else
					{
						//Initialize renderer color
						SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

						//Initialize PNG loading
						int imgFlags = IMG_INIT_PNG;
						if( !( IMG_Init( imgFlags ) & imgFlags ) )
						{
							printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
							success = false;
						}
						
						 //Initialize SDL_ttf
						if( TTF_Init() == -1 )
						{
							printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
							success = false;
						}
							
						//Initialize SDL_mixer
						if( Mix_OpenAudio( MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024*2 ) < 0 )
						{
							printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
							success = false;
						}
					}
				}
			}

		}
		void display()
		{		
			SDL_RenderPresent( gRenderer );
		}
		void setBackColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
		{
			SDL_SetRenderDrawColor( gRenderer, r, g, b, a);
			SDL_RenderClear( gRenderer );
		}
		void clearDisplay()
		{
			SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
			SDL_RenderClear( gRenderer );
		}
		void playMusic(int i) // for playing a song in an area
		{
			if(i!= currentTrack)
			{
				Mix_HaltMusic();
				switch(i)
				{
					case 1:
					gMusic =  Mix_LoadMUS( "OST/Battle/ReReRe Fight.wav" );
					break;
					case 2:
					gMusic = Mix_LoadMUS( "OST/TEST TALK.mp3" );
					break;
				}
				currentTrack = i;
				Mix_PlayMusic( gMusic, -1 ); // plays battle theme
			}
		}
		void close()
		{
			SDL_DestroyTexture( gTexture );
			gTexture = NULL;

			//Destroy window	
			SDL_DestroyRenderer( gRenderer );
			SDL_DestroyWindow( gWindow );
			gWindow = NULL;
			gRenderer = NULL;
			
			Mix_Quit();
			IMG_Quit();
			SDL_Quit();
			
			exit(0);
		}

		bool success = false;
		
		int currentTrack = 0; // current song being played on the window
		Mix_Music * gMusic; // for playing music	
		
		SDL_Window* gWindow = NULL; // window 
		SDL_Renderer* gRenderer = NULL; // window renderer
		SDL_Texture* gTexture = NULL; // displayed texture
};

window main_window = window();

// handles different graphical components in program
class image
{
	public:
		image() {}
		image(string pathr)
		{
			path = pathr;
			loadTexture();
		} 
		image(string pathr, int s)
		{
			scale = s;
			path = pathr;
			loadTexture();
		} 
		void setPath(string p)
		{
			path = p;
		}
		void setColor( Uint8 red, Uint8 green, Uint8 blue )
		{
			//Modulate texture
			SDL_SetTextureColorMod(texture, red, green, blue );
		}
		void setAlpha(Uint8 alph)
		{
			SDL_SetTextureAlphaMod(texture, alph);
		}
		void loadTexture()
		{
			SDL_Renderer* gRenderer = main_window.gRenderer;
			//The final texture
			SDL_Texture* newTexture = NULL;

			//Load image at specified path
			SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
			if( loadedSurface == NULL )
			{
				printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
			}
			else
			{
				//Create texture from surface pixels
				newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
				if( newTexture == NULL )
				{
					printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
				}
				else
				{
					width = loadedSurface->w;
					height = loadedSurface->h;
				}
				//Get rid of old loaded surface
				SDL_FreeSurface( loadedSurface );
			}
			texture = newTexture;
		}
		void setScale(int num)
		{
			scale = num;
		}
		void renderRotate(int x, int y, SDL_Rect* clip=NULL, double angle=0.0, int size=0, SDL_Point* center=NULL, SDL_RendererFlip flip  = SDL_FLIP_NONE)
		{
			SDL_Rect renderQuad;
			//Set rendering space and render to screen
			if(size == 0)
				renderQuad = { x, y, width*scale, height*scale };
			else
				renderQuad = { x, y, width*size, height*size};
			if(clip!=NULL && size==0)
			{
				renderQuad.w = clip->w*scale;
				renderQuad.h = clip->h*scale;	
			}
			else if(clip!=NULL && size!=0)
			{
				renderQuad.w = clip->w*size;
				renderQuad.h = clip->h*size;	
			}

			//Render to screen
			SDL_RenderCopyEx(main_window.gRenderer, texture, clip, &renderQuad, angle, center, flip );
		}
		void render( int x, int y, SDL_Rect* clip=NULL, int size=0)
		{
			SDL_Rect renderQuad;
			//Set rendering space and render to screen
			if(size == 0)
				renderQuad = { x, y, width*scale, height*scale };
			else
				renderQuad = { x, y, width*size, height*size};
			if(clip!=NULL && size==0)
			{
				renderQuad.w = clip->w*scale;
				renderQuad.h = clip->h*scale;	
			}
			else if(clip!=NULL && size!=0)
			{
				renderQuad.w = clip->w*size;
				renderQuad.h = clip->h*size;	
			}
			
			//Render to screen
			SDL_RenderCopy(main_window.gRenderer, texture, clip, &renderQuad );
		}
	private:
		//Set rendering space and render to screen
		
		int scale = 1;
		int x = 0;
		int y = 0;
		int height;
		int width;
		
		string path;
		SDL_Texture* texture;
};

class text
{
	public:
		text()
		{
		}
		text(string font,int sizer)
		{
			gFont = TTF_OpenFont(font.c_str(), sizer);
		}
		void setSize(int si)
		{
			sizer = si;
			gFont = TTF_OpenFont(font.c_str(), sizer);
		}
		void setColor( Uint8 red, Uint8 green, Uint8 blue )
		{
			//Modulate texture
			SDL_SetTextureColorMod(mTexture, red, green, blue );
		}
		void setAlpha(Uint8 alph)
		{
			SDL_SetTextureAlphaMod(mTexture, alph);
		}
		void load(string text)
		{
			//Render text surface
			SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, text.c_str(), textColor );
			if( textSurface == NULL )
			{
				printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
			}
			else
			{
				//Create texture from surface pixels
				mTexture = SDL_CreateTextureFromSurface(main_window.gRenderer, textSurface );
				if( mTexture == NULL )
				{
					printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
				}
				else
				{
					//Get image dimensions
					width = textSurface->w;
					height = textSurface->h;
				}
				
				//Get rid of old surface
				SDL_FreeSurface( textSurface );
			}
			
		}
		void render( int x, int y)
		{
			//Set rendering space and render to screen
			SDL_Rect renderQuad = { x, y, width, height};

			//Render to screen
			SDL_RenderCopy(main_window.gRenderer, mTexture, NULL, &renderQuad );
		}
		void display(string text, int x, int y)
		{
			load(text);
			render(x,y);
		}
		void display(string text, int x, int y, int size)
		{
			setSize(size);
			load(text);
			render(x,y);
		}
	private:
		
		SDL_Color textColor = {0,0,0};
		string font;
		int sizer = 1;
		
		int width;
		int height;
		
		SDL_Texture* mTexture;
		TTF_Font *gFont = NULL;
};

class handler // handles inputs
{
	public:
		handler()
		{
		}
		int getEvents()
		{
			const Uint8* keyStates = SDL_GetKeyboardState(NULL);
			while(SDL_PollEvent(&e) != 0)
			{
				switch(e.type)
				{
					case SDL_JOYBUTTONDOWN: // for controller input
					if(e.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) // up
						return 1;
					if(e.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN ) // down
						return 2;
					if(e.jbutton.button == SDL_CONTROLLER_BUTTON_B) // switch a button
						return 3;
					if(e.jbutton.button == SDL_CONTROLLER_BUTTON_A) // switch b button
						return 4;
					if(e.jbutton.button == SDL_CONTROLLER_BUTTON_X) // switch y button
						return 5;
					break;
					case SDL_KEYDOWN: // for keyboard input
					if(e.key.keysym.sym == SDLK_RETURN) // enter = a
						return 3;
					if(e.key.keysym.sym == SDLK_w || e.key.keysym.sym == SDLK_UP) // up
						return 1;
					if(e.key.keysym.sym == SDLK_s || e.key.keysym.sym == SDLK_DOWN) // down
						return 2;
					if(e.key.keysym.sym == SDLK_BACKSPACE) // backspace = b
						return 4;
					break;
					case SDL_QUIT: // clicking the x window button
					return -1;
					break;
				}
			}
			return 0;
		}
	private:
		SDL_Event e;
};

/*
0 - top left corner
1 - bottom left corner
2 - left
3 - right 
4 - top
5 - bottom
6 - bottom right corner
7 - top right corner
*/

class arrow // for picking targets in combat
{
	public:
		arrow(){}
		arrow(int opts)
		{
			move = Mix_LoadWAV("OST/Sound/arrow.wav");
			options = opts;
		}
		void renderArrow()
		{
			if(active)
			{
				int x,y;
				switch(option)
				{
					case 1:
					x=55;
					break;
					case 4:
					x=75;
					break;
					case 2:
					x= 230;
					break;
					case 5:
					x= 260;
					break;
					case 3:
					x = 400;
					break;
					case 6:
					x = 420;
					break;
				}
				if(option > 3)
					y = 230;
				else
					y= 80;
				icon.render(x,y);
			}
		}
		void increaseOption()
		{
			option++;
			Mix_PlayChannel( -1, move, 0 );	
			if(option > options)
				option = 1;
		}
		void decreaseOption()
		{
			option--;
			Mix_PlayChannel( -1, move, 0 );	
			if(option < 1)
				option=options;
		}
		int handle(int input) // handles input
		{
			switch(input)
			{
				case -1:
				main_window.close();
				break;
				case 1:
				increaseOption();
				break;
				case 2:
				decreaseOption();
				break;
				case 3:
				active = false;
				return option;
				break;
				case 4:
				active = false;
				break;
			}
			return 0;
		}
		bool active = false;
		int getOption() {return option;}
	private:
		Mix_Chunk * move;
		image icon = image("sprites/arrow.png",3);
		int option = 1;
		int options;
};

class block
{
	public:
	block(int x, int y)
	{
		sizeX = x;
		sizeY = y;
		presenter = image("sprites/menutest.png");
		presenter.loadTexture();
		for(int i=0;i<tiles;i++)
		{
			menus[i].x = i*width;
			menus[i].y = 0;
			menus[i].w = width;
			menus[i].h = width;
		}
	}
	void render(int xx, int yy)
	{
		for(int y=0;y<sizeY;y++)
		{
			for(int x=0;x<sizeX;x++)
			{
				if(x==0 && y==0)
					presenter.render(width*x+xx, width*y+yy,&menus[0]);
				else if(x==0 && y == sizeY-1)
					presenter.render(width*x+xx, width*y+yy,&menus[1]);
				else if(x==sizeX-1 && y == sizeY-1)
					presenter.render(width*x+xx, width*y+yy,&menus[7]);
				else if(x==sizeX-1 && y == 0)
					presenter.render(width*x+xx, width*y+yy,&menus[8]);
				else if(x==0)
					presenter.render(width*x+xx, width*y+yy,&menus[2]);
				else if(x==sizeX-1)
					presenter.render(width*x+xx, width*y+yy,&menus[4]);
				else if(y==0)
					presenter.render(width*x+xx, width*y+yy,&menus[5]);	
				else if(y==sizeY-1)
					presenter.render(width*x+xx, width*y+yy,&menus[6]);	
				else
					presenter.render(width*x+xx, width*y+yy,&menus[3]);		
			}
		}
	}
	private:
		int width = 40;
		int sizeX;
		int sizeY;
		image presenter;
		SDL_Rect menus[9];
		int tiles = 9;
};

// class for handling main input for commands in battle
class menus 
{
	public:
		menus(string path, int maxFrames, int w, int h)
		{
			base = image(path);
			frames = maxFrames;
			clips = new SDL_Rect[maxFrames];
			for(int i=0;i<maxFrames;i++)
			{
				clips[i].x = i*w;
				clips[i].y = 0;
				clips[i].w = w;
				clips[i].h = h;
			}
		}
		void increaseOption()
		{
			frame = frame + 1;
			if(frame >= frames)
				frame = 0;
		}
		void decreaseOption()
		{
			frame = frame - 1;
			if(frame < 0)
				frame = frames-1;
		}	
		void render(int x, int y)
		{
			base.render(x,y,&clips[frame]);
		}
		int getOption(){return frame;}
	private:
		image base;
		SDL_Rect* clips;
		int frames;
		int frame = 0;
};

// class for handling animations with frames
class animation // for any animation
{
	public:
		animation(string path, int framers, int w, int h, double rater)
		{
			frames = framers;
			rate = rater;
			base = image(path);
			clips = new SDL_Rect[frames];
			for(int i=0;i<frames;i++)
			{
				clips[i].x = i*w;
				clips[i].y = 0;
				clips[i].w = w;
				clips[i].h = h;	
			}
		}
		void update_image(int x, int y)
		{
			set_frame+=rate;
			if(set_frame > frames)
				set_frame = 0;
			current_frame = set_frame;
			base.render(x,y,&clips[current_frame]);
			
		}
	private:
		image base;
		SDL_Rect * clips;
		int current_frame = 0;
		int frames = 0; // number of frames
		double set_frame = 0;
		double rate = 1; // rate of frames updating
};

// class for handling battle dialogue selection with talking/fusion
class selector
{
	public:
		selector() {}
		selector(int ss, string * s)
		{
			move = Mix_LoadWAV("OST/Sound/sound_effect1.wav");
			accept = Mix_LoadWAV("OST/Sound/sound_effect3.wav");
			selections = ss;
			for(int i=0;i<ss;i++)
			{
				choices.push_back(s[i]);
			}
		}
		selector(int ss, string * s, int sss)
		{
			scale = sss;
			selections = ss;
			for(int i=0;i<ss;i++)
			{
				choices.push_back(s[i]);
			}
			
		}
		void render(int x,int y)
		{
			for(int i=0;i<selections;i++)
			{
				if(i == selection)
				{
					selectorIcon.setColor(255,0,0);
					selectorIcon.render(x+scale*10,y+(40*scale*i)+9*scale,NULL,scale);
					fonter.display(choices.at(i),x+10+scale*10,y+(40*scale*i)+9*scale+5);
				}
				else
				{
					selectorIcon.render(x,y+(40*scale*i)+9*scale,NULL,scale);
					fonter.display(choices.at(i),x+10,y+(40*scale*i)+9*scale+5);
				}
				if(i == selection)
					selectorIcon.setColor(255,255,255);
			}
		}
		void increaseOption()
		{
			if(selection != selections-1)
			{
				Mix_PlayChannel( -1, move, 0 );			
				selection++;	
			}
		}
		void decreaseOption()
		{
			if(selection != 0)
			{
				Mix_PlayChannel( -1, move, 0 );
				selection--;
			}
		}
		int getSelection()
		{
			Mix_PlayChannel( -1, accept, 0 );			
			return selection;
		}
		void deleteStrings()
		{
			delete &choices;
		}
	private:
		Mix_Chunk * move; // sound effect for using selections
		Mix_Chunk * accept; // sound effect for accepting option
		int scale = 1;
		int selection = 0;
		int selections = 1;
		image selectorIcon = image("sprites/selection.png",2);
		text fonter = text("fonts/Roman.ttf",30);
		vector<string> choices;
};

void talkBubble(string textt, int x, int y) // creates a talk bubble for enemies in combat
{
	image icon = image("sprites/talkbubble.png"); // dialouge output
	text printer = text("fonts/Roman.ttf",30); // for showing negotiation questions
	
	icon.render(x,y,NULL,2);
	if(textt.length() > 20)
	{
		int spot = 20;
		while(textt[spot] != ' ' && spot!=textt.length())
		{
			spot = spot + 1;
		}
		if(textt[20] != ' ' && spot != textt.length())
		{
			
			printer.display(textt.substr(0,spot),x+15,y+5);
			printer.display(textt.substr(spot),x+15,y+35);
		}
		else
		{
			printer.display(textt.substr(0,20),x+15,y+5);
			printer.display(textt.substr(20),x+15,y+35);
		}
	}
	else
		printer.display(textt,x+15,y+5);
}

class negotiation // handling negotiations with enemies in combat
{
	public:
		negotiation(){}
		negotiation(int en)
		{
			decline = Mix_LoadWAV("OST/Sound/sound_effect2.wav");
			pick = arrow(en); // picks which enemy to talk to
			pick.active = true;
			
			pickingEnemy = true;
			
			generateTopic();
			
			p = selector(talkings,talks);
		}
		void generateTopic() // generates topic and dialogue options to present
		{
			// chose conversation
			question = "Want some soda?";
			talkings = 2;
			talks = new string[4]; 
			talks[0] = "Yes";
			talks[1] = "No";
			
		}
		void handle(int i) // moving input handling to negotiation 
		{
			int getter;
			if(pickingEnemy) // if the player is picking a target, the arrow handles input
			{
				getter = pick.handle(i);
				if(getter != 0) 
				{
					pos = getter;
					active = true;
					pickingEnemy = false;
				}
				else if(!pick.active) // if declining to negotiate 
				{
					Mix_PlayChannel( -1, decline, 0 );			
					pickingEnemy = false;
				}
			}
			else // for when the player is talking to the enemy
			{
				switch(i)
				{
					case 1: // up
					p.decreaseOption();
					break;
					case 2: // down
					p.increaseOption();
					break;
					case 3: // select
					p.getSelection();
					active = false;
					break;
					case -1:
					main_window.close();
					break;
				}
			}
		}
		void render() // render what's happening to screen
		{
			int x = 0;
			int y = 0;
			if(pickingEnemy == true)
			{
				pick.renderArrow();
			}
			else if(active)
			{
				switch(pos) // getting enemy coordinates based on selection
				{
					case 1:
					x = 40;
					y = 180;
					break;
					case 2:
					x = 220;
					y = 220;
					break;
					case 3:
					x = 390;
					y = 170;
					break;
					case 4:
					x = 50;
					y = 330;
					break;
					case 5:
					x = 230;
					y = 360;
					break;
					case 6:
					x = 390;
					y = 330;
					break;
				}	
				x+=(64*2);
				y-=(64*2);
				talkBubble(question,x,y);
				p.render(x+100,y+81);
			}
		}
		bool pickingEnemy = false;
		bool active = false;
	private:
		Mix_Chunk * decline; // sound effect for declining option
		arrow pick; // for picking target to talk with before talking/fusion
		string question; // what the enemy is asking
		string * talks; // holding dialogue choices;
		int pos; // which enemy is talking?
		int talkings = 0; // responses
		selector p; // selector for showing/receiving input
};

struct enemyGraphics
{
	image icon;
	SDL_Rect turn;
	SDL_Rect party;
};

enemyGraphics inBattle[7];

void declareEnemy() // sets up icons and loads them
{
	inBattle[0].icon = image("sprites/Monster/000.png",3);
	inBattle[0].turn = {0,42,64,19};
	inBattle[0].party = {38,59,23,23}; 
	
	inBattle[1].icon = image("sprites/Monster/001.png",3);
	inBattle[1].turn = {0,6,64,19};
	inBattle[1].party = {12,12,23,23};
	
	inBattle[2].icon = image("sprites/Monster/002.png",3);
	inBattle[2].turn = {0,0,64,19};
	inBattle[2].party = {12,0,23,23};
	
	inBattle[3].icon = image("sprites/Monster/003.png",3);
	inBattle[3].turn = {0,2,64,19};
	inBattle[3].party = {13,7,23,23};
	
	inBattle[4].icon = image("sprites/Monster/004.png",3);
	inBattle[4].turn = {0,0,64,19};
	inBattle[4].party = {10,0,23,23};
	
	inBattle[5].icon = image("sprites/Monster/005.png",3);
	inBattle[5].turn = {0,10,64,19};
	inBattle[5].party = {10,14,23,23};
	
	inBattle[6].icon = image("sprites/Monster/006.png",2);
	inBattle[6].turn = {0,6,64,19};
	inBattle[6].party = {29,12,23,23};

}
