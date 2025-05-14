#include "Animation.h"

// later removed
#include <iostream>

void Animation::Append(SubTexture s1, float a_time) {
	frames.push_back(s1);
	frametimes.push_back(a_time);
}



void Animation::Update(float fs) {
	time += fs;

	unsigned int last_index = frames.size() - 1;

	if (time > frametimes[last_index]) {
		time = time - frametimes[last_index];

		if (time > frametimes[last_index])
			time = 0;
	}
}


SubTexture Animation::getframe() {

	unsigned int index = 0;

	for (index = 0; index < frames.size(); index++) {
		if (time <= frametimes[index])
			break;
	}

	return frames[index];
}
