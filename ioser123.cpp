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

QSize IOSer123::getWidthHeight(float scale)
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
            renderFromImage(getPlatform());
        } else {
            renderFromSvg(getPlatform());
        }
    }
}

Platform IOSer123::getPlatform() {
    if (ui->iOSRadioButton->isChecked()) {
        return ios;
    }
    return android;
}

void IOSer123::openFile(QString file)
{
    setFile(file);
}

void IOSer123::renderFromImage(Platform platform) {
    QFileInfo fileInfo(*sourcePath);

    QImage sourceImage = QImage(*sourcePath);

    if (sourceImage.width() > 0) {

        QString targetFilename(ui->nameEdit->text());

        QList<float> sizes;
        QList<QString> filenames;

        switch (platform) {
        case ios:
            for (int i = 1; i < 4; i++)  {
                sizes.append((float) i);
                filenames.append(getImagesPathIOS(targetFilename) + "/" + targetFilename + "@x" + QString::number(i) + ".png");
            }
            break;
        default:
            sizes.append(120.0/160.0);
            sizes.append(160.0/160.0);
            sizes.append(240.0/160.0);
            sizes.append(320.0/160.0);
            sizes.append(480.0/160.0);
            sizes.append(640.0/160.0);
            filenames.append(*outputPath + "/ldpi/" + targetFilename + ".png");
            filenames.append(*outputPath + "/mdpi/" + targetFilename + ".png");
            filenames.append(*outputPath + "/hdpi/" + targetFilename + ".png");
            filenames.append(*outputPath + "/xhdpi/" + targetFilename + ".png");
            filenames.append(*outputPath + "/xxhdpi/" + targetFilename + ".png");
            filenames.append(*outputPath + "/xxxhdpi/" + targetFilename + ".png");
            QDir(*outputPath).mkdir("ldpi");
            QDir(*outputPath).mkdir("mdpi");
            QDir(*outputPath).mkdir("hdpi");
            QDir(*outputPath).mkdir("xhdpi");
            QDir(*outputPath).mkdir("xxhdpi");
            QDir(*outputPath).mkdir("xxxhdpi");
            break;
        }

        for (int i = 0; i < sizes.length(); i++) {
            QSize size = getWidthHeight(sizes.at(i));
            sourceImage
                    .scaled(size.width(), size.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation)
                    .save(filenames.at(i));
                    ;
        }

        if (platform == ios) {
            saveContentsJSON(targetFilename, "png");
        }
    }
}

void IOSer123::renderFromSvg(Platform platform) {
    QFileInfo fileInfo(*sourcePath);

    QSvgRenderer svg(*sourcePath);
    const QSize svgSize(svg.defaultSize());

    if (svgSize.width() > 0) {
        QString targetFilename(ui->nameEdit->text());

        QList<float> sizes;
        QList<QString> filenames;

        switch (platform) {
        case ios:
            for (int i = 1; i < 4; i++)  {
                sizes.append((float) i);
                filenames.append(getImagesPathIOS(targetFilename) + "/" + targetFilename + "@x" + QString::number(i) + ".png");
            }
            break;
        default:
            sizes.append(120.0/160.0);
            sizes.append(160.0/160.0);
            sizes.append(240.0/160.0);
            sizes.append(320.0/160.0);
            sizes.append(480.0/160.0);
            sizes.append(640.0/160.0);
            filenames.append(*outputPath + "/ldpi/" + targetFilename + ".png");
            filenames.append(*outputPath + "/mdpi/" + targetFilename + ".png");
            filenames.append(*outputPath + "/hdpi/" + targetFilename + ".png");
            filenames.append(*outputPath + "/xhdpi/" + targetFilename + ".png");
            filenames.append(*outputPath + "/xxhdpi/" + targetFilename + ".png");
            filenames.append(*outputPath + "/xxxhdpi/" + targetFilename + ".png");
            QDir(*outputPath).mkdir("ldpi");
            QDir(*outputPath).mkdir("mdpi");
            QDir(*outputPath).mkdir("hdpi");
            QDir(*outputPath).mkdir("xhdpi");
            QDir(*outputPath).mkdir("xxhdpi");
            QDir(*outputPath).mkdir("xxxhdpi");
            break;
        }

        for (int i = 0; i < sizes.length(); i++) {
            QSize size = getWidthHeight(sizes.at(i));
            QImage imageThumb = QImage(size.width(), size.height(), QImage::Format_ARGB32);
            imageThumb.fill(0x00ffffff);
            QPainter painter(&imageThumb);
            svg.render(&painter);
            imageThumb.save(filenames.at(i));
        }

        if (platform == ios) {
            saveContentsJSON(targetFilename, "png");
        }

    }
}

QString IOSer123::getImagesPathIOS(QString targetFilename) {
    QString imagesPath(*outputPath + QString("/") + targetFilename + QString(".imageset"));
    QDir imagesPathRef(imagesPath);
    if (!imagesPathRef.exists()) {
        QDir(*outputPath).mkdir(targetFilename + QString(".imageset"));
    }
    return imagesPath;
}

void IOSer123::saveContentsJSON(QString targetFilename, QString extension) {
    QFile f(getImagesPathIOS(targetFilename) + "/Contents.json");
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
