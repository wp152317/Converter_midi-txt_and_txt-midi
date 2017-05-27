#include "MidiParser.h"
#include <stdio.h>

int main() {
	mid_parseNotation("Test");
	mid_parseNotation("Test2");
	notation_parseMid("Test2");
	notation_parseMid("Test");
}