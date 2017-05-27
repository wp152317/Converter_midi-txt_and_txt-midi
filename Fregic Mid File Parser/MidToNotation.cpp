#include "MidiParser.h"
#include <stdio.h>
#include <vector>
#include <string.h>
#include <algorithm>
using namespace std;
namespace MidToNotation_nameSpace {
	unsigned char byte[1000000];		//.mid File Data
	//////////////////////////// Header Data
	int header_length;		//header_length
	short format;			//Format
	short numOfTC;			//Track Chunk Number
	short division;			//division
	//////////////////////////// Parsing Data
	vector<mdEv> eventsLog;			//Midi Event
	vector<mdNt> notations;			//Notation
	int nowp;			//Array Byte index
	//Initialize
	void init() {
		nowp = 0;
		char nullArray[100000] = {};
		memcpy(byte, nullArray, 100000);
		header_length = 0;
		format = 0;
		numOfTC = 0;
		division = 0;
		eventsLog.erase(eventsLog.begin(), eventsLog.end());
		notations.erase(notations.begin(), notations.end());
	}
	//Load Mid File
	int load(char *name) {
		FILE *in;
		int ch;
		char str[1000] = {};
		sprintf(str, "%s.mid", name);
		if ((in = fopen(str, "rb")) == NULL) {
			fputs("파일 열기 에러!", stderr);
			return 1;
		}
		/* Reading */ {
			int i = 0;
			while ((ch = fgetc(in)) != EOF) {
				byte[i++] = ch;
			}
		}
		fclose(in); // close
		return 0;
	}
	//Check File is Mid
	int check_head() {
		int result = memcmp(byte, MThd, 4); header_length = 0; nowp = 4;
		for (; nowp < 8; nowp++) {
			header_length *= 256;
			header_length += byte[nowp];
		}format = 0;
		for (; nowp < 10; nowp++) {
			format *= 256;
			format += byte[nowp];
		}numOfTC = 0;
		for (; nowp < 12; nowp++) {
			numOfTC *= 256;
			numOfTC += byte[nowp];
		}division = 0;
		for (; nowp < 14; nowp++) {
			division *= 256;
			division += byte[nowp];
		}
		return result;
	}
	//Delete Meta Data
	void throwMetaData() {
		nowp++;
		//nowp += byte[nowp]+1;
		int l = 0;
		while (byte[nowp] > 0x80) {
			l += byte[nowp++];
			l *= 128;
		}l += byte[nowp++];
		nowp += l;
	}
	//Delete System Data
	void throwSysData() {
		int l = 0;
		while (byte[nowp] > 0x80) {
			l += byte[nowp++];
			l *= 128;
		}l += byte[nowp++];
		nowp += l;
	}
	//Check Midi Event
	void checkMidiEvent(const int time) {
		switch (byte[nowp++]) {
		case 0xB0:			//par
			nowp += 2;
			break;
		case 0xC0:			//prch
			nowp++;
			break;
		case 0x90: {			//On
			int key = byte[nowp++];
			int vel = byte[nowp++];
			eventsLog.push_back({ time, 1,key,vel });
		}
				   break;
		case 0x80: {			//Off
			int key = byte[nowp++];
			int vel = byte[nowp++];
			eventsLog.push_back({ time, 0,key,vel });
		}
				   break;
		}
	}
	//Read Track Chunk
	void read_Chunk(int nowLength) {
		int time = 0, k = 0;
		while (nowp < nowLength) {
			if (byte[nowp] > 0x80) {
				k += byte[nowp++] - 0x80;
				k *= 128;
			}
			else {
				k += byte[nowp++];
				time += k;
				k = 0;
				switch (byte[nowp++]) {
				case 0xFF: 
					throwMetaData();
					break;
				case 0xF0:
				case 0xF7:
					throwSysData();
						   break;
				default: {
					nowp--;
					checkMidiEvent(time);
				}
				}
			}
		}
		//printf("%d\n", nowp);
	}
	//Send Track Chunk
	int getTrackChunk() {
		for (int i = 0; i < numOfTC; i++) {
			if (memcmp(MTrk, byte + nowp, 4))return 1;
			nowp += 4;
			int nowLength = 0;
			for (int j = 0; j < 4; j++) {
				nowLength *= 256;
				nowLength += byte[nowp++];
			}
			read_Chunk(nowLength + nowp);
		}
		return 0;
	}
	//Compare for Sort - used buildNotation
	bool compareNotation(mdNt desc,mdNt src) {
		return desc.startTime < src.startTime;
	}
	//Build Notation With EventLog
	void buildNotation() {
		vector<int> onNum[128];
		for (int i = 0,n=eventsLog.size(); i < n; i++) {
			if (eventsLog[i].isOn)
				onNum[eventsLog[i].pos].push_back(i);
			else {
				if (onNum[eventsLog[i].pos].empty()) {
					printf("Error!\n");
					return;
				}
				int past=onNum[eventsLog[i].pos][onNum[eventsLog[i].pos].size() - 1];
				notations.push_back({eventsLog[past].startTime,
									 eventsLog[i].startTime-eventsLog[past].startTime,
									 eventsLog[i].pos-20,
									 eventsLog[past].vel});
			}
		}
		sort(notations.begin(), notations.end(), compareNotation);
		for (int i = notations.size() - 1; i > 0; i--) {
			notations[i].startTime -= notations[i-1].startTime;
		}
	}
	///Test Print Information of Notation
	void printNotationInform() {
		for (int i = 0, n = notations.size(); i < n; i++) {
			printf("%-7d %-7d %2d %3d\n", notations[i].startTime, notations[i].length, notations[i].pos, notations[i].vel);
		}
	}
	//Make .txt File
	void write(char* name) {
		char str[1000];
		sprintf(str, "%s.txt", name);
		FILE* out = fopen(str, "wt");
		for (auto i : notations) {
			fprintf(out, "%d %d %d %d\n", i.startTime,i.length,i.pos,i.vel);
		}
		fclose(out);
	}
}
using namespace MidToNotation_nameSpace;
int mid_parseNotation(char* name) {
	init();
	if(load(name))return 1;
	if (check_head())return 2;
	if (getTrackChunk())return 2;
	buildNotation();
	///printNotationInform();
	write(name);
	printf("%s to %s - Notation.txt is Completed\n",name,name);
	return 0;
}
