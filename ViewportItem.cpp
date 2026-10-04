#include "app/ViewportItem.h"
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <algorithm>

ViewportItem::ViewportItem(QQuickItem* parent) : QQuickPaintedItem(parent) {
    setAcceptedMouseButtons(Qt::AllButtons);
    setAntialiasing(true);
    cam_.updateFromOrbit();
}

void ViewportItem::setController(CadController* c) {
    if (controller_ == c) return;
    controller_ = c;
    if (c) {
        connect(c, &CadController::modelChanged, this, &ViewportItem::onModelChanged);
        connect(c, &CadController::selectionChanged, this, [this] { update(); });
        onModelChanged();
    }
    emit controllerChanged();
}

void ViewportItem::onModelChanged() {
    std::vector<Triangle> tris;
    if (controller_) {
        const MeshData& m = controller_->mesh();
        for (size_t t = 0; t < m.triangleCount(); ++t) {
            auto v = [&](int k) { const auto& g = m.vertices[m.indices[3 * t + k]]; return Eigen::Vector3f(g.x, g.y, g.z); };
            tris.push_back({v(0), v(1), v(2), (int)t});
        }
        if (!m.empty() && !hadGeometry_) fitView();
        hadGeometry_ = !m.empty();
    }
    bvh_.build(tris);
    update();
}

void ViewportItem::fitView() {
    if (!controller_ || controller_->mesh().empty()) return;
    Eigen::Vector3f mn = Eigen::Vector3f::Constant(1e30f), mx = -mn;
    for (const auto& g : controller_->mesh().vertices) {
        Eigen::Vector3f p(g.x, g.y, g.z);
        mn = mn.cwiseMin(p); mx = mx.cwiseMax(p);
    }
    cam_.target = (mn + mx) * 0.5f;
    cam_.distance = std::max(1.f, (mx - mn).norm() * 1.8f);
    cam_.updateFromOrbit();
    update();
}

void ViewportItem::paint(QPainter* p) {
    p->setRenderHint(QPainter::Antialiasing);
    p->fillRect(boundingRect(), QColor(38, 42, 50));
    if (width() < 2 || height() < 2) return;
    cam_.aspect = float(width() / height());
    const Eigen::Matrix4f V = cam_.view(), VP = cam_.projection() * V;

    auto project = [&](const Eigen::Vector3f& w, QPointF& out) {
        Eigen::Vector4f c = VP * Eigen::Vector4f(w.x(), w.y(), w.z(), 1.f);
        if (c.w() <= 1e-6f) return false;
        out = QPointF((c.x() / c.w() * 0.5 + 0.5) * width(), (1.0 - (c.y() / c.w() * 0.5 + 0.5)) * height());
        return true;
    };

    // Ground axes
    float len = cam_.distance * 0.25f;
    const struct { Eigen::Vector3f d; QColor c; } axes[] = {{{1, 0, 0}, QColor(230, 80, 80)}, {{0, 1, 0}, QColor(90, 200, 90)}, {{0, 0, 1}, QColor(90, 140, 240)}};
    for (const auto& a : axes) {
        QPointF o, e;
        if (project(Eigen::Vector3f::Zero(), o) && project(a.d * len, e)) { p->setPen(QPen(a.c, 2)); p->drawLine(o, e); }
    }
    if (!controller_) return;

    struct Item { float depth; QPolygonF poly; QColor color; };
    std::vector<Item> items;
    const MeshData& m = controller_->mesh();
    const Eigen::Vector3f light = (cam_.position - cam_.target).normalized();
    const int selected = controller_->selectedFace();
    for (size_t t = 0; t < m.triangleCount(); ++t) {
        QPolygonF poly;
        Eigen::Vector3f centroid = Eigen::Vector3f::Zero();
        bool ok = true;
        for (int k = 0; k < 3; ++k) {
            const auto& g = m.vertices[m.indices[3 * t + k]];
            Eigen::Vector3f w(g.x, g.y, g.z);
            QPointF s;
            if (!project(w, s)) { ok = false; break; }
            poly << s; centroid += w / 3.f;
        }
        if (!ok) continue;
        const auto& g0 = m.vertices[m.indices[3 * t]];
        float shade = std::clamp(Eigen::Vector3f(g0.nx, g0.ny, g0.nz).dot(light), 0.f, 1.f) * 0.75f + 0.25f;
        QColor base = (m.triangleFace[t] == selected) ? QColor(255, 160, 40) : QColor(150, 175, 215);
        QColor col(int(base.red() * shade), int(base.green() * shade), int(base.blue() * shade));
        items.push_back({(V * Eigen::Vector4f(centroid.x(), centroid.y(), centroid.z(), 1.f)).z(), poly, col});
    }
    std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) { return a.depth < b.depth; });   // far -> near
    p->setPen(QPen(QColor(20, 20, 25, 90), 0.6));
    for (const auto& it : items) { p->setBrush(it.color); p->drawPolygon(it.poly); }
}

void ViewportItem::mousePressEvent(QMouseEvent* e) { last_ = pressPos_ = e->position(); dragged_ = false; }
void ViewportItem::mouseMoveEvent(QMouseEvent* e) {
    QPointF d = e->position() - last_;
    last_ = e->position();
    if ((e->position() - pressPos_).manhattanLength() > 4) dragged_ = true;
    if (e->buttons() & Qt::LeftButton) cam_.orbit(float(d.x()), float(d.y()));
    else if (e->buttons() & (Qt::RightButton | Qt::MiddleButton)) cam_.pan(float(d.x()), float(d.y()));
    update();
}
void ViewportItem::mouseReleaseEvent(QMouseEvent* e) { if (!dragged_ && e->button() == Qt::LeftButton) pick(e->position()); }
void ViewportItem::wheelEvent(QWheelEvent* e) { cam_.zoom(e->angleDelta().y() > 0 ? 0.88f : 1.14f); update(); }

void ViewportItem::pick(const QPointF& pt) {
    if (!controller_ || width() < 1 || height() < 1) return;
    cam_.aspect = float(width() / height());
    float x = float(2.0 * pt.x() / width() - 1.0), y = float(1.0 - 2.0 * pt.y() / height());
    Eigen::Matrix4f inv = (cam_.projection() * cam_.view()).inverse();
    Eigen::Vector4f n = inv * Eigen::Vector4f(x, y, -1, 1), f = inv * Eigen::Vector4f(x, y, 1, 1);
    Eigen::Vector3f a = n.head<3>() / n.w(), b = f.head<3>() / f.w();
    auto hits = bvh_.raycast({a, (b - a).normalized()});
    controller_->setSelectedFace(hits.empty() ? -1 : controller_->mesh().triangleFace[hits.front()]);
}
