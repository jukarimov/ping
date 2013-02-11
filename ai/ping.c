#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>

#define maxx	64
#define maxy	20

#define max	maxx-2
#define may	maxy-2

#define mix	1
#define miy	2

#define minx	0
#define miny	0

unsigned int SLEEPT	= 125000;

#define ARRAYSIZE(a)	(sizeof(a)/sizeof(a[0]))

int	end = 0;

struct Position {
	int		x;
	int		y;
};

void setpos(struct Position *p, int x, int y)
{
	p->x = x;
	p->y = y;
}

enum {
	UPRIGHT,
	UPLEFT,
	DOWNRIGHT,
	DOWNLEFT,
};

struct Ball {
	struct Position pos;
	int		dir;
	int		speed;
};

struct Ball ball;

struct Paddle {
	struct Position pos;
	int		score;
};

struct Paddle user;
struct Paddle ai;

#define LEN(a)	((sizeof(a)/sizeof(*a)))

int user_serving = 1, times_served = 0;

/* ========================================================================= */

void init_curses()
{
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    scrollok(stdscr, FALSE);
    curs_set(0);
}

void end_curses()
{
    flushinp();
    erase();
    refresh();
    endwin();
}

void init_game()
{
	setpos(&user.pos, mix+3, maxy/2);
	setpos(&ai.pos, max-3, maxy/2);
	setpos(&ball.pos, mix+4, maxy/2);

	user.score = 0;
	ai.score = 0;

	ball.speed = 1;
}

void gpause();
void draw_borders();
void draw_ball();
void draw_users();
void draw_info();
void user_serve();
void ai_serve();

void serve()
{
	if (++times_served == 5) {

		if (user_serving)
			user_serving = 0;
		else
			user_serving = 1;

		times_served = 0;
	}

	setpos(&user.pos, mix+3, maxy/2);
	setpos(&ai.pos, max-3, maxy/2);

	if (user_serving)
		setpos(&ball.pos, mix+4, maxy/2);
	else
		setpos(&ball.pos, max-4, maxy/2);

	erase();
	draw_borders();
	draw_ball();
	draw_users();
	draw_info();
	refresh();

	if (user_serving)
		user_serve();
	else
		ai_serve();

}

void game_finish()
{
	end_curses();
}

void draw_info()
{
	char res[10];

	sprintf(res, "%d", user.score);
	mvprintw(0, 1, res);

	sprintf(res, "%d", ai.score);
	mvprintw(0, maxx-2, res);
}

void move_ball()
{
	int x, y, xv, yv;

	x = ball.pos.x;
	y = ball.pos.y;

	xv = 0;
	yv = 0;

	switch (ball.dir)
	{
	case UPRIGHT:
		xv = 1;
		yv = -1;
		break;
	case UPLEFT:
		xv = -1;
		yv = -1;
		break;
	case DOWNRIGHT:
		xv = 1;
		yv = 1;
		break;
	case DOWNLEFT:
		xv = -1;
		yv = 1;
		break;
	}

	x += xv;
	y += yv;

	if (x < minx) {
		ai.score++;
		serve();
		return;
	}
	else if (x > maxx) {
		++user.score;
		serve();
		return;
	}

	if (y == miy)
	{
		if (ball.dir == UPRIGHT)
			ball.dir = DOWNRIGHT;
		else
			ball.dir = DOWNLEFT;
	}

	if (y == may) {
		if (ball.dir == DOWNRIGHT)
			ball.dir = UPRIGHT;
		else
			ball.dir = UPLEFT;
	}

	if ((y >= ai.pos.y && y <= ai.pos.y+2) && x == ai.pos.x)
	{
		if (ball.dir == UPRIGHT)
			ball.dir = DOWNLEFT;
		else
			ball.dir = UPLEFT;
		return;
	}

	if ((y >= user.pos.y && y <= user.pos.y+2) && x == user.pos.x)
	{
		if (ball.dir == DOWNLEFT)
			ball.dir = DOWNRIGHT;
		else
			ball.dir = UPRIGHT;
	}

	setpos(&ball.pos, x, y);

	if (y < miy+6)
		ai.pos.y = (y > miy ? y-1 : y);
	else
	if (y < may-1)
		ai.pos.y = y;
}

void user_serve()
{
	while (1) {
		int c = getchar();
		if (c == 'w' || c == 's') {
			ball.dir = c == 'w' ? UPRIGHT : DOWNRIGHT;
			break;
		}
		else if (c == 27)
		{
			game_finish();
			exit(0);
		}
	}
}

void ai_serve()
{
	int d = random() % 2;

	ball.dir = d ? UPLEFT: DOWNLEFT;
}

void gpause()
{
	while (1) {
		int c = getchar();
		if (c == 'p' || c == 'P')
			break;
	}
	erase();
	refresh();
}

void draw_ball()
{
	mvprintw(ball.pos.y, ball.pos.x, "o");
}

void draw_users()
{
	int i;
	for (i = 0; i < 3; i++)
	{
		mvprintw(user.pos.y+i, user.pos.x, "#");
		mvprintw(ai.pos.y+i, ai.pos.x, "#");
	}
}

void draw_borders()
{
	int i;

	/* Horizontal */
	mvprintw(1, 0, "+");
	for (i = 1; i < maxx-1; i++)
		mvprintw(1, i, "--");
	mvprintw(1, maxx-1, "+");

	mvprintw(maxy-1, 0, "+");
	for (i = 1; i < maxx-1; i++)
		mvprintw(maxy-1, i, "--");
	mvprintw(maxy-1, maxx-1, "+");

	/* Vertical */
	for (i = 2; i < maxy-2; i++)
		mvprintw(i, 0, "|");
	mvprintw(maxy-2, 0, "|");

	for (i = 2; i < maxy-2; i++)
		mvprintw(i, maxx-1, "|");
	mvprintw(maxy-2, maxx-1, "|");
}

void userctl(int key)
{
	switch (key)
	{
	case 27:
		end = 1;
		break;
	case 'p':
		gpause();
		break;
	case 'w':
		user.pos.y -= (user.pos.y > miy ? 2 : 0);
		break;
	case 's':
		user.pos.y += (user.pos.y < may-2 ? 2 : 0);
		break;
	case '+':
		SLEEPT -= 10000;
		break;
	case '-':
		SLEEPT += 10000;
		break;
	}
}

/* ========================================================================= */

int main()
{
	unsigned int seed = time(NULL);
	srandom(seed);

	init_curses();
	init_game();

	draw_borders();
	draw_ball();
	draw_users();
	draw_info();
	refresh();
	user_serve();

	while (!end)
	{
		erase();
		draw_borders();
		draw_ball();
		draw_users();
		draw_info();
		move_ball();
		refresh();
		userctl(getch());
		usleep(SLEEPT);
	}

	end_curses();
	return 0;
}

