#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL2_gfxPrimitives.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define WINDOW_WIDTH 680
#define WINDOW_HEIGHT 480
#define NUM  8
#define RAD 10
#define MAX_X WINDOW_WIDTH/(2*RAD)
#define MAX_Y WINDOW_HEIGHT/(2*RAD)
#define SPEED_X 300
#define SPEED_Y 300

bool running;

SDL_Renderer* rend;
SDL_Window* win;

int frameCount, lastTime, timerFPS, lastFrame, fps;

void input() 
{
	SDL_Event e;
	while(SDL_PollEvent(&e))
		if(e.type == SDL_QUIT)
			running = false;
	const Uint8 *keystates = SDL_GetKeyboardState(NULL);
	if(keystates[SDL_SCANCODE_ESCAPE])
		running = false;
}

void initialize_grid(bool grid[MAX_X][MAX_Y])
{
	int X, Y;
	bool occupied = true;
	bool unoccupied = false;

	// seed RNG
	srand(time(NULL));

	// set grid to unoccupied
	for(int i=0; i<MAX_X; ++i)
			for(int j=0; j<MAX_Y; ++j)
					grid[i][j] = unoccupied;

	// randomly fill grid
	for(int count=0; count<NUM; ++count)
	{
			X = rand() % (MAX_X - 2) + 1;
			Y = rand() % (MAX_Y - 2) + 1;
			
			if (!grid[X][Y])
					grid[X][Y] = occupied;
			else
					--count;
	}
}

void initial_conditions(bool grid[MAX_X][MAX_Y],
												float pos_x[NUM],
												float pos_y[NUM],
												float vel_x[NUM],
												float vel_y[NUM])
{
	// particle number between 1 and NUM
	int particle = 1;

	for (int y_cell = 0; y_cell < MAX_Y; ++y_cell)
		for (int x_cell = 0; x_cell < MAX_X; ++x_cell) {
			if (grid[x_cell][y_cell]) {
				pos_x[particle] = x_cell * (2 * RAD);
				pos_y[particle] = y_cell * (2 * RAD);
				vel_x[particle] = pow(-1, rand()%2) * SPEED_X;
				vel_y[particle] = pow(-1, rand()%2) * SPEED_Y;
				++particle; 
			}
		}
}

void test_boundary_collisions(float pos_x[NUM], 
															float pos_y[NUM], 
															float vel_x[NUM], 
															float vel_y[NUM])
{
	for (int particle = 1; particle <= NUM; ++particle)    
	{
			// collision detection on bounds
			if (pos_x[particle] <= RAD)
			{
					pos_x[particle] = RAD;
					vel_x[particle] = -vel_x[particle];
			}
			if (pos_y[particle] <= RAD)
			{
					pos_y[particle] = RAD;
					vel_y[particle] = -vel_y[particle];
			}
			if (pos_x[particle] >= WINDOW_WIDTH - RAD) 
			{
					pos_x[particle] = WINDOW_WIDTH - RAD;
					vel_x[particle] = -vel_x[particle];
			}
			if (pos_y[particle] >= WINDOW_HEIGHT - RAD)
			{
					pos_y[particle] = WINDOW_HEIGHT - RAD;
					vel_y[particle] = -vel_y[particle];
			}
	}
}

float distance(float X_1, float X_2, float Y_1, float Y_2)
{

}

