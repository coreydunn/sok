#include<fcntl.h>
#include<ncurses.h>
#include<ctype.h>
#include<signal.h>
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include"config.h"
#include"../maps/maps.h"

extern int tiger;
extern char includefile;
extern char includefilesize;

typedef struct Coord
{
	bool exists;
	int status;
	size_t x;
	size_t y;
} Coord;

Coord pl;
Coord place[THINGS];
Coord thing[THINGS];
size_t total_places=0;
size_t total_things=0;

char map[WIDTH*HEIGHT];
bool complete=false;
bool win=false;
bool file_is_output=false;
size_t level=1;

int map_file(char*filen);
void load_level(size_t l);
void output_file(char*fname);
void reset(void);
void sh(int sig);

int main(int argc,char**argv)
{
	bool running=true;
	bool ascii_level=false;
	char inp;

	if(argc>1)
	{
		if(strcmp(argv[1],"--help")==0)
		{
			fprintf(stderr,"usage: %s [LEVEL]\n",*argv);
			exit(0);
		}

		else if(!isdigit(argv[1][0]))
		{
			//fprintf(stderr,"loading '%s'\n",argv[1]);
			if(map_file(argv[1])==0)
			{
				ascii_level=true;
				level=9999;
			}
		}

		else
		{
			size_t lev=atoll(argv[1]);
			if(lev>0)
				level=lev;
		}
	}

	get_map(0);

	// Init ncurses
	initscr();
	signal(SIGINT,sh);
	curs_set(0);
	noecho();
	if(has_colors())
	{
		start_color();
		use_default_colors();
		init_pair(1,COLOR_WHITE,-1);
		init_pair(2,COLOR_GREEN,COLOR_GREEN);
		init_pair(3,COLOR_RED,COLOR_RED);
		init_pair(4,COLOR_YELLOW,COLOR_YELLOW);
		init_pair(5,COLOR_WHITE,COLOR_WHITE);
	}
	attron(COLOR_PAIR(1));

	// Initialize things
	srand(time(NULL));
	if(!ascii_level)
		load_level(level);

	while(running)
	{
		// Render
		clear();

		//mvhline(HEIGHT,0,'#',WIDTH);
		//mvvline(0,WIDTH,'#',HEIGHT);
		//mvprintw(HEIGHT,WIDTH,"#");
		for(size_t i=0;i<THINGS;++i)
		{
			if(place[i].exists/*&&!place[i].status*/)
			{
				attron(COLOR_PAIR(4));
				//mvprintw(place[i].y,place[i].x,"%c",place[i].status?'%':'x');
				mvprintw(place[i].y,place[i].x,"%c",'x');
				attron(COLOR_PAIR(1));
			}
		}

		for(size_t i=0;i<THINGS;++i)
		{
			if(thing[i].exists)
			{
				attron(COLOR_PAIR(3));
				mvprintw(thing[i].y,thing[i].x,"o");
				attron(COLOR_PAIR(1));
			}
		}

		if(pl.exists)
		{
			attron(COLOR_PAIR(2));
			mvprintw(pl.y,pl.x,"@");
			attron(COLOR_PAIR(1));
		}
		for(size_t i=0;i<WIDTH*HEIGHT;++i)
			if(map[i])
			{
				attron(COLOR_PAIR(5));
				mvprintw(i/WIDTH,i%WIDTH,"%c",'#');
				attron(COLOR_PAIR(1));
			}

		// Display complete message
		{
			size_t j=0;

			if(!complete)
			{
				for(size_t i=0;i<THINGS;++i)
					if(place[i].exists/*&&place[i].status*/)
						for(size_t k=0;k<THINGS;++k)
							if(thing[k].exists&&thing[k].x==place[i].x&&thing[k].y==place[i].y)
								++j;
				if(j>=total_places)
					complete=true;
			}

			if(complete)
			{
				mvprintw(0,WIDTH+2,"Level %u complete!",level);
				mvprintw(1,WIDTH+2,"Level: %lu/%lu",level,LEVELS);
				mvprintw(2,WIDTH+2,"Done: %lu/%lu",j,total_things);
				if(level<LEVELS)
					mvprintw(3,WIDTH+2,"press n for next level");
			}
			else
			{
				mvprintw(1,WIDTH+2,"Level: %lu/%lu",level,LEVELS);
				mvprintw(2,WIDTH+2,"Done: %lu/%lu",j,total_things);
			}
		}

		if(win)
		{
			if(!file_is_output)
			{
				output_file("win.dat");
				file_is_output=true;
			}
			mvprintw(5,5,"win");
		}

		refresh();
		inp=getch();
		refresh();

		switch(inp)
		{

			case 'q':
				running=0;
				break;

			case 'n':
				if(complete)
				{
					++level;
					if(level>=LEVELS)
					{
						reset();
						win=true;
					}
					else
						load_level(level);
				}
				break;

			case 'r':
				load_level(level);
				break;

			case 'j':
				if(pl.y<HEIGHT-1)
				{
					if(map[(pl.y+1)*WIDTH+(pl.x)])
						goto end;
					for(size_t i=0;i<THINGS;++i)
					{
						if(thing[i].exists&&pl.x==thing[i].x &&
								pl.y+1==thing[i].y)
							if(thing[i].y<HEIGHT-1&&thing[i].exists)
							{
								bool thing_thing_collide=false;
								bool thing_map_collide=false;
								for(size_t j=0;j<THINGS;++j)
									if(thing[j].exists&&thing[i].x==thing[j].x&&
											thing[i].y+1==thing[j].y&&i!=j)
										thing_thing_collide=true;
								if(map[(thing[i].y+1)*WIDTH+(thing[i].x)])
									thing_map_collide=true;
								if(!thing_thing_collide&&!thing_map_collide)
									++thing[i].y;
								else
									goto end;
							}
							else
								goto end;
					}
					++pl.y;
				}
				break;

			case 'k':
				if(pl.y>0)
				{
					if(map[(pl.y-1)*WIDTH+(pl.x)])
						goto end;
					for(size_t i=0;i<THINGS;++i)
					{
						if(thing[i].exists&&pl.x==thing[i].x &&
								pl.y-1==thing[i].y)
							if(thing[i].y>0&&thing[i].exists)
							{
								bool thing_thing_collide=false;
								bool thing_map_collide=false;
								for(size_t j=0;j<THINGS;++j)
									if(thing[i].x==thing[j].x&&
											thing[i].y-1==thing[j].y&&i!=j)
										thing_thing_collide=true;
								if(map[(thing[i].y-1)*WIDTH+(thing[i].x)])
									thing_map_collide=true;
								if(!thing_thing_collide&&!thing_map_collide)
									--thing[i].y;
								else
									goto end;
							}
							else
								goto end;
					}
					--pl.y;
				}
				break;

			case 'h':
				if(pl.x>0)
				{
					if(map[(pl.y)*WIDTH+(pl.x-1)])
						goto end;
					for(size_t i=0;i<THINGS;++i)
					{
						if(thing[i].exists&&pl.x-1==thing[i].x &&
								pl.y==thing[i].y)
							if(thing[i].x>0&&thing[i].exists)
							{
								bool thing_thing_collide=false;
								bool thing_map_collide=false;
								for(size_t j=0;j<THINGS;++j)
									if(thing[i].x-1==thing[j].x&&
											thing[i].y==thing[j].y&&i!=j)
										thing_thing_collide=true;
								if(map[(thing[i].y)*WIDTH+(thing[i].x-1)])
									thing_map_collide=true;
								if(!thing_thing_collide&&!thing_map_collide)
									--thing[i].x;
								else
									goto end;
							}
							else
								goto end;
					}
					--pl.x;
				}
				break;

			case 'l':
				if(pl.x<WIDTH-1)
				{
					if(map[(pl.y)*WIDTH+(pl.x+1)])
						goto end;
					for(size_t i=0;i<THINGS;++i)
					{
						if(thing[i].exists&&pl.x+1==thing[i].x &&
								pl.y==thing[i].y)
							if(thing[i].x<WIDTH-1&&thing[i].exists)
							{
								bool thing_thing_collide=false;
								bool thing_map_collide=false;
								for(size_t j=0;j<THINGS;++j)
									if(thing[j].exists&&thing[i].x+1==thing[j].x&&
											thing[i].y==thing[j].y&&i!=j)
										thing_thing_collide=true;
								if(map[(thing[i].y)*WIDTH+(thing[i].x+1)])
									thing_map_collide=true;
								if(!thing_thing_collide&&!thing_map_collide)
									++thing[i].x;
								else
									goto end;
							}
							else
								goto end;
					}
					++pl.x;
				}
				break;

			//case 'o':
				//output_file("output");
				//break;

end:
				break;
		}

		for(size_t i=0;i<THINGS;++i)
		{
			for(size_t j=0;j<THINGS;++j)
				if(thing[i].exists&&place[j].exists&&thing[i].x==place[j].x&&thing[i].y==place[j].y)
				{
					thing[i].status=1;
					//place[j].status=1;
				}
		}

	}

	//sleep(2);

quit:
	curs_set(1);
	echo();
	endwin();
}

