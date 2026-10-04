#pragma once
#include <QPointF>
#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>
#include "app/CadController.h"
#include "render/Camera.h"
#include "render/SpatialIndex.h"

// Software (QPainter) 3D viewport: orbit = left drag, pan = right/middle drag, zoom = wheel,
// click = pick a face (BVH ray cast).
class ViewportItem : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(CadController* controller READ controller WRITE setController NOTIFY controllerChanged)
public:
    explicit ViewportItem(QQuickItem* parent = nullptr);
    CadController* controller() const { return controller_; }
    void setController(CadController* c);
    void paint(QPainter* painter) override;
    Q_INVOKABLE void fitView();

signals:
    void controllerChanged();

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

private:
    void onModelChanged();
    void pick(const QPointF& p);

    CadController* controller_ = nullptr;
    Camera cam_;
    BVH bvh_;
    QPointF last_, pressPos_;
    bool dragged_ = false, hadGeometry_ = false;
};
