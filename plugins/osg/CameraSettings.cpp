#include "stdafx.h"
#include <osg/Camera>
#include "CameraSettings.h"

#include <QPushButton>
#include <QTimer>

#include <ui_CameraSettings.h>

#include <sgi/plugins/SGISettingsDialogImpl>
#include <osgViewer/View>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace sgi {
namespace osg_plugin {

CameraSettings::CameraSettings(QWidget * parent, osg::Camera * camera)
	: QDialog(parent)
	, _camera(camera)
    , _interface(new SettingsDialogImpl(this))
    , _timer(new QTimer(this))
{
	ui = new Ui_CameraSettings;
	ui->setupUi( this );

	//connect(ui->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(save()));
	connect(ui->buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()), this, SLOT(restoreDefaults()));

    connect(_timer, SIGNAL(timeout()), this, SLOT(load()));

	load();
}

CameraSettings::~CameraSettings()
{
	if (ui)
	{
		delete ui;
		ui = NULL;
	}
}

void CameraSettings::showEvent( QShowEvent * event )
{
    QDialog::showEvent(event);
    load();
    _timer->start(1000);
}

void CameraSettings::hideEvent  ( QHideEvent * event )
{
    QDialog::hideEvent(event);
    _timer->stop();
}

void CameraSettings::load()
{
	float scale = _camera->getLODScale();
    float smallFeature = _camera->getSmallFeatureCullingPixelSize();
    double nearFarRatio = _camera->getNearFarRatio();

    double fovy, aspectRatio, zNear, zFar;
    _camera->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    ui->lodScaleSpin->blockSignals(true);
    ui->lodScale->blockSignals(true);
	ui->lodScaleSpin->setValue(scale);
    ui->lodScale->setValue(scale * 10);
    ui->lodScaleSpin->blockSignals(false);
    ui->lodScale->blockSignals(false);

    ui->smallFeatureSpin->blockSignals(true);
    ui->smallFeature->blockSignals(true);
    ui->smallFeatureSpin->setValue(smallFeature);
    ui->smallFeature->setValue(smallFeature * 10);
    ui->smallFeatureSpin->blockSignals(false);
    ui->smallFeature->blockSignals(false);

    ui->nearClippingSpin->blockSignals(true);
    ui->nearClipping->blockSignals(true);
    ui->nearClippingSpin->setValue(zNear);
    ui->nearClipping->setValue(zNear * 100000);
    ui->nearClippingSpin->blockSignals(false);
    ui->nearClipping->blockSignals(false);

    ui->farClippingSpin->blockSignals(true);
    ui->farClipping->blockSignals(true);
    ui->farClippingSpin->setValue(zFar);
    ui->farClipping->setValue(zFar * 10);
    ui->farClippingSpin->blockSignals(false);
    ui->farClipping->blockSignals(false);

    ui->nearFarRatioSpin->blockSignals(true);
    ui->nearFarRatio->blockSignals(true);
    ui->nearFarRatioSpin->setValue(nearFarRatio);
    ui->nearFarRatio->setValue(nearFarRatio * 10000);
    ui->nearFarRatioSpin->blockSignals(false);
    ui->nearFarRatio->blockSignals(false);

    ui->computeNearFarMode->blockSignals(true);
    ui->computeNearFarMode->setCurrentIndex((int)_camera->getComputeNearFarMode());
    ui->computeNearFarMode->blockSignals(false);

}

void CameraSettings::restoreDefaults()
{
    _camera->setLODScale(1.0f);
    _camera->setSmallFeatureCullingPixelSize(2.0f);

    double fovy, aspectRatio, zNear, zFar;
    _camera->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    zNear = 0.001;
    zFar = 10000.0;

    _camera->setProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

	load();
}

void CameraSettings::changeLODScale(int value)
{
    ui->lodScaleSpin->blockSignals(true);
	ui->lodScaleSpin->setValue((double)value/10.0);
    ui->lodScaleSpin->blockSignals(false);
	apply();
}

void CameraSettings::changeLODScale(double value)
{
    ui->lodScale->blockSignals(true);
	ui->lodScale->setValue(value * 10);
    ui->lodScale->blockSignals(false);
	apply();
}

void CameraSettings::changeSmallFeature(int value)
{
    ui->smallFeatureSpin->blockSignals(true);
    ui->smallFeatureSpin->setValue((double)value/10.0);
    ui->smallFeatureSpin->blockSignals(false);
    apply();
}

void CameraSettings::changeSmallFeature(double value)
{
    ui->smallFeature->blockSignals(true);
    ui->smallFeature->setValue(value * 10);
    ui->smallFeature->blockSignals(false);
    apply();
}

void CameraSettings::changeNearClipping(int value)
{
    ui->nearClippingSpin->blockSignals(true);
    ui->nearClippingSpin->setValue((double)value/100000.0);
    ui->nearClippingSpin->blockSignals(false);
    apply();
}

void CameraSettings::changeNearClipping(double value)
{
    ui->nearClipping->blockSignals(true);
    ui->nearClipping->setValue(value * 100000);
    ui->nearClipping->blockSignals(false);
    apply();
}

void CameraSettings::changeFarClipping(int value)
{
    ui->farClippingSpin->blockSignals(true);
    ui->farClippingSpin->setValue((double)value/10.0);
    ui->farClippingSpin->blockSignals(false);
    apply();
}

void CameraSettings::changeFarClipping(double value)
{
    ui->farClipping->blockSignals(true);
    ui->farClipping->setValue(value * 10);
    ui->farClipping->blockSignals(false);
    apply();
}

void CameraSettings::changeNearFarRatio(int value)
{
    ui->nearFarRatioSpin->blockSignals(true);
    ui->nearFarRatioSpin->setValue((double)value/10000.0);
    ui->nearFarRatioSpin->blockSignals(false);
    apply();
}

void CameraSettings::changeNearFarRatio(double value)
{
    ui->nearFarRatio->blockSignals(true);
    ui->nearFarRatio->setValue(value * 10000);
    ui->nearFarRatio->blockSignals(false);
    apply();
}

void CameraSettings::changeComputeNearFarMode(int value)
{
    apply();
}

void CameraSettings::save()
{
	apply(true);
	accept();
}

void CameraSettings::apply()
{
	apply(false);
}

void CameraSettings::apply(bool save)
{
	double scale = ui->lodScaleSpin->value();
    _camera->setLODScale(scale);

    double smallFeature = ui->smallFeatureSpin->value();
    _camera->setSmallFeatureCullingPixelSize(smallFeature);

    double fovy, aspectRatio, zNear, zFar;
    _camera->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    zNear = ui->nearClippingSpin->value();
    zFar = ui->farClippingSpin->value();

    _camera->setProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

    _camera->setComputeNearFarMode((osg::Camera::ComputeNearFarMode)ui->computeNearFarMode->currentIndex());

    _camera->setNearFarRatio(ui->nearFarRatioSpin->value());

    osgViewer::View* view = dynamic_cast<osgViewer::View*>(_camera->getView());
    if(view)
        view->requestRedraw();
}

void CameraSettings::reject()
{
	QDialog::reject();
}

} // namespace osg_plugin
} // namespace sgi
