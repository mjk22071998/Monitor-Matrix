#include <QTest>
#include <QQmlApplicationEngine>

class SanityTest : public QObject {
    Q_OBJECT
private slots:
    void testQmlEngineInstantiates() {
        QQmlApplicationEngine engine;
        QVERIFY(engine.rootObjects().isEmpty());
    }
};

QTEST_MAIN(SanityTest)
#include "SanityTest.moc"
