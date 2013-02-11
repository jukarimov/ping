#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <curses.h>
#include <assert.h>
#include <time.h>

#define maxx		64
#define maxy		20

#define max		maxx-2
#define may		maxy-2

#define mix		1
#define miy		2

#define minx		0
#define miny		0

#define SERVS		5

unsigned int SLEEPT;

#define ARRAYSIZE(a)	(sizeof(a)/sizeof(a[0]))

int	end;

struct Position {
	int		x;
	int		y;
};

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

int user1_serving;
int user1_serving_now;
int user2_serving_now;
int user_served;

int sockfd;
struct sockaddr_in serv_addr;
struct hostent *server;
char buffer[256];

void setpos(struct Position *p, int x, int y);
void gpause();
void redraw();
void draw_borders();
void draw_ball();
void draw_users();
void draw_info();
void user_serve();
void netset(int port, char *hname);
void peersync();
