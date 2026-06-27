#pragma once

#include <QList>

#include "../models/AppItem.h"
#include "../models/Placement.h"

class LayoutLaunchService {
public:
	void launchPlacements(
		const QList<AppItem>& apps,
		const QList<Placement>& placements) const;
};