void test_particle_collisions(float pos_x[NUM],
															float pos_y[NUM],
															float vel_x[NUM],
															float vel_y[NUM])
{
  float vx_11, vx_21, vx_12, vx_22, xx_11, xx_22, xx_12, sub12_x, sub12_y, 
        sub21_x, sub21_y;
  float Dvel_x[NUM], Dvel_y[NUM];
	
	for (int incident_particle = 1; incident_particle <= NUM; ++incident_particle)
		for (int particle = 1; particle < incident_particle; ++particle)
			if ((fabsf(pos_x[incident_particle] - pos_x[particle]) <= 2*RAD) && 
					(fabsf(pos_y[incident_particle] - pos_y[particle]) <= 2*RAD))
			{
					vx_11 = vel_x[incident_particle]*pos_x[incident_particle]
								+ vel_y[incident_particle]*pos_y[incident_particle];

					vx_21 = vel_x[particle]*pos_x[incident_particle] 
								+ vel_y[particle]*pos_y[incident_particle];
					
					vx_12 = vel_x[incident_particle]*pos_x[particle] 
								+ vel_y[incident_particle]*pos_y[particle];
					
					vx_22 = vel_x[particle]*pos_x[particle] 
								+ vel_y[particle]*pos_y[particle];

					xx_11 = pos_x[incident_particle]*pos_x[incident_particle]
								+ pos_y[incident_particle]*pos_y[incident_particle];

					xx_22 = pos_x[particle]*pos_x[particle] 
								+ pos_y[particle]*pos_y[particle];

					xx_12 = pos_x[incident_particle]*pos_x[particle] 
								+ pos_y[incident_particle]*pos_y[particle];

					sub12_x = pos_x[incident_particle] - pos_x[particle];
					sub12_y = pos_y[incident_particle] - pos_y[particle];
					sub21_x = pos_x[particle] - pos_x[incident_particle];
					sub21_y = pos_y[particle] - pos_y[incident_particle];

			for (int incident_particle = 1; incident_particle <= NUM; ++incident_particle)
		     	Dvel_x[incident_particle] = vel_x[incident_particle] - (((vx_11 - vx_21 - vx_12 + vx_22) 
											/ (xx_11 + xx_22 - (2 * xx_12))) * sub12_x );
					
					Dvel_y[incident_particle] = vel_y[incident_particle] - (((vx_11 - vx_21 - vx_12 + vx_22) 
											/ (xx_11 + xx_22 - (2 * xx_12))) * sub12_y );
					
					Dvel_x[particle] = vel_x[particle] - (((vx_11 - vx_21 - vx_12 + vx_22) 
											/ (xx_11 + xx_22 - (2 * xx_12))) * sub21_x );
					
					Dvel_y[particle] = vel_y[particle] - (((vx_11 - vx_21 - vx_12 + vx_22) 
											/ (xx_11 + xx_22 - (2 * xx_12))) * sub21_y );

					vel_x[incident_particle] = Dvel_x[incident_particle];
					vel_y[incident_particle] = Dvel_y[incident_particle];
					vel_x[particle] = Dvel_x[particle];
					vel_y[particle] = Dvel_y[particle];
			}
}

void draw_state(float pos_x[NUM], float pos_y[NUM]) 
{
	SDL_SetRenderDrawColor(rend, 40, 40, 40, 255);
	SDL_Rect rect;
	rect.x = rect.y = 0;
	rect.w = WINDOW_WIDTH;
	rect.h = WINDOW_HEIGHT;
	SDL_RenderFillRect(rend, &rect);

	// draw particle state
	for (int particle = 1; particle <= NUM; ++particle) {
		filledCircleRGBA(rend, pos_x[particle], pos_y[particle], RAD, 215, 153, 33, 255);
	}

	frameCount++;
	int timerFPS = SDL_GetTicks() - lastFrame;
	if(timerFPS<(1000/60))
		SDL_Delay((1000/60) - timerFPS);

	SDL_RenderPresent(rend);
}

int main()
{
	running = 1;
	bool grid[MAX_X][MAX_Y];
	float pos_x[NUM], pos_y[NUM], vel_x[NUM], vel_y[NUM];
	
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		printf("Failed at SDL_Init()\n");
	if (SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &win, &rend) < 0)
		printf("Failed at SDL_CreateWindowAndRenderer()\n");
	SDL_SetWindowTitle(win, "Title");
	SDL_ShowCursor(0);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

	initialize_grid(grid);
	initial_conditions(grid, pos_x, pos_y, vel_x, vel_y);

	while (running) {
		lastFrame = SDL_GetTicks();
		if (lastFrame >= (lastTime+1000)) {
			lastTime = lastFrame;
			fps = frameCount;
			frameCount = 0;
		}
		
		// print statement for fps debugging.
		// printf("%d\n", fps);

		// look for input.
		input();

		// test for particle or boundary collisions and change (x, v) conditions.
		test_boundary_collisions(pos_x, pos_y, vel_x, vel_y);
		test_particle_collisions(pos_x, pos_y, vel_x, vel_y);

		// update positions
		for (int particle = 1; particle <= NUM; ++particle) {
			pos_x[particle] += vel_x[particle] / 60;
			pos_y[particle] += vel_y[particle] / 60;
		}
		
		printf("(%f,%f)\n", pos_x[1], pos_y[1]);
		printf("(%f,%f)\n", vel_x[1], vel_y[1]);
		draw_state(pos_x, pos_y);
	}

	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);
	SDL_Quit();
}
