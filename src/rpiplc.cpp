#include "rpiplc.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
	initPins();

	setup();

	while (1) {
		loop();
	}

	return 0;
}
