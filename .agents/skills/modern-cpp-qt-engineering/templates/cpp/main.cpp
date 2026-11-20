#include <QApplication>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>

int main(int argc, char* argv[])
{
    // High-DPI PassThrough policy for crisp rendering
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough
    );

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Antigravity"));
    QCoreApplication::setApplicationName(QStringLiteral("ModernQtApp"));

    QMainWindow mainWindow;
    mainWindow.setWindowTitle(QStringLiteral("Modern Qt Application"));
    mainWindow.resize(800, 600);

    auto* centralWidget = new QWidget(&mainWindow);
    auto* layout = new QVBoxLayout(centralWidget);
    auto* label = new QLabel(QStringLiteral("Modern C++23 & Qt 6 Application"), centralWidget);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    mainWindow.setCentralWidget(centralWidget);
    mainWindow.show();

    return app.exec();
}
