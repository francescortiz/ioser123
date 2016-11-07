#include "ioser123.h"
#include "ui_ioser123.h"

#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>
#include <QPainter>
#include <QSvgRenderer>
#include <QTimer>
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>
#include <cmath>


IOSer123::IOSer123(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::IOSer123)
{
    ui->setupUi(this);

    ui->logo->setPixmap(QPixmap("://img/iOSer123.png"));

    setAcceptDrops(true);

    setUnifiedTitleAndToolBarOnMac(true);

    m_sSettingsFile = new QString(QStandardPaths::QStandardPaths::AppConfigLocation + QString(":/ioser123.ini"));
    loadSettings();

    ui->generateButton->setDisabled(true);

    //QObject::connect(QApplication::instance(), SIGNAL(openFile(QString)), this, SLOT(openFile(QString)));
}

IOSer123::~IOSer123()
{
    if (m_sSettingsFile != NULL) delete m_sSettingsFile;
    if (sourcePath != NULL) delete sourcePath;
    if (outputPath != NULL) delete outputPath;
    delete ui;
}

void IOSer123::dragEnterEvent(QDragEnterEvent *event)
{
    // if some actions should not be usable, like move, this code must be adopted
    event->acceptProposedAction();
}


void IOSer123::dragMoveEvent(QDragMoveEvent *event)
{
    // if some actions should not be usable, like move, this code must be adopted
    event->acceptProposedAction();
}

void IOSer123::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void IOSer123::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();

    if (mimeData->hasUrls())
    {
        QStringList pathList;
        QList<QUrl> urlList(mimeData->urls());
        if (urlList.length() == 1) {
            QString path(urlList.at(0).toLocalFile());

            if (setFile(path)) {
                event->acceptProposedAction();
            }
        }
    }
}


void IOSer123::loadSettings()
{
    QSettings settings(*m_sSettingsFile, QSettings::NativeFormat);
    outputPath = new QString(settings.value("outputPath", "").toString());
}

void IOSer123::saveSettings()
{
    QSettings settings(*m_sSettingsFile, QSettings::NativeFormat);
    settings.setValue("outputPath", *outputPath);
}

QSize IOSer123::getWidthHeight(int scale)
{
    int w, h;
    float ratio = (float) sourceWidth / (float) sourceHeight;
    int targetSize = ui->targetSizeEdit->value();

    if (ui->widthRadioButton->isChecked()) {
        w = targetSize * scale;
        h = (int) round((float) w / ratio);
        if ((int) ((float) h * ratio) < w) {
            h += 1;
        }
    } else {
        h = targetSize * scale;
        w = (int) round((float) h * ratio);
        if ((int) ((float) w / ratio) < h) {
            w += 1;
        }
    }
    return QSize(w, h);
}

bool IOSer123::setFile(QString path) {
    sourcePath = new QString(path);
    QFileInfo fileInfo(*sourcePath);

    QImage imageThumb = QImage(*sourcePath);

    bool success = false;
    isImage = false;

    if (fileInfo.completeSuffix() != "svg" &&
            fileInfo.completeSuffix() != "svgz" &&
            imageThumb.width() > 0) {
        success = true;
        isImage = true;
    } else {
        QSvgRenderer svg(path);
        const QSize &svgSize = svg.defaultSize();
        imageThumb = QImage(svgSize, QImage::Format_ARGB32);
        imageThumb.fill(0x00ffffff);
        QPainter painter(&imageThumb);
        svg.render(&painter);
        if (imageThumb.width() > 0) {
            success = true;
        }
    }

    if (success) {

        sourceWidth = imageThumb.width();
        sourceHeight = imageThumb.height();

        ui->sizeLabel->setText(QString::number(sourceWidth) + "x" + QString::number(sourceHeight));
        ui->ratioLabel->setText(QString::number((float) sourceWidth / (float) sourceHeight));

        if (sourceWidth > 500 ||sourceHeight  > 500) {
            imageThumb = imageThumb.scaled(500, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        ui->filenameLabel->setText(fileInfo.fileName());
        ui->nameEdit->setText(fileInfo.baseName());
        ui->imagePreview->setPixmap(QPixmap::fromImage(imageThumb));

        QTimer::singleShot(0, this, SLOT(adjustSizeDelayed()));

    }

    ui->generateButton->setDisabled(!success);
    return success;
}

void IOSer123::adjustSizeDelayed()
{
    adjustSize();
}

void IOSer123::on_generateButton_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    if (outputPath) {
        dialog.setDirectory(QDir(*outputPath));
    }
    QString savePath = dialog.getExistingDirectory();

    if (savePath.length() > 0) {
        outputPath = new QString(savePath);

        saveSettings();

        if (isImage) {
            renderFromImage();
        } else {
            renderFromSvg();
        }
    }
}

void IOSer123::openFile(QString file)
{
    setFile(file);
}

void IOSer123::renderFromImage() {
    QFileInfo fileInfo(*sourcePath);

    QImage sourceImage = QImage(*sourcePath);

    if (sourceImage.width() > 0) {

        QString targetFilename(ui->nameEdit->text());

        for (int i = 1; i < 4; i++) {
            QSize size = getWidthHeight(i);
            sourceImage
                    .scaled(size.width(), size.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation)
                    .save(getImagesPath(targetFilename) + "/" + targetFilename + "@x" + QString::number(i) + ".png")
                    ;
        }

        saveContentsJSON(targetFilename, "png");

    }
}

void IOSer123::renderFromSvg() {
    QFileInfo fileInfo(*sourcePath);

    QSvgRenderer svg(*sourcePath);
    const QSize svgSize(svg.defaultSize());

    if (svgSize.width() > 0) {
        QString targetFilename(ui->nameEdit->text());

        for (int i = 1; i < 4; i++) {
            QSize size(getWidthHeight(i));
            QImage imageThumb = QImage(size.width(), size.height(), QImage::Format_ARGB32);
            imageThumb.fill(0x00ffffff);
            QPainter painter(&imageThumb);
            svg.render(&painter);
            imageThumb.save(getImagesPath(targetFilename) + "/" + targetFilename + "@x" + QString::number(i) + ".png");
        }
        saveContentsJSON(targetFilename, "png");

    }
}

QString IOSer123::getImagesPath(QString targetFilename) {
    QString imagesPath(*outputPath + QString("/") + targetFilename + QString(".imageset"));
    QDir imagesPathRef(imagesPath);
    if (!imagesPathRef.exists()) {
        QDir(*outputPath).mkdir(targetFilename + QString(".imageset"));
    }
    return imagesPath;
}

void IOSer123::saveContentsJSON(QString targetFilename, QString extension) {
    QFile f(getImagesPath(targetFilename) + "/Contents.json");
    if (f.open(QFile::WriteOnly)) {
        f.write(getContentsJSON(targetFilename, extension).toLocal8Bit());
    }
}

QString IOSer123::getContentsJSON(QString name, QString extension) {
    return QString("{\r\n  \"images\" : [\r\n    {\r\n      \"idiom\" : \"universal\",\r\n      \"filename\" : \"{{name}}@x1.{{extension}}\",\r\n      \"scale\" : \"1x\"\r\n    },\r\n    {\r\n      \"idiom\" : \"universal\",\r\n      \"filename\" : \"{{name}}@x2.{{extension}}\",\r\n      \"scale\" : \"2x\"\r\n    },\r\n    {\r\n      \"idiom\" : \"universal\",\r\n      \"filename\" : \"{{name}}@x3.{{extension}}\",\r\n      \"scale\" : \"3x\"\r\n    }\r\n  ],\r\n  \"info\" : {\r\n    \"version\" : 1,\r\n    \"author\" : \"xcode\"\r\n  }\r\n}")
            .replace("{{name}}", name)
            .replace("{{extension}}", extension)
            ;
}
