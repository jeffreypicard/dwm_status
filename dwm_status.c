/*
 * dwm_status.c
 *
 * dwm status bar.
 *
 * Author: Jeffrey Picard (jeff@jeffreypicard.com)
 */

#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>

#define BUF_SIZE 512

uint64_t prev_total = 0, prev_idle = 0;
unsigned char spotify_found = 0;
Window spotify_window;

int
BadWindow_handler(Display *d, XErrorEvent *e)
{
	switch (e->error_code) {
		case BadWindow:
			spotify_found = 0;
	}

	return 0;
}

void
get_spotify(char *buf)
{
	int i;
	char *name;

	Display *disp;
	Window w;

	Window root_return;
	Window parent_return;
	Window *children;
	unsigned int nchild;

	disp = XOpenDisplay(0);

	if (spotify_found) {
		XFetchName(disp, spotify_window, &name);
		if (name && (strcmp(name, "Spotify") != 0)) {
			snprintf(buf, BUF_SIZE, "%s", name);
		} else {
			sprintf(buf, "N/A");
		}
		XCloseDisplay(disp);
		return;
	}

	w = DefaultRootWindow(disp);

	name = 0;
	children = 0;
	XQueryTree(disp, w, &root_return, &parent_return, &children, &nchild);

	for (i = 0; i < nchild; i++) {
		XFetchName(disp, children[i], &name);
		if (name && !strcmp(name, "Spotify")) {
			spotify_window = children[i];
			spotify_found = 1;
		}
		if (name) XFree(name);
		name = 0;
	}

	if (children) XFree(children);
	
	/* Needed to reset status to N/A after spotify stops */
	if (!spotify_found) sprintf(buf, "N/A");

	XCloseDisplay(disp);
}

void
set_status(char *status)
{
	/* How much point is there do doing error checking on each
	   of these functions?*/
	Display *disp; 

	disp = XOpenDisplay(0);

	XStoreName(disp, DefaultRootWindow(disp), status);
	XSync(disp, 0);
	XCloseDisplay(disp);
}

void
get_bat(char *buf)
{
	uint64_t energy_now, energy_full, percent;
	FILE *f_eng_now, *f_eng_full;
	const char *path_now = "/sys/class/power_supply/BAT0/energy_now";
	const char *path_full = "/sys/class/power_supply/BAT0/energy_full";

	memset(buf, 0, BUF_SIZE);

	if (!(f_eng_now = fopen(path_now, "r"))) goto error;

	if (!(f_eng_full = fopen(path_full, "r"))) goto error;

	if (fscanf(f_eng_now, "%lu", &energy_now) == -1) goto error;

	if (fscanf(f_eng_full, "%lu", &energy_full) == -1) goto error;

	fclose(f_eng_now);
	fclose(f_eng_full);

	percent = (100 * energy_now) / energy_full;

	snprintf(buf, BUF_SIZE, "%lu", percent);

	return;

	error:
	sprintf(buf, "%s", "ERR");
}

void
get_cpu(char *buf)
{
	int i;
	uint64_t percent, diff_total, diff_idle, total_time, idle_time, cpu[10];
	FILE *proc_stat;

	memset(cpu, 0, sizeof(cpu));
	memset(buf, 0, BUF_SIZE);

	if (!(proc_stat = fopen("/proc/stat", "r"))) {
		sprintf(buf, "%s", "ERR");
		return;
	}

	if (fscanf(proc_stat, "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
		&cpu[0], &cpu[1], &cpu[2], &cpu[3], &cpu[4], &cpu[5],
		&cpu[6], &cpu[7], &cpu[8], &cpu[9]) == -1) {

		fclose(proc_stat);
		sprintf(buf, "%s", "ERR");
		return;
	}

	total_time = 0;
	idle_time = 0;
	for (i = 0; i < 10; i++) {
		total_time += cpu[i];
	}
	idle_time = cpu[3];

	diff_total = total_time - prev_total;
	diff_idle = idle_time - prev_idle;

	prev_total = total_time;
	prev_idle = idle_time;

	percent = (100 * (diff_total - diff_idle)) / diff_total;

	snprintf(buf, BUF_SIZE, "%lu", percent);
	fclose(proc_stat);
}

int
main(int argc, char **argv)
{
	/* Sleep for a half second. */
	struct timespec rqtp;
	rqtp.tv_sec = 0;
	rqtp.tv_nsec = 500000000L;

	/* Set error handler so we don't crash when spotify closes */
	XSetErrorHandler(BadWindow_handler);

	/* Maybe don't use static buffers? */
	char cpu[BUF_SIZE], bat[BUF_SIZE], spotify[BUF_SIZE];
	char buf[3 * BUF_SIZE];

	memset(cpu, 0, BUF_SIZE);
	memset(bat, 0, BUF_SIZE);
	memset(spotify, 0, BUF_SIZE);
	
	while (1) {
		nanosleep(&rqtp, 0);

		get_cpu(cpu);
		get_bat(bat);
		get_spotify(spotify);

		memset(buf, 0, 3 * BUF_SIZE);

		snprintf(buf, 
		         3 * BUF_SIZE,
		         "BAT: %s%% | CPU: %s%% | SPOTIFY: %s",
		         bat,
		         cpu,
		         spotify);

		set_status(buf);
	}

	return 0;
}
