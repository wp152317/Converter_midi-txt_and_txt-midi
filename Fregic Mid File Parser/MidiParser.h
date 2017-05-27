#pragma once
//Midi File Header
#define MThd "MThd"
//Midi File Track Chunk
#define MTrk "MTrk"
//Midi_Event
typedef struct MIDIEVENT {
	int startTime;
	int isOn;
	int pos;
	int vel;
}mdEv;	
//Notation
typedef struct MIDINOTATION {
	int startTime;
	int length;
	int pos;
	int vel;
}mdNt; 

//.mid -> .txt
int mid_parseNotation(char*);
//.txt -> .mid
int notation_parseMid(char*);