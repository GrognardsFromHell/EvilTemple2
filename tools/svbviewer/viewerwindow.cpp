
#include <QFileDialog>
#include <QFile>
#include <QImage>
#include <QErrorMessage>
#include <QLabel>
#include <QPixmap>
#include <QPainter>

#include "viewerwindow.h"
#include "ui_viewerwindow.h"

ViewerWindow::ViewerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ViewerWindow)
{
    ui->setupUi(this);

    ui->graphicsView->setScene(&scene);
}

ViewerWindow::~ViewerWindow()
{
    delete ui;
}

void ViewerWindow::on_actionExit_triggered()
{
    close();
}

// Extend the nibble of each byte into a separate entry of the resulting vector.
static QVector<uchar> transformNibbles(const QByteArray &data)
{
    QVector<uchar> result;

    for (int i = 0; i < data.size(); i++) {
        uchar byte = data[i];
        result.append(byte & 0xF);
        result.append((byte >> 4) & 0xF);
    }

    return result;
}

const uint PixelPerSubtile = 5;
const uint SectorSidelength = 64;

void ViewerWindow::on_actionOpen_triggered()
{
    QString svbFilename = QFileDialog::getOpenFileName(this, "Choose SVB file", QString(), "*.svb");
    if (svbFilename.isNull())
        return;

    QFile file(svbFilename);

    if (!file.open(QIODevice::ReadOnly)) {
        QErrorMessage dialog(this);
        dialog.showMessage("Couldn't open the SVB file.");
        return;
    }

    QByteArray svbData = file.readAll();
    QVector<uchar> subtiles = transformNibbles(svbData); // This data is tile-packed

    QImage image(192 * PixelPerSubtile, 192 * PixelPerSubtile, QImage::Format_ARGB32);
    image.fill(0);
    QPainter painter(&image);
    painter.drawRect(0, 0, 192 * PixelPerSubtile - 1, 192 * PixelPerSubtile - 1);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 0, 0));

    for (int ty = 0; ty < 192; ++ty) {
        for (int tx = 0; tx < 192; ++tx) {
            // Adressing in bit
            int offset = ty * 192 + tx;
            if ((subtiles[offset] & 1) == 1) {
                int px = tx;
                int py = ty;

                painter.drawRect(px * PixelPerSubtile, py * PixelPerSubtile,
                                 PixelPerSubtile, PixelPerSubtile);
            }
        }
    }

    // Fill a subrect
    painter.setPen(QColor(128, 128, 128));
    for (int x = 0; x < 64; ++x) {
        painter.drawLine(x * 3 * PixelPerSubtile, 1, x * 3 * PixelPerSubtile, 192 * PixelPerSubtile - 1);
        painter.drawLine(1, x * 3 * PixelPerSubtile, 192 * PixelPerSubtile - 1, x * 3 * PixelPerSubtile);
    }

    painter.end();

    scene.clear();

    scene.addPixmap(QPixmap::fromImage(image));


}