void sh(int sig)
{
	curs_set(1);
	echo();
	endwin();
}

int map_file(char*filen)
{
	FILE*f=fopen(filen,"r");
	total_things=0;

	if(!f)return 1;
	for(size_t i=0;i<WIDTH*HEIGHT;++i)
	{
		char b;
		fread(&b,1,1,f);
		if(b=='\n')
		{
			i+=WIDTH-i%WIDTH-1;
			continue;
		}
		else
		{

			if(b=='t'&&total_things<THINGS)
			{
				thing[total_things++]=(Coord){
					.exists=true,
					.x=i%WIDTH,
					.y=i/WIDTH,
				};
			}

			if(b=='p'&&total_places<THINGS)
			{
				place[total_places++]=(Coord){
					.exists=true,
					.status=0,
					.x=i%WIDTH,
					.y=i/WIDTH,
				};
			}

			if(b=='@')
			{
				pl.exists=true;
				pl.x=i%WIDTH;
				pl.y=i/WIDTH;
			}
			map[i]=(b=='#');
		}
	}
	fclose(f);
	return 0;
}

int map_data(int l)
{
	size_t offset=0;
	total_things=0;

	if(l>LEVELS)return 1;
	--l;
	for(size_t i=0;mapsb[l]+offset<mapse[l]&&i<WIDTH*HEIGHT;++i)
	{
		char b;
		//fread(&b,1,1,f);
		b=mapsb[l][offset++];
		if(b=='\n')
		{
			i+=WIDTH-i%WIDTH-1;
			continue;
		}
		else
		{

			if(b=='t'&&total_things<THINGS)
			{
				thing[total_things++]=(Coord){
					.exists=true,
					.x=i%WIDTH,
					.y=i/WIDTH,
				};
			}

			if(b=='p'&&total_places<THINGS)
			{
				place[total_places++]=(Coord){
					.exists=true,
					.status=0,
					.x=i%WIDTH,
					.y=i/WIDTH,
				};
			}

			if(b=='@')
			{
				pl.exists=true;
				pl.x=i%WIDTH;
				pl.y=i/WIDTH;
			}
			map[i]=(b=='#');
		}
	}
	return 0;
}

