// (c) Copyright 2018, Sean Connelly (@voidqk), http://sean.cm
// MIT License
// Project Home: https://github.com/voidqk/basicmidi

#include <stdio.h>
#include <string.h>
#include "basicmidi.h"

static enum {
	MODE_ALL,
	MODE_WARN,
	MODE_EV
} mode = MODE_ALL;

static void onevent(bm_delta_ev_st event, void *user){
	if (mode != MODE_ALL && mode != MODE_EV)
		return;
	switch (event.ev.type){
		case BM_EV_RESET:
			printf("[%4d] {*} RESET    %d\n", event.delta, event.ev.u.reset);
			break;
		case BM_EV_TEMPO:
			printf("[%4d] {*} TEMPO    %d\n", event.delta, event.ev.u.tempo);
			break;
		case BM_EV_MASTVOL:
			printf("[%4d] {*} MASTVOL  %d\n", event.delta, event.ev.u.mastvol);
			break;
		case BM_EV_MASTPAN:
			printf("[%4d] {*} MASTPAN  %d\n", event.delta, event.ev.u.mastpan);
			break;
		case BM_EV_NOTEON:
			printf("[%4d] {%X} NOTEON   %d %d\n", event.delta, event.ev.u.noteon.channel,
				event.ev.u.noteon.note, event.ev.u.noteon.velocity);
			break;
		case BM_EV_NOTEOFF:
			printf("[%4d] {%X} NOTEOFF  %d\n", event.delta, event.ev.u.noteoff.channel,
				event.ev.u.noteoff.note);
			break;
		case BM_EV_PEDALON:
			printf("[%4d] {%X} PEDALON  %d\n", event.delta, event.ev.u.pedalon.channel,
				event.ev.u.pedalon.pedal);
			break;
		case BM_EV_PEDALOFF:
			printf("[%4d] {%X} PEDALOFF %d\n", event.delta, event.ev.u.pedaloff.channel,
				event.ev.u.pedaloff.pedal);
			break;
		case BM_EV_CHANVOL:
			printf("[%4d] {%X} CHANVOL  %d\n", event.delta, event.ev.u.chanvol.channel,
				event.ev.u.chanvol.vol);
			break;
		case BM_EV_CHANPAN:
			printf("[%4d] {%X} CHANPAN  %d\n", event.delta, event.ev.u.chanpan.channel,
				event.ev.u.chanpan.pan);
			break;
		case BM_EV_PATCH:
			printf("[%4d] {%X} PATCH    %d # %s\n", event.delta, event.ev.u.patch.channel,
				event.ev.u.patch.patch, bm_patchstr(event.ev.u.patch.patch));
			break;
		case BM_EV_BEND:
			printf("[%4d] {%X} BEND     %d\n", event.delta, event.ev.u.bend.channel,
				event.ev.u.bend.bend);
			break;
		case BM_EV_MOD:
			printf("[%4d] {%X} MOD      %d\n", event.delta, event.ev.u.mod.channel, event.ev.u.mod.mod);
			break;
	}
}

static void onwarn(const char *msg, void *user){
	if (mode != MODE_ALL && mode != MODE_WARN)
		return;
	printf("WARNING: %s\n", msg);
}

static void printhelp(){
	printf(
		"BasicMidi v1.0\n"
		"Copyright (c) 2018 Sean Connelly (@voidqk), MIT License\n"
		"https://github.com/voidqk/basicmidi  http://sean.cm\n\n"
		"Usage:\n"
		"  basicmidi [-w|-e] input.midi\n\n"
		"Where:\n"
		"  -w   Only print warnings\n"
		"  -e   Only print events\n"
		"  --   Default, print both warnings and events\n");
}

int main(int argc, char **argv){
	if (argc <= 1){
		printhelp();
		return 0;
	}

	const char *file = argv[1];
	if (strcmp(file, "-w") == 0 || strcmp(file, "-e") == 0 || strcmp(file, "--") == 0){
		if (strcmp(file, "-w") == 0)
			mode = MODE_WARN;
		else if (strcmp(file, "-e") == 0)
			mode = MODE_EV;
		if (argc <= 2){
			printhelp();
			return 1;
		}
		file = argv[2];
	}

	// read entire file
	FILE *fp = fopen(file, "rb");
	if (fp == NULL){
		fprintf(stderr, "Failed to open file: %s\n", file);
		return 1;
	}
	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	uint8_t *data = malloc(sizeof(uint8_t) * size);
	if (data == NULL){
		fprintf(stderr, "Out of memory\n");
		fclose(fp);
		return 1;
	}
	if (fread(data, 1, size, fp) != size){
		fprintf(stderr, "Failed to read all of file\n");
		fclose(fp);
		return 1;
	}
	fclose(fp);

	// process file
	bm_readmidi(data, size, onevent, onwarn, NULL);

	return 0;
}
