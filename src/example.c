/*
 * Copyright 2017, Adrien Destugues, pulkomandy@pulkomandy.tk
 * Distributed under terms of the MIT license.
 */


#include "devpicolcd.h"

#include <stdio.h>
#include <OS.h>


int main(int argc, char* argv[])
{
	MyLcdDevice* dev = picolcd_open(PICOLCD_20x2);
	dev->init_lcd(dev);
	dev->clear(dev);

	// Setup bargraph chars

	dev->set_char(dev, 0, "\x00\x00\x00\x00\x00\x00\x00\x00");
	dev->set_char(dev, 1, "\x1F\x1F\x00\x00\x00\x00\x00\x00");
	dev->set_char(dev, 2, "\x00\x00\x00\x1F\x1F\x00\x00\x00");
	dev->set_char(dev, 3, "\x1F\x1F\x00\x1F\x1F\x00\x00\x00");
	dev->set_char(dev, 4, "\x00\x00\x00\x00\x00\x00\x1F\x1F");
	dev->set_char(dev, 5, "\x1F\x1F\x00\x00\x00\x00\x1F\x1F");
	dev->set_char(dev, 6, "\x00\x00\x00\x1F\x1F\x00\x1F\x1F");
	dev->set_char(dev, 7, "\x1F\x1F\x00\x1F\x1F\x00\x1F\x1F");

	// To know CPU count
	system_info info;
	get_system_info(&info);

	cpu_info cpus[info.cpu_count];
	bigtime_t t[info.cpu_count];
	get_cpu_info(0, info.cpu_count, &cpus[0]);

	for(;;)
	{
		int i, j;
		for (i = 0; i < info.cpu_count; i++)
			t[i] = cpus[i].active_time;

		usleep(40000);

		get_cpu_info(0, info.cpu_count, &cpus[0]);

		char str[21];
		memset(str, 8, 20);
		str[20] = 0;

		int shift = 0;
		int line = 0;
		for (i = 0; i < info.cpu_count; i++) {
			int load = (cpus[i].active_time - t[i]) / 2000;
			//printf("%d: %lld %lld %d   ", i, cpus[i].active_time, t[i], load);

			for (j = 0; j < 20; j++) {
				if (j < load)
					str[j] |= 1 << shift;
				else
					str[j] &= ~(1 << shift);
			}

			shift++;
			if (shift >= 3) {
				dev->display(dev, line++, 0, str);
				shift = 0;
				memset(str, 8, 20);
			}
		}

		//puts("");

		// Display any leftover partial line
		if (shift != 0)
			dev->display(dev, line++, 0, str);
	}
}
