#include "ping.h"

void setpos(struct Position *p, int x, int y)
{
	p->x = x;
	p->y = y;
}

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
	setpos(&ball.pos, mix+4, maxy/2);

	setpos(&user2.pos, max-3, maxy/2);

	user.score = 0;
	user2.score = 0;
}

void user_score()
{
	int x = ball.pos.x;

	if (x > maxx)
		++user.score;
	else
		++user2.score;
}

void serve()
{
	if (++user_served == SERVS)
	{
		user1_serving = user1_serving ? 0 : 1;
		user_served = 0;
	}

	if (user1_serving)
		user_serve();
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

	if ((y >= user.pos.y && y <= user.pos.y+2) && x == user.pos.x)
	{
		if (ball.dir == DOWNLEFT)
			ball.dir = DOWNRIGHT;
		else
			ball.dir = UPRIGHT;
	}

	if ((y >= user2.pos.y && y <= user2.pos.y+2) && x == user2.pos.x)
	{
		if (ball.dir == UPRIGHT)
			ball.dir = DOWNLEFT;
		else
			ball.dir = UPLEFT;
	}
}

void user_serve()
{
	setpos(&user.pos, mix+3, maxy/2);
	setpos(&ball.pos, mix+4, maxy/2);

	redraw();

	user1_serving_now = 1;
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
		mvprintw(user.pos.y+i, user.pos.x, "1");
		mvprintw(user2.pos.y+i, user2.pos.x, "2");
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
	if (user1_serving_now)
	{
		while (1)
		{
			if (key == 'w')
			{
				ball.dir = UPRIGHT;
				user1_serving_now = 0;
				break;
			}
			if (key == 's')
			{
				ball.dir = DOWNRIGHT;
				user1_serving_now = 0;
				break;
			}
			key = getch();
		}
	}

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
	}
}

void error(const char *msg)
{
    game_finish();
    perror(msg);
    exit(0);
}

void netset(int port, char *hname)
{
    int r;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    server = gethostbyname(hname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(port);
    r = connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    if (r != 0)
        error("ERROR connecting");
}

void peersync()
{
    int n;
    bzero(buffer, 256);
    sprintf(buffer, "%d %d", user.pos.x, user.pos.y);
    n = send(sockfd, buffer, strlen(buffer), 0);
    if (n < 0) 
	error("ERROR writing to socket");

    bzero(buffer, 256);
    sprintf(buffer, "%d %d", ball.pos.x, ball.pos.y);
    n = send(sockfd, buffer, strlen(buffer), 0);
    if (n < 0) 
	error("ERROR writing to socket");

    bzero(buffer,256);
    n = recv(sockfd, buffer, 255, 0);
    if (n < 0) 
	 error("ERROR reading from socket");

    char *str = strdup(buffer);

    char *p = strtok(str, " ");
    user2.pos.x = atoi(p);

    p = strtok(NULL, " ");
    user2.pos.y = atoi(p);
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

int main(int argc, char *argv[])
{
	int   serverport;
	char *serveraddr;

	if (argc >= 3) {
		serveraddr = argv[1];
		serverport = atoi(argv[2]);
	} else {
		puts("Usage: ping [server] [port]");
		exit(0);
	}

	unsigned int seed = time(NULL);
	srandom(seed);

	SLEEPT = 125000;
	user1_serving = 1;
	user1_serving_now = 1;
	user_served = 0;

	init_curses();
	init_game();

	netset(serverport, serveraddr);
	redraw();
	serve();

	while (!end)
	{
		redraw();

		userctl(getch());

		peersync();

		move_ball();

		redraw();

		usleep(SLEEPT);
	}

	end_curses();
	return 0;
}