void reset(void)
{
	complete=false;
	win=false;
	file_is_output=false;
	total_places=0;
	total_things=0;
	for(size_t i=0;i<THINGS;++i)
	{
		thing[i]=(Coord){
			.exists=false,
			.status=0,
			.x=0,
			.y=0,
		};

		place[i]=(Coord){
			.exists=false,
			.status=0,
			.x=0,
			.y=0,
		};
	}

	pl=(Coord){
		.status=0,
		.exists=false,
		.x=5,
		.y=6,
	};

	for(size_t i=0;i<WIDTH*HEIGHT;++i)
		map[i]=0;
}

void load_level(size_t l)
{
	char buf[1024];
	reset();
	sprintf(buf,"map%lu.txt",l);
	// Programmatically find map data
	//if(map_file(buf))
	if(map_data(l))
	{
		level=0;
		total_places=9999;
		total_things=9999;
	}
}

void output_file(char*fname)
{
	FILE*f=fopen(fname,"w");
	size_t n=(size_t)&includefilesize-(size_t)&includefile;

	if(!f)
	{
		fprintf(stderr,"error: file '%s' cannot be opened\n",fname);
		return;
	}

	else
	{
		mvprintw(4,0,"WRITING %lu bytes\n",n);
		fwrite(&includefile,1,n,f);
	}
	fclose(f);
}
