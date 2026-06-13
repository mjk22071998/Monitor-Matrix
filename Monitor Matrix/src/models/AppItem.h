#pragma once
#include <qstring.h>
#include <qicon.h>
#include <memory>

struct AppItem {
	QString id;
	QString name;
	QString type;
	QString path;
	std::shared_ptr<class LaunchStrategy> launchStrategy;
	QIcon icon;
};

