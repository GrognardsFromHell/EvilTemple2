#include <QtCore/QString>
#include <QtTest/QtTest>

#include "common/quadtree.h"

class CommonTest : public QObject
{
    Q_OBJECT

public:
    CommonTest();

private Q_SLOTS:
    void testCase1();
    void test3x3Grid();
    void testRegressionOddSidelength();
};

CommonTest::CommonTest()
{
}

void CommonTest::testCase1()
{
    BoolQuadtree quadTree(1024, false);

    QVERIFY2(!quadTree.get(0, 0), "Initial value should be false.");
    QVERIFY2(!quadTree.get(1023, 1023), "Initial value should be false.");

    quadTree.set(0, 0, true);

    QCOMPARE(quadTree.get(0, 0), true);
    QCOMPARE(quadTree.get(1023, 1023), false);
    QCOMPARE(quadTree.get(1, 0), false);

    quadTree.set(0, 0, false);
    quadTree.set(1, 1, true);

    QCOMPARE(quadTree.get(0, 0), false);
    QCOMPARE(quadTree.get(1023, 1023), false);
    QCOMPARE(quadTree.get(1, 0), false);
    QCOMPARE(quadTree.get(1, 1), true);

    quadTree.set(1, 1, false);
    QCOMPARE(quadTree.get(1, 1), false);

    quadTree.set(1023, 1023, true);
    QCOMPARE(quadTree.get(1, 1), false);
    QCOMPARE(quadTree.get(1023, 1023), true);
}

void CommonTest::testRegressionOddSidelength()
{
    BoolQuadtree quadTree(3, false);

    quadTree.set(0, 2, true);

    // These all succeed
    QCOMPARE(quadTree.get(0, 0), false);
    QCOMPARE(quadTree.get(2, 2), false);
    QCOMPARE(quadTree.get(2, 0), false);
    QCOMPARE(quadTree.get(0, 2), true);

    // The "boundary" between two quadrants cause problems
    QCOMPARE(quadTree.get(1, 2), false);
}

void CommonTest::test3x3Grid()
{
    BoolQuadtree quadTree(3, false);

    for (int x = 0; x < 3; ++x)  {
        for (int y = 0; y < 3; ++y) {
            quadTree.set(x, y, true);

            for (int sx = 0; sx < 3; ++sx)  {
                for (int sy = 0; sy < 3; ++sy) {
                    bool expected = (sx == x) && (sy == y);
                    qDebug("Testing %d,%d with %d,%d being set", sx, sy, x, y);
                    QCOMPARE(quadTree.get(sx, sy), expected);
                }
            }

            quadTree.set(x, y, false);
        }
    }

}

QTEST_APPLESS_MAIN(CommonTest);

#include "tst_commontest.moc"
