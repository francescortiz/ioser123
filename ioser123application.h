#include <QApplication>
#include <QFileOpenEvent>
#include <QtDebug>
#include <QEvent>

class IOSer123Application : public QApplication
{
public:

    IOSer123 *w;

    IOSer123Application(int &argc, char **argv)
        : QApplication(argc, argv)
    {
    }

    bool event(QEvent *event)
    {
        if (event->type() == QEvent::FileOpen) {
            QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
            const QString file = openEvent->file();
            qDebug() << "Open file" << file;

            w->setFile(file);
        }

        return QApplication::event(event);
    }

signals:
    void openFile(const QString file);
};
