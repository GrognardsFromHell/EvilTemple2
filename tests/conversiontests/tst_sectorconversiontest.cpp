#include <QtCore/QString>
#include <QtCore/QScopedPointer>
#include <QtTest/QtTest>

#include <conversion/basepathfinder.h>
#include <conversion/sectorconverter.h>

#include <zonetemplates.h>
#include <prototypes.h>
#include <virtualfilesystem.h>

class SectorConversionTest : public QObject
{
    Q_OBJECT

public:
    SectorConversionTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCase1();

private:
    QDir mBasePath;
    Troika::VirtualFileSystem *mVfs;
    Troika::Prototypes *mPrototypes;
    Troika::ZoneTemplates *mZoneTemplates;
};

SectorConversionTest::SectorConversionTest()
    : mVfs(0), mPrototypes(0), mZoneTemplates(0)
{
}

void SectorConversionTest::initTestCase()
{
    mBasePath = EvilTemple::BasepathFinder::find();

    if (!mBasePath.exists()) {
        qFatal("You don't have Temple of Elemental Evil installed.");
    }

    mVfs = new Troika::VirtualFileSystem(this);
    mVfs->loadDefaultArchives(mBasePath.absolutePath() + '/');

    mPrototypes = new Troika::Prototypes(mVfs, this);

    mZoneTemplates = new Troika::ZoneTemplates(mVfs, mPrototypes);
}

void SectorConversionTest::cleanupTestCase()
{
    delete mZoneTemplates;
}

void SectorConversionTest::testCase1()
{
    QScopedPointer<Troika::ZoneTemplate> zone(mZoneTemplates->load(5066)); // Load a standard map
    // QVERIFY2(zone && zone->name() == "Hommlet", "Hommlet map couldn't be loaded.");

    SectorConverter sectorConverter(zone.data());
    sectorConverter.convert();
    qDebug("Created tile information.");
    QImage img = sectorConverter.createWalkableImage();
    img.save("test-walkable.png");

    img = sectorConverter.createGroundMaterialImage();
    img.save("test-material.png");

    img = sectorConverter.createHeightImage();
    img.save("test-height.png");

    img = sectorConverter.createVisionExtendImage();
    img.save("test-visionextend.png");

    img = sectorConverter.createVisionEndImage();
    img.save("test-visionend.png");

    img = sectorConverter.createVisionArchwayImage();
    img.save("test-visionarchway.png");

    img = sectorConverter.createVisionBaseImage();
    img.save("test-visionbase.png");

    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(SectorConversionTest);

#include "tst_sectorconversiontest.moc"
