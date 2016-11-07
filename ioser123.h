#ifndef IOSER123_H
#define IOSER123_H

#include <QMainWindow>

namespace Ui {
class IOSer123;
}

class IOSer123 : public QMainWindow
{
    Q_OBJECT

public:
    explicit IOSer123(QWidget *parent = 0);
    ~IOSer123();

    bool setFile(QString path);

protected:
    // —— events ———————————————————————————
   /*
    * this event is called when the mouse enters the widgets area during a drag/drop operation
    */
   void dragEnterEvent(QDragEnterEvent *event);

   /**
    * this event is called when the mouse moves inside the widgets area during a drag/drop operation
    */
   void dragMoveEvent(QDragMoveEvent *event);

   /**
    * this event is called when the mouse leaves the widgets area during a drag/drop operation
    */
   void dragLeaveEvent(QDragLeaveEvent *event);

   /**
    * this event is called when the drop operation is initiated at the widget
    */
   void dropEvent(QDropEvent *event);


private:
    Ui::IOSer123 *ui;

    bool openFiles(const QStringList& pathList);
    QString *sourcePath = NULL;
    int sourceWidth = 0;
    int sourceHeight = 0;
    QString *outputPath = NULL;
    bool isImage;
    const QString *m_sSettingsFile = NULL;

    void renderFromImage();
    void renderFromSvg();
    void createImages(QImage sourceImage, int targetWidth);
    void loadSettings();
    void saveSettings();
    QSize getWidthHeight(int scale);

    QString getImagesPath(QString targetFilename);

    void saveContentsJSON(QString targetFilename, QString extension);
    QString getContentsJSON(QString name, QString extension);
private slots:
    void adjustSizeDelayed();

    void on_generateButton_clicked();

    void openFile(QString file);
};

#endif // IOSER123_H
