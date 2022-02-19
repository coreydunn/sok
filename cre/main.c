#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include"../src/config.h"

typedef struct Miner
{
	int x;
	int y;
	int life;
} Miner;

const size_t minw=2;
const size_t minh=2;

char map[WIDTH*HEIGHT];

int main(void)
{
	Miner miner;
	size_t h;
	size_t total_places;
	size_t total_things;
	size_t w;

	// Randomize map dimensions
	srand(time(NULL));
	w=rand()%(WIDTH-minw)+minw;
	h=rand()%(HEIGHT-minh)+minh;
	//total_things=rand()%THINGS;
	//total_things=rand()%THINGS;
	total_places=rand()%5;
	total_places=rand()%5;

	fprintf(stderr,"w: %lu\nh: %lu\n",w,h);
	fprintf(stderr,"t: %lu\n",total_things);
	miner=(Miner){
		.x=w/2,
		.y=h/2,
		.life=128,
	};

	// Fill map
	for(size_t i=0;i<w*h;++i)
		map[i]='#';

	for(;miner.life>0;--miner.life)
	{
		size_t dir=rand()%4;
		if(miner.x>0&&miner.x<w&&miner.y>0&&miner.y<h)
			map[miner.y*w+miner.x]=' ';
		else
			miner=(Miner){
				.x=rand()%w,
				.y=rand()%h,
			};

		switch(dir)
		{
			case 0:--miner.x;break;
			case 1:++miner.x;break;
			case 2:--miner.y;break;
			case 3:++miner.y;break;
		}
	}

	// Things
	for(size_t i=0;i<total_things;++i)
	{
		size_t pos;
		do
		{
			pos=(rand()%h)*w+(rand()%w);
			if(map[pos]==' ')
				map[pos]='t';
		}
		while(map[pos]!='t');
	}

	// Places
	for(size_t i=0;i<total_things;++i)
	{
		size_t giveup=w*h;
		size_t pos;

		do
		{
			pos=(rand()%h)*w+(rand()%w);
			if(map[pos]==' ')
				map[pos]='p';
		}
		while(giveup--&&map[pos]!='p');
	}

	// Player
	{
		size_t giveup=w*h;
		size_t pos;

		do
		{
			pos=(rand()%h)*w+(rand()%w);
			if(map[pos]==' ')
				map[pos]='@';
		}
		while(giveup--&&map[pos]!='@');
	}

	// Print map
	for(size_t i=0;i<w*h;++i)
	{
		if(!i)continue;
		if(i%w==0)
			printf("\n");
		else
			printf("%c",map[i]);
	}

	printf("\n");
}
