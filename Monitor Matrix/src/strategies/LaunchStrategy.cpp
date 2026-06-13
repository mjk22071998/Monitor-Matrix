#include "LaunchStrategy.h"
#include <qdebug.h>

LaunchStrategy::LaunchStrategy()
{
	p_launchService = LaunchService::getInstance();
}

bool LaunchStrategy::ensureLaunchService()
{
	if (!p_launchService) {
		qWarning() << "LaunchService instance is null";
		return false;
	}
	return true;
}
