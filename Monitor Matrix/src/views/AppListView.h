#pragma once
#include <qlistwidget.h>
#include <qlist.h>
#include <qset.h>
#include <qstringlist.h>
#include <qwidget.h>
#include "../models/AppItem.h"

class QPushButton;
class QEvent;
class QMimeData;

class AppListView : public QWidget {
	Q_OBJECT
private:
	QListWidget* p_listWidget;
	QPushButton* p_addButton = nullptr;
	QPushButton* p_removeButton = nullptr;
	QList<AppItem> m_apps;

	void startDrag();
	void updateRemoveButton();
	QStringList applicationFilePathsFromMimeData(const QMimeData* mimeData) const;

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

public:
	explicit AppListView(QWidget* parent = nullptr);
	void setApps(const QList<AppItem>& items);
	void setPlacedApps(const QSet<QString> &placedItems);
	QString selectedAppId() const;

signals:
	void addRequested();
	void removeRequested(const QString& appId);
	void applicationFilesDropped(const QStringList& filePaths);
};
