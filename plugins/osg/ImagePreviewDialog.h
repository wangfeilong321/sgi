#pragma once

#include <QDialog>
#include <sgi/plugins/SGIPluginInterface.h>

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QToolBar;
class QScrollBar;
class Ui_ImagePreviewDialog;
QT_END_NAMESPACE

namespace osg {
    class Texture;
    class Image;
    class Camera;
}

namespace sgi {
namespace osg_plugin {

class ImagePreviewDialog : public QDialog
{
    Q_OBJECT

public:
	ImagePreviewDialog(QWidget * parent, osg::Image * image);
    ImagePreviewDialog(QWidget * parent, osg::Texture * texture);
    virtual ~ImagePreviewDialog();

public:
    ISettingsDialog *   dialogInterface() { return _interface; }

public slots:
    void setLabel(const QString & label);

private slots:
    void open();
    void save();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
	void load(const QString & filename);
	void load(const QImage * image);
    void load(const osg::Image * image);
    void load(const osg::Texture * texture);

    void textureReady(QImage image);

signals:
    void textureRendered(QImage image);

public:
    void emitTextureRendered(QImage image);

private:
    void createToolbar();
    void updateToolbar();
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    void renderTextureToQImage(osg::Texture * texture);

private:
    Ui_ImagePreviewDialog *         ui;
    osg::ref_ptr<osg::Image>        _image;
    osg::ref_ptr<osg::Texture>      _texture;
    osg::ref_ptr<osg::Camera>       _textureCamera;
    ISettingsDialogPtr              _interface;
    QToolBar *                      _toolBar;
    double                          _scaleFactor;
    QAction *                       _saveAction;
    QAction *                       _zoomInAction;
    QAction *                       _zoomOutAction;
    QAction *                       _normalSizeAction;
    QAction *                       _fitToWindowAction;
    QString                         _labelText;
};

} // namespace osg_plugin
} // namespace sgi
