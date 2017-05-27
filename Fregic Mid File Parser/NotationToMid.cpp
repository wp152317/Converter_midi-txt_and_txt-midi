#include "MidiParser.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>
using namespace std;
namespace NotationToMid {
	unsigned char byte[100000] = {};	//.mid File Data
	int nowp = 0;						//Array Byte index
	vector<mdEv> eventsLog;				//Midi Event
	vector<mdNt> notations;				//Notaion
	//Initialize
	void init() {
		char nullArray[100000] = {};
		memcpy(byte, nullArray, 100000);
		nowp = 0;
		eventsLog.erase(eventsLog.begin(), eventsLog.end());
		notations.erase(notations.begin(), notations.end());
	}
	//Load .txt File
	void load(char* name) {		//Name.mid - Notation.txt
		char str[10000] = {};
		sprintf(str, "%s.txt",name);
		FILE *in = fopen(str, "rt");
		int d1, d2, d3, d4;
		while (fscanf(in, "%d %d %d %d", &d1, &d2, &d3, &d4) != EOF) {
			notations.push_back({ d1,d2,d3+20,d4 });
		}
		fclose(in);
	}
	//Preparing convert Notation to EventsLog
	void deltaTimeToAbTime() {
		for (int i = 0, n = notations.size() - 1; i < n; i++) 
			notations[i + 1].startTime += notations[i].startTime;
	}
	//Compare for Sort - used buildEventLog
	bool compareEventsLog(mdEv desc,mdEv src) {
		return desc.startTime < src.startTime;
	}
	//Build EventLog with Notation
	void buildEventLog() {
		for (auto i : notations) {
			eventsLog.push_back({i.startTime,1,i.pos,i.vel});		//Add On
			eventsLog.push_back({i.startTime + i.length,0,i.pos,0});		//Add Off
		}
		sort(eventsLog.begin(), eventsLog.end(), compareEventsLog);
		for (int i = eventsLog.size()-1;i > 0; i--) 
			eventsLog[i].startTime -= eventsLog[i - 1].startTime;
	}
	//Write .mid File Header
	void writeHead() {
		unsigned char str[1000] = "MThd";
		for (int i = 0; str[i]; i++)
			byte[nowp++] = str[i];
		str[0] = 0, str[1] = 0, str[2] = 0, str[3] = 6, str[4] = 0,
			str[5] = 1, str[6] = 0, str[7] = 2, str[8] = 3, str[9] = 0xC0, str[10] = 0;
		for (int i = 0; i < 10; i++)
			byte[nowp++] = str[i];
	}
	//Write .mid File First Track Chunk(Meta Data)
	void writeDumyChunk() {		//Meta Data Chunk
		char str[1000] = "MTrk";
		for (int i = 0; str[i]; i++)
			byte[nowp++] = str[i];
		int tmpNowP=0;
		unsigned char tmpByte[50000] = {};
		unsigned char tmp[100] = { 0x00, 0xFF, 0x58, 0x04 };
		for (int i = 0; i < 4; i++)
			tmpByte[tmpNowP++] = tmp[i];
		tmp[0] = 4, tmp[1] = 2, tmp[2] = 0x18, tmp[3] = 8;
		for (int i = 0; i < 4; i++)
			tmpByte[tmpNowP++] = tmp[i];
		tmp[0] = 0x00, tmp[1] = 0xFF, tmp[2] = 0x59, tmp[3] = 0x02;
		for (int i = 0; i < 4; i++)
			tmpByte[tmpNowP++] = tmp[i];
		tmp[0] = 0xFE, tmp[1] = 0x01;
		for (int i = 0; i < 2; i++)
			tmpByte[tmpNowP++] = tmp[i];
		tmp[0] = 0, tmp[1] = 0xFF, tmp[2] = 3, tmp[3] = 9;
		for (int i = 0; i < 4; i++)
			tmpByte[tmpNowP++] = tmp[i];
		strcpy((char*)tmp, "Annonymus");
		for (int i = 0; i < 9; i++)
			tmpByte[tmpNowP++] = tmp[i];
		tmp[0] = 0, tmp[1] = 0xFF, tmp[2] = 1, tmp[3] = 0x11;
		for (int i = 0; i < 4; i++)
			tmpByte[tmpNowP++] = tmp[i];
		strcpy((char*)tmp, "Created by Fregic");
		for (int i = 0; i < 17; i++)
			tmpByte[tmpNowP++] = tmp[i];
		tmp[0] = 00, tmp[1] = 0xFF, tmp[2] = 0x02, tmp[3] = 0x0C;
		for (int i = 0; i < 4; i++)
			tmpByte[tmpNowP++] = tmp[i];
		tmp[0] = 0x43, tmp[1] = 0x6F, tmp[2] = 0x70, tmp[3] = 0x79, tmp[4] = 0x72, tmp[5] = 0x69
			, tmp[6] = 0x67, tmp[7] = 0x68, tmp[8] = 0x74, tmp[9] = 0x20, tmp[10] = 0xA9, tmp[11] = 0x20;
		for (int i = 0; i < 11; i++)
			tmpByte[tmpNowP++] = tmp[i];
		tmp[0] = 0x9A, tmp[1] = 0x20, tmp[2] = 0xFF, tmp[3] = 0x51, tmp[4] = 0x03
			, tmp[5] = 0x06, tmp[6] = 0xD5, tmp[7] = 0x11;
		for (int i = 0; i < 8; i++)
			tmpByte[tmpNowP++] = tmp[i];
		tmp[0] = 0x01, tmp[1] = 0xFF, tmp[2] = 0x2F, tmp[3] = 0x00;
		for (int i = 0; i < 4; i++)
			tmpByte[tmpNowP++] = tmp[i];
		byte[nowp++] = tmpNowP / 256 / 256 / 256;
		byte[nowp++] = tmpNowP / 256 / 256 % 256;
		byte[nowp++] = tmpNowP / 256 % 256;
		byte[nowp++] = tmpNowP % 256;
		for (int i = 0; i < tmpNowP; i++)
			byte[nowp++] = tmpByte[i];
	}
	//Wrtie .mid File Midi Event
	void writeRealDataChunk() {
		char str[1000] = "MTrk";
		for (int i = 0; str[i]; i++)
			byte[nowp++] = str[i];
		int tmpNowP = 0;
		unsigned char tmpByte[200000] = {};
		int sTime = eventsLog[0].startTime;
		eventsLog[0].startTime = 0;
		do {
			if (sTime < 128) {
				tmpByte[tmpNowP++] = sTime;
			}
			else {
				int t = sTime;
				while (t > 128)t /= 128;
				tmpByte[tmpNowP++] = t + 0x80;
			}
			sTime /= 128;
		} while (sTime);
		/* Par & Prch & Meta */ {
			tmpByte[tmpNowP++] = 0xC0;tmpByte[tmpNowP++] = 0x00;tmpByte[tmpNowP++] = 0x00;tmpByte[tmpNowP++] = 0xB0;tmpByte[tmpNowP++] = 0x79;
			tmpByte[tmpNowP++] = 0x00;tmpByte[tmpNowP++] = 0x00;tmpByte[tmpNowP++] = 0xB0;tmpByte[tmpNowP++] = 0x40;tmpByte[tmpNowP++] = 0x00;
			tmpByte[tmpNowP++] = 0x00;tmpByte[tmpNowP++] = 0xB0;tmpByte[tmpNowP++] = 0x5B;tmpByte[tmpNowP++] = 0x30;tmpByte[tmpNowP++] = 0x00;
			tmpByte[tmpNowP++] = 0xB0;tmpByte[tmpNowP++] = 0x0A;tmpByte[tmpNowP++] = 0x33;tmpByte[tmpNowP++] = 0x00;tmpByte[tmpNowP++] = 0xB0;
			tmpByte[tmpNowP++] = 0x07;tmpByte[tmpNowP++] = 0x64;tmpByte[tmpNowP++] = 0x00;tmpByte[tmpNowP++] = 0xB0;tmpByte[tmpNowP++] = 0x79;
			tmpByte[tmpNowP++] = 0x00;tmpByte[tmpNowP++] = 0x00;tmpByte[tmpNowP++] = 0xB0;tmpByte[tmpNowP++] = 0x40;tmpByte[tmpNowP++] = 0x00;
			tmpByte[tmpNowP++] = 0x00;tmpByte[tmpNowP++] = 0xB0;tmpByte[tmpNowP++] = 0x5B;tmpByte[tmpNowP++] = 0x30;tmpByte[tmpNowP++] = 0x00;
			tmpByte[tmpNowP++] = 0xB0;tmpByte[tmpNowP++] = 0x0A;tmpByte[tmpNowP++] = 0x33;tmpByte[tmpNowP++] = 0x00;tmpByte[tmpNowP++] = 0xB0;
			tmpByte[tmpNowP++] = 0x07;tmpByte[tmpNowP++] = 0x64;tmpByte[tmpNowP++] = 0x00;tmpByte[tmpNowP++] = 0xFF;tmpByte[tmpNowP++] = 0x03;
			tmpByte[tmpNowP++] = 0x07;tmpByte[tmpNowP++] = 0x4B;tmpByte[tmpNowP++] = 0x6C;tmpByte[tmpNowP++] = 0x61;tmpByte[tmpNowP++] = 0x76;
			tmpByte[tmpNowP++] = 0x69;tmpByte[tmpNowP++] = 0x65;tmpByte[tmpNowP++] = 0x72;
		}
		for (int i = 0, n = eventsLog.size(); i < n; i++) {
			sTime = eventsLog[i].startTime;
			do {
				if (sTime < 128) {
					tmpByte[tmpNowP++] = sTime;
				}
				else {
					int t = sTime;
					while (t > 128)t /= 128;
					tmpByte[tmpNowP++] = t + 0x80;
				}
				sTime /= 128;
			} while (sTime);
			tmpByte[tmpNowP++] = eventsLog[i].isOn ? 0x90 : 0x80;
			tmpByte[tmpNowP++] = eventsLog[i].pos;
			tmpByte[tmpNowP++] = eventsLog[i].vel;
		}
		tmpByte[tmpNowP++] = 0x01; tmpByte[tmpNowP++] = 0xFF; tmpByte[tmpNowP++] = 0x2F; tmpByte[tmpNowP++] = 0x00;
		int length = tmpNowP;
		byte[nowp++] = length / 256 / 256 / 256;
		byte[nowp++] = length / 256 / 256 % 256;
		byte[nowp++] = length / 256 % 256;
		byte[nowp++] = length % 256;
		for (int i = 0; i < tmpNowP; i++) {
			byte[nowp++] = tmpByte[i];
		}
	}
	//Build .mid File Binary with EventLog
	void buildMidBinary() {
		writeHead();
		writeDumyChunk();
		writeRealDataChunk();
	}
	///Test Print .mid File's Binary Code
	void printBinaryInform() {
		for (int i = 0; i < nowp; i++)
			printf("%02X ", byte[i]);
		FILE* out = fopen("TestOutput.txt", "wt");
		for (int i = 0; i < nowp; i++) {
			fprintf(out, "%02X ", byte[i]);
		}
		fclose(out);
	}
	//Make .mid File with Binary
	void makeMid(char* name) {
		char str[1000] = {};
		sprintf(str, "Fregic - %s.mid", name);
		FILE *out = fopen(str, "wb");
		for (int i = 0; i < nowp; i++) {
			fputc(byte[i], out);
		}
		fclose(out);
	}
}
using namespace NotationToMid;
int notation_parseMid(char* name) {
	init();
	load(name);
	deltaTimeToAbTime();
	buildEventLog();
	buildMidBinary();
	//printBinaryInform();
	makeMid(name);
	return 0;
}