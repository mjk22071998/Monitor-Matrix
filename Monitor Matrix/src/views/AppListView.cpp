#include "AppListView.h"
#include <qlayout.h>
#include <qdrag.h>
#include <qmimedata.h>
#include <qevent.h>
#include <qfileinfo.h>
#include <qicon.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qsize.h>
#include <qurl.h>
#include <qpalette.h>
#include <qabstractitemview.h>

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
	layout->setContentsMargins(14, 14, 14, 12);
	layout->setSpacing(10);

	auto* sectionHeader = new QLabel("Applications", this);
	sectionHeader->setObjectName("sectionHeader");
	layout->addWidget(sectionHeader);

	auto* actionsLayout = new QHBoxLayout();
	actionsLayout->setContentsMargins(0, 0, 0, 0);
	actionsLayout->setSpacing(8);
	p_addButton = new QPushButton("Add App", this);
	p_removeButton = new QPushButton("Remove", this);
	p_addButton->setObjectName("secondaryButton");
	p_removeButton->setObjectName("dangerButton");
	p_addButton->setIcon(QIcon(QStringLiteral(":/icons/action-add.svg")));
	p_removeButton->setIcon(QIcon(QStringLiteral(":/icons/action-remove.svg")));
	p_addButton->setIconSize(QSize(16, 16));
	p_removeButton->setIconSize(QSize(16, 16));
	p_removeButton->setEnabled(false);
	actionsLayout->addWidget(p_addButton);
	actionsLayout->addWidget(p_removeButton);

	p_listWidget = new QListWidget(this);
	p_listWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
	p_listWidget->setDragEnabled(false);
	p_listWidget->setAcceptDrops(true);
	p_listWidget->viewport()->setAcceptDrops(true);
	p_listWidget->setDropIndicatorShown(false);
	p_listWidget->setIconSize(QSize(32, 32));
	layout->addLayout(actionsLayout);
	layout->addWidget(p_listWidget);
	setLayout(layout);

	connect(p_listWidget, &QListWidget::itemPressed, this, &AppListView::startDrag);
	connect(p_listWidget, &QListWidget::currentItemChanged,
		this, [this]() { updateRemoveButton(); });
	connect(p_addButton, &QPushButton::clicked,
		this, &AppListView::addRequested);
	connect(p_removeButton, &QPushButton::clicked, this, [this]() {
		const QString appId = selectedAppId();
		if (!appId.isEmpty()) {
			emit removeRequested(appId);
		}
	});

	p_listWidget->installEventFilter(this);
	p_listWidget->viewport()->installEventFilter(this);
}

void AppListView::setApps(const QList<AppItem>& items)
{
	m_apps = items;
	p_listWidget->clear();
	for (const AppItem& item : items) {
		QListWidgetItem* listItem = new QListWidgetItem(item.icon, item.name + " (" + item.type + ")");
		listItem->setData(Qt::UserRole, item.id);
		listItem->setData(Qt::UserRole + 1, item.name);
		listItem->setData(Qt::UserRole + 2, item.path);
		listItem->setData(Qt::UserRole + 3, false);
		listItem->setFlags(Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled);
		p_listWidget->addItem(listItem);
	}

	updateRemoveButton();
}

void AppListView::setPlacedApps(const QSet<QString>& placedItems)
{
	for (int i = 0; i < p_listWidget->count(); i++) {
		auto* item = p_listWidget->item(i);
		QString id = item->data(Qt::ItemDataRole::UserRole).toString();
		bool isPlaced = placedItems.contains(id);
		item->setData(Qt::UserRole + 3, isPlaced);
		item->setFlags(Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled);
		if (isPlaced) {
			item->setForeground(QBrush());
			item->setToolTip(QStringLiteral("Placed in the current layout. Drag again to add another placement."));
		}
		else {
			item->setForeground(QBrush());
			item->setToolTip(QString());
		}
	}

	updateRemoveButton();
}

QString AppListView::selectedAppId() const
{
	QListWidgetItem* item = p_listWidget->currentItem();
	if (!item) {
		return QString();
	}

	return item->data(Qt::UserRole).toString();
}

void AppListView::updateRemoveButton()
{
	p_removeButton->setEnabled(!selectedAppId().isEmpty());
}

QStringList AppListView::applicationFilePathsFromMimeData(const QMimeData* mimeData) const
{
	QStringList filePaths;

	if (!mimeData || !mimeData->hasUrls()) {
		return filePaths;
	}

	for (const QUrl& url : mimeData->urls()) {
		if (!url.isLocalFile()) {
			continue;
		}

		QFileInfo fileInfo(url.toLocalFile());

		if (fileInfo.exists() &&
			fileInfo.isFile() &&
			(fileInfo.suffix().compare("exe", Qt::CaseInsensitive) == 0 ||
				fileInfo.suffix().compare("lnk", Qt::CaseInsensitive) == 0)) {
			filePaths.append(fileInfo.absoluteFilePath());
		}
	}

	return filePaths;
}

bool AppListView::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == p_listWidget || watched == p_listWidget->viewport()) {
		if (event->type() == QEvent::DragEnter || event->type() == QEvent::DragMove) {
			auto* dragEvent = static_cast<QDragMoveEvent*>(event);
			if (!applicationFilePathsFromMimeData(dragEvent->mimeData()).isEmpty()) {
				dragEvent->setDropAction(Qt::CopyAction);
				dragEvent->accept();
			}
			else {
				dragEvent->ignore();
			}

			return true;
		}

		if (event->type() == QEvent::Drop) {
			auto* dropEvent = static_cast<QDropEvent*>(event);
			const QStringList filePaths =
				applicationFilePathsFromMimeData(dropEvent->mimeData());

			if (!filePaths.isEmpty()) {
				dropEvent->setDropAction(Qt::CopyAction);
				dropEvent->accept();
				emit applicationFilesDropped(filePaths);
			}
			else {
				dropEvent->ignore();
			}

			return true;
		}
	}

	return QWidget::eventFilter(watched, event);
}
