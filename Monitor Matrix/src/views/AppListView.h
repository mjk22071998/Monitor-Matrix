#pragma once
#include <qlistwidget.h>
#include <qlist.h>
#include <qset.h>
#include <qwidget.h>
#include "../models/AppItem.h"

class AppListView : public QWidget {
	Q_OBJECT
private:
	QListWidget* p_listWidget;
	QList<AppItem> m_apps;

	void startDrag();

public:
	explicit AppListView(QWidget* parent = nullptr);
	void setApps(const QList<AppItem>& items);
	void setPlacedApps(const QSet<QString> &placedItems);
};