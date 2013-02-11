#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>

#define maxx		64
#define maxy		20

#define max		maxx-2
#define may		maxy-2

#define mix		1
#define miy		2

#define minx		0
#define miny		0

#define SERVS		5

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
};

struct Ball ball;

struct Paddle {
	struct Position pos;
	int		score;
};

struct Paddle user;
struct Paddle user2;

#define LEN(a)	((sizeof(a)/sizeof(*a)))

int user1_serving	= 1;
int user_served		= 0;

void gpause();
void redraw();
void draw_borders();
void draw_ball();
void draw_users();
void draw_info();
void user_serve();
void user2_serve();

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
	setpos(&user2.pos, max-3, maxy/2);
	setpos(&ball.pos, mix+4, maxy/2);

	user.score = 0;
	user2.score = 0;
}

void user_score()
{
	int x = ball.pos.x;

	if (x < minx)
		++user2.score;
	else
	if (x > maxx)
		++user.score;
}

void serve()
{
	setpos(&user.pos, mix+3, maxy/2);
	setpos(&user2.pos, max-3, maxy/2);

	redraw();

	if (++user_served == SERVS)
	{
		user1_serving = user1_serving ? 0 : 1;
		user_served = 0;
	}

	if (user1_serving)
		user_serve();
	else
		user2_serve();
}

void game_finish()
{
	end_curses();
}

void draw_info()
{
	char res[10];
	int n = user.score;

	sprintf(res, "%d", n);
	mvprintw(0, 1, res);

	sprintf(res, "%d", user2.score);
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

	setpos(&ball.pos, x, y);

	if (x < minx || x > maxx)
	{
		user_score();
		serve();
		return;
	}

	if (y <= miy)
	{
		if (ball.dir == UPRIGHT)
			ball.dir = DOWNRIGHT;
		else
			ball.dir = DOWNLEFT;
	}

	if (y >= may) {
		if (ball.dir == DOWNRIGHT)
			ball.dir = UPRIGHT;
		else
			ball.dir = UPLEFT;
	}

	if ((y >= user2.pos.y && y <= user2.pos.y+2) && x == user2.pos.x)
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

}

void user_serve()
{
	setpos(&ball.pos, mix+4, maxy/2);
	redraw();

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

void user2_serve()
{
	setpos(&ball.pos, max-4, maxy/2);
	redraw();

	while (1) {
		int c = getchar();
		if (c == 'o' || c == 'l') {
			ball.dir = c == 'o' ? UPLEFT : DOWNLEFT;
			break;
		}
		else if (c == 27)
		{
			game_finish();
			exit(0);
		}
	}
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
		mvprintw(user2.pos.y+i, user2.pos.x, "#");
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
	case 'o':
		user2.pos.y -= (user2.pos.y > miy ? 2 : 0);
		break;
	case 'l':
		user2.pos.y += (user2.pos.y < may-2 ? 2 : 0);
		break;
	case '+':
		SLEEPT -= 10000;
		break;
	case '-':
		SLEEPT += 10000;
		break;
	}
}

void redraw()
{
	erase();
	draw_borders();
	draw_ball();
	draw_users();
	draw_info();
	refresh();
}

/* ========================================================================= */

int main()
{
	unsigned int seed = time(NULL);
	srandom(seed);

	init_curses();
	init_game();

	redraw();
	user_serve();

	while (!end)
	{
		move_ball();
		redraw();
		userctl(getch());
		usleep(SLEEPT);
	}

	end_curses();
	return 0;
}

