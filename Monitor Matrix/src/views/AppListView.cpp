#include "AppListView.h"
#include <qlayout.h>
#include <qdrag.h>
#include <qmimedata.h>

void AppListView::startDrag()
{
	QListWidgetItem* listItem = p_listWidget->currentItem();
	if (!listItem) return;

	QString appId = listItem->data(Qt::UserRole).toString();
	QString appName = listItem->data(Qt::UserRole + 1).toString();
	QMimeData* data = new QMimeData();
	data->setData("application/x-screendrop-appId",appId.toUtf8());
	data->setData("application/x-screendrop-appName", appName.toUtf8());

	QDrag* drag = new QDrag(this);
	drag->setMimeData(data);
	drag->setPixmap(listItem->icon().pixmap(32, 32));
	drag->exec(Qt::DropAction::CopyAction);
}

AppListView::AppListView(QWidget* parent): QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	p_listWidget = new QListWidget(this);
	p_listWidget->setDragEnabled(true);
	p_listWidget->setIconSize(QSize(32, 32));
	layout->addWidget(p_listWidget);
	setLayout(layout);

	connect(p_listWidget, &QListWidget::itemPressed, this, &AppListView::startDrag);
}

void AppListView::setApps(const QList<AppItem>& items)
{
	m_apps = items;
	p_listWidget->clear();
	for (const AppItem& item : items) {
		QListWidgetItem* listItem = new QListWidgetItem(item.icon, item.name + " (" + item.type + ")");
		listItem->setData(Qt::UserRole, item.id);
		listItem->setData(Qt::UserRole + 1, item.name);
		listItem->setFlags(listItem->flags() & ~(Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::NoItemFlags));
		p_listWidget->addItem(listItem);
	}
}

void AppListView::setPlacedApps(const QSet<QString>& placedItems)
{
	for (int i = 0; i < p_listWidget->count(); i++) {
		auto* item = p_listWidget->item(i);
		QString id = item->data(Qt::ItemDataRole::UserRole).toString();
		bool isPlaced = placedItems.contains(id);
		if (isPlaced) {
			item->setFlags(item->flags() & ~(Qt::ItemFlag::ItemIsEnabled));
			item->setForeground(Qt::GlobalColor::gray);
		}
		else {
			item->setFlags(item->flags() | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled);
			item->setForeground(Qt::GlobalColor::black);
		}
	}
}
