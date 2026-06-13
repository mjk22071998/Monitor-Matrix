#pragma once
#include<qlist.h>
#include "../models/AppItem.h"

class ConfigManager {
public:
	static QList<AppItem> getApplications(const QString& fileName);
};