#pragma once
#include <TugBoat/Core.h>

namespace TugBoat {
struct DeltaTime {
	double m_DeltaSecs;

	DeltaTime(){
		m_DeltaSecs = 0.0;
	}

	DeltaTime(ulong oldTicks, ulong newTicks) {
		m_DeltaSecs = 0.0;
		Update(oldTicks, newTicks);
	}

	void Update(ulong oldTicks, ulong newTicks) {
		m_DeltaSecs = (newTicks - oldTicks) / 1000000.0;
	}
};
}