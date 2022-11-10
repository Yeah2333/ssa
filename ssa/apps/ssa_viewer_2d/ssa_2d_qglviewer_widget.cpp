#include "ssa_2d_qglviewer_widget.h"
#include <QFileDialog>
#include "standardCamera.h"

/**
 * \brief handle the GLU quadratic
 */
class GLUWrapper {
public:
    static GLUquadricObj *getQuadradic() {
        static GLUWrapper inst;
        return inst._quadratic;
    }

protected:
    GLUWrapper() {
        //std::cerr << __PRETTY_FUNCTION__ << std::endl;
        _quadratic = gluNewQuadric();              // Create A Pointer To The Quadric Object ( NEW )
        gluQuadricNormals(_quadratic, GLU_SMOOTH); // Create Smooth Normals ( NEW )
    }

    ~GLUWrapper() {
        //std::cerr << __PRETTY_FUNCTION__ << std::endl;
        gluDeleteQuadric(_quadratic);
    }

    GLUquadricObj *_quadratic;;
};

namespace ssa {

    SSA2DglWidget::SSA2DglWidget() {
        setInitialParams();
        _ssa_graph = 0;
        _poseVertices = 0;
        _obsVertices = 0;
        _normals = 0;
        _mesh = 0;
    }

    SSA2DglWidget::SSA2DglWidget(QWidget *&widget) {
        (void) widget; ///remove warning
        setInitialParams();
        _ssa_graph = 0;
        _poseVertices = 0;
        _obsVertices = 0;
        _normals = 0;
        _mesh = 0;

        // replace camera
        qglviewer::Camera *oldcam = camera();
        qglviewer::Camera *cam = new Vis::StandardCamera();
        cam->setPosition(qglviewer::Vec(0., 0., 10.));
        cam->setUpVector(qglviewer::Vec(0., 1., 0.));
        cam->lookAt(qglviewer::Vec(0., 0., 0.));
        setCamera(cam);
        delete oldcam;


        init();
    }

    SSA2DglWidget::~SSA2DglWidget() {
        if (_poseVertices > 0)
            glDeleteLists(_poseVertices, 1);
        if (_obsVertices > 0)
            glDeleteLists(_obsVertices, 1);
    }

    void SSA2DglWidget::init() {
        QGLViewer::init();
        // gl setup
        //  glDisable(GL_LIGHT0);
        //  glDisable(GL_LIGHTING);

        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glShadeModel(GL_SMOOTH);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // mouse bindings
        setMouseBinding(Qt::RightButton, CAMERA, ZOOM);
        setMouseBinding(Qt::MidButton, CAMERA, TRANSLATE);
        setMouseBinding(Qt::LeftButton, CAMERA, TRANSLATE);

//   // keyboard shortcuts
//   setShortcut(CAMERA_MODE, 0);
//   setShortcut(EXIT_VIEWER, 0);
//   setShortcut(SAVE_SCREENSHOT, 0);

        // description for shortcuts
//   setKeyDescription(Qt::CTRL + Qt::Key_S, "capture a screenshot (stored in current directory)");
//   setKeyDescription(Qt::CTRL + Qt::Key_M, "Toggle visibility of the main menu");
//   setKeyDescription(Qt::CTRL + Qt::Key_W, "Toggles wire frame display");

//   emit initOpenGl();
//
//   _initCalled = true;
    }

    void SSA2DglWidget::draw() {
        if (!_ssa_graph)
            return;

        GLboolean hasLight = glIsEnabled(GL_LIGHTING);
        if (hasLight)
            glDisable(GL_LIGHTING);

        //glDisable(GL_DEPTH_TEST);
        // Draw Vertex3 Nodes
        if (_drawVertices)
            glCallList(_poseVertices);

        // Draw VertexXYZCOV Nodes
        if (_drawLandmarkVertices)
            glCallList(_obsVertices);
        //drawVertices();

        if (_drawNormals)
            glCallList(_normals);

        drawEdges();

        if (_drawMesh)
            glCallList(_mesh);

        if (hasLight)
            glEnable(GL_LIGHTING);
    }

    void SSA2DglWidget::drawWithNames() {

        glColor3f(0.0, 0.0, 1.0);
        // Draw Vertex2 Nodes
        for (g2o::OptimizableGraph::VertexIDMap::const_iterator it = _ssa_graph->_optimizer.vertices().begin();
             it != _ssa_graph->_optimizer.vertices().end(); it++) {
            g2o::VertexSE2 *v = dynamic_cast<g2o::VertexSE2 *>(it->second);
            if (v) {
                glPushName(v->id());
                glPushMatrix();
                glTranslatef(v->estimate().translation()(0), v->estimate().translation()(1), 0.0f);
                glScalef(0.25f, 0.25f, 0.25f);
                drawPoseBox();
                //drawPoseCircleWithName(v->estimate()[0],v->estimate()[1],  v->estimate()[2]);
                glPopMatrix();
                glPopName();
            }
        }

    }

    void SSA2DglWidget::drawEdges() {

        glColor3f(0.0, 0.0, 0.0);
        // Draw Edges
        if (_drawVertices)
            for (unsigned int i = 0; i < _ssa_graph->_edges_odometry.size(); i++) {
                g2o::EdgeSE2 *&odomEdge = _ssa_graph->_edges_odometry[i];

                g2o::VertexSE2 *from = dynamic_cast<g2o::VertexSE2 *>(odomEdge->vertices()[0]);
                g2o::VertexSE2 *to = dynamic_cast<g2o::VertexSE2 *>(odomEdge->vertices()[1]);
                Eigen::Vector2d fromPose = from->estimate().translation();
                Eigen::Vector2d toPose = to->estimate().translation();
                glBegin(GL_LINES);
                glVertex3f(fromPose(0), fromPose(1), 0.0f);
                glVertex3f(toPose(0), toPose(1), 0.0f);
                glEnd();
            }

        glColor3f(1.0, 0.0, 0.0);
        if (_drawCorrespondences && false) ///TODO: Make thread safe
            for (unsigned int i = 0; i < _ssa_graph->_edges_data_association.size(); ++i) {
                EdgePointXYCovPointXYCov *&edge = _ssa_graph->_edges_data_association[i];
                VertexPointXYCov *from = dynamic_cast<VertexPointXYCov * >(edge->vertices()[0]);
                VertexPointXYCov *to = dynamic_cast<VertexPointXYCov * >(edge->vertices()[1]);
                glBegin(GL_LINES);
                glVertex3f(from->estimate()[0], from->estimate()[1], 0.0f);
                glVertex3f(to->estimate()[0], to->estimate()[1], 0.0f);
                glEnd();
            }

        ///draw observation covariances
        if (false)
            for (g2o::OptimizableGraph::EdgeSet::iterator it = _ssa_graph->_optimizer.edges().begin();
                 it != _ssa_graph->_optimizer.edges().end(); it++) {
                EdgeSE2PointXYCov *e = dynamic_cast<EdgeSE2PointXYCov *>(*it);
                if (e) {
                    g2o::VertexSE2 *from = dynamic_cast<g2o::VertexSE2 *>(e->vertices()[0]);
                    ssa::VertexPointXYCov *to = dynamic_cast<ssa::VertexPointXYCov *>(e->vertices()[1]);
                    if (from->id() != selectedName() || from == 0 || to == 0)
                        continue;
                    glPushMatrix();
                    glTranslatef(to->estimate()(0), to->estimate()(1), 0.0f);
                    Eigen::Matrix2d cov = e->information().inverse();
                    drawCovariance(cov, from->estimate().rotation().angle());
                    glPopMatrix();

                }
            }
    }


    void SSA2DglWidget::drawMeshEdges() {
        for (unsigned int i = 0; i < _ssa_graph->_edges_surface_mesh.size(); ++i) {
            glColor3f(0.0, 0.0, 1.0);
            EdgePointXYCovPointXYCov *&edge = _ssa_graph->_edges_surface_mesh[i];
            VertexPointXYCov *from = dynamic_cast<VertexPointXYCov * >(edge->vertices()[0]);
            VertexPointXYCov *to = dynamic_cast<VertexPointXYCov * >(edge->vertices()[1]);
            if (from->parentVertex()->id() != selectedName() && to->parentVertex()->id() != selectedName())
                continue;
            glBegin(GL_LINES);
            glVertex3f(from->estimate()[0], from->estimate()[1], from->estimate()[2]);
            glVertex3f(to->estimate()[0], to->estimate()[1], to->estimate()[2]);
            glEnd();

        }
    }


    void SSA2DglWidget::drawCovariance() {
//   for(unsigned int i = 0; i < _ssa_graph->_meshEdges.size(); ++i){
//      EdgePointXYCovPointXYCov*& edge = _ssa_graph->_meshEdges[i];
//      VertexPointXYCov* from = dynamic_cast<VertexPointXYCov* >(edge->vertices()[0]);
//      if(from->parentVertex()->id() != selectedName())
//         continue;
//     glPushMatrix();
//       glColor3f(0.0, 1.0, 0.0);
//       Vector2d lambda = from->lambda();
//       Matrix4d transf = from->covTransform();
//       glMatrixMode(GL_MODELVIEW);
//       glMultMatrixd(transf.data());
//       drawEllipsoid(lambda(0), lambda(1), lambda(2));
//     glPopMatrix();
//   }
    }

    void SSA2DglWidget::drawEllipsoid(double &l1, double &l2, double &l3) {
        GLboolean hasNormalization = glIsEnabled(GL_NORMALIZE);
        if (!hasNormalization)
            glEnable(GL_NORMALIZE);

        glPushMatrix();
        glScalef(l1, l2, l3);
        gluSphere(GLUWrapper::getQuadradic(), 10.0, 32, 32);
        glPopMatrix();

        if (!hasNormalization)
            glDisable(GL_NORMALIZE);
    }

    void SSA2DglWidget::drawCircle(GLfloat radius, int segments) {
        double angleStep = DEG2RAD(360.0 / (segments));
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= segments; i++) {
            double angle = i * angleStep;
            float x = radius * cos(angle);
            float y = radius * sin(angle);
            glVertex3f(x, y, 0.f);
        }
        glEnd();
    }


    void SSA2DglWidget::drawGrid() {
        glColor3f(0.7, 0.7, 0.7);
        int size = 100;
        glBegin(GL_LINES);
        for (int i = -size; i < size; ++i) {
            glVertex3f(i, -size, _height);
            glVertex3f(i, size, _height);
            glVertex3f(-size, i, _height);
            glVertex3f(size, i, _height);
        }
        glEnd();

        glColor3f(0.8, 0.8, 0.9);
        size = 1000;
        glBegin(GL_LINES);
        for (int i = -size; i < size; ++i) {
            glVertex3f(0.1 * i, 0.1 * -size, _height);
            glVertex3f(0.1 * i, 0.1 * size, _height);
            glVertex3f(0.1 * -size, 0.1 * i, _height);
            glVertex3f(0.1 * size, 0.1 * i, _height);
        }
        glEnd();

    }


    void SSA2DglWidget::drawPoseBox() {
        glPushMatrix();
        glScalef(0.5, 1, 1);
        glPushMatrix();
        glScalef(1, 0.25, 0.5);
        glTranslatef(-0.5, 0.5, 0);
        glColor3f(1.0, 0.3, 0.3);
        drawBox(1, 1, 1);
        glPopMatrix();

        glPushMatrix();
        glScalef(1, 0.25, 0.5);
        glTranslatef(-0.5, -0.5, 0);
        glColor3f(1.0, 0.1, 0.1);
        drawBox(1, 1, 1);
        glPopMatrix();

        glPushMatrix();
        glScalef(1, 0.25, 0.5);
        glTranslatef(+0.5, 0.5, 0);
        glColor3f(0.3, 0.3, 1.0);
        drawBox(1, 1, 1);
        glPopMatrix();

        glPushMatrix();
        glScalef(1, 0.25, 0.5);
        glTranslatef(+0.5, -0.5, 0);
        glColor3f(0.1, 0.1, 1.);
        drawBox(1, 1, 1);
        glPopMatrix();
        glPopMatrix();
    }

    void SSA2DglWidget::drawBox(GLfloat l, GLfloat w, GLfloat h) {
        GLfloat sx = l * 0.5f;
        GLfloat sy = w * 0.5f;
        GLfloat sz = h * 0.5f;

        glBegin(GL_QUADS);
        // bottom
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(-sx, -sy, -sz);
        glVertex3f(-sx, sy, -sz);
        glVertex3f(sx, sy, -sz);
        glVertex3f(sx, -sy, -sz);
        // top
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-sx, -sy, sz);
        glVertex3f(-sx, sy, sz);
        glVertex3f(sx, sy, sz);
        glVertex3f(sx, -sy, sz);
        // back
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(-sx, -sy, -sz);
        glVertex3f(-sx, sy, -sz);
        glVertex3f(-sx, sy, sz);
        glVertex3f(-sx, -sy, sz);
        // front
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(sx, -sy, -sz);
        glVertex3f(sx, sy, -sz);
        glVertex3f(sx, sy, sz);
        glVertex3f(sx, -sy, sz);
        // left
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-sx, -sy, -sz);
        glVertex3f(sx, -sy, -sz);
        glVertex3f(sx, -sy, sz);
        glVertex3f(-sx, -sy, sz);
        //right
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-sx, sy, -sz);
        glVertex3f(sx, sy, -sz);
        glVertex3f(sx, sy, sz);
        glVertex3f(-sx, sy, sz);
        glEnd();
    }

    void SSA2DglWidget::postSelection(const QPoint &point) {
        if (selectedName() != -1) {
            qglviewer::Vec orig;
            qglviewer::Vec dir;

            // Compute orig and dir, used to draw a representation of the intersecting line
            camera()->convertClickToLine(point, orig, dir);
            bool found;
            _selectedPoint = camera()->pointUnderPixel(point, found);
            cerr << "Selected Vertex " << selectedName() << endl;
            Gen3DObjectList_updateSelection();
        }
        //cerr << selectedName() << std::endl;
    }

    void SSA2DglWidget::setOptimize() {
        _optimize = true;
    }


    void SSA2DglWidget::openFile() {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open Graph"), ".", tr("Graph Files (*.ssa *.ssa3d)"));

        ifstream graph(fileName.toStdString().c_str());
        if (!graph) {
            cerr << "error in loading the graph" << endl;
        }
        _ssa_graph->_optimizer.load(graph);
        _ssa_graph->linkNodesToVertices();
        cerr << "loaded baGraph with " << _ssa_graph->_optimizer.vertices().size() << " vertices and "
             << _ssa_graph->_optimizer.edges().size() << " edges." << std::endl;

    }

    void SSA2DglWidget::saveFile() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Graph"),
                                                        ".",
                                                        tr("Graph Files (*.ssa *.ssa3d)"));
        _ssa_graph->_optimizer.save(fileName.toStdString().c_str());
    }


    GLint SSA2DglWidget::Gen3DObjectList_poseVertices() {
        GLint lid = glGenLists(1);
        //double size = 0.1;
        glNewList(lid, GL_COMPILE);
        if (_ssa_graph)
            for (g2o::OptimizableGraph::VertexIDMap::const_iterator it = _ssa_graph->_optimizer.vertices().begin();
                 it != _ssa_graph->_optimizer.vertices().end(); it++) {
                g2o::VertexSE2 *v = dynamic_cast<g2o::VertexSE2 *>(it->second);
                if (v) {
                    if (_drawVertices) {
                        glPushMatrix();
                        glTranslatef(v->estimate().translation()(0), v->estimate().translation()(1), 0.0f);
                        glRotatef(rad2deg(v->estimate().rotation().angle()), 0.0f, 0.0f, 1.0);
                        glScalef(0.1f, 0.1f, 0.f);
                        drawPoseBox();
                        glPopMatrix();
                    }
                }
            }
        glEndList();
        return lid;
    };

    void SSA2DglWidget::drawVertices() {
        // Draw vertices
        glPointSize(_pointSize * 100);
        std::vector<int> indices = _ssa_graph->getPoseIds();
        for (size_t i = 0; i < indices.size(); ++i) {
            std::vector < ssa::VertexPointXYCov * > vertices = _ssa_graph->getPointVertices(indices[i], _level);
            for (std::vector<ssa::VertexPointXYCov *>::const_iterator it = vertices.begin();
                 it != vertices.end(); it++) {
                ssa::VertexPointXYCov *v = (*it);
                if (v) {
                    if (v->covariance() == Eigen::Matrix2d::Identity())
                        continue;
                    glColor3ub(v->cr, v->cg, v->cb);
                    glBegin(GL_POINTS);
                    glVertex3f(v->estimate()(0), v->estimate()(1), 0.0);
                    glEnd();
                }
            }

        }

    }

    GLint SSA2DglWidget::Gen3DObjectList_obsVertices() {
        GLint lid = glGenLists(1);
        glNewList(lid, GL_COMPILE);
        glPointSize(_pointSize * 100);

        // Draw LandmarkCov Nodes
        if (_ssa_graph)
            drawVertices();

        glEndList();
        return lid;
    };

    GLint SSA2DglWidget::Gen3DObjectList_normals() {
        double normalLength = 0.05;

        GLint lid = glGenLists(1);
        glNewList(lid, GL_COMPILE);
        glPointSize(_pointSize * 100);

        if (_ssa_graph)
            for (g2o::OptimizableGraph::VertexIDMap::const_iterator it = _ssa_graph->_optimizer.vertices().begin();
                 it != _ssa_graph->_optimizer.vertices().end(); it++) {
                VertexPointXYCov *v = dynamic_cast<VertexPointXYCov *>(it->second);
                if (v) {
                    if (v->parentVertex()->id() != selectedName())
                        continue;
                    Vector2d normal = v->globalNormal() * normalLength;
                    glColor3f(0.0, 0.8, 0.0);
                    glPushMatrix();
                    glTranslatef(v->estimate()(0), v->estimate()(1), 0.0f);
                    glBegin(GL_LINES);
                    glVertex3f(0.0, 0.0, 0.0);
                    glVertex3f(normal(0), normal(1), 0.0f);
                    glEnd();
                    if (v->covariance() != Eigen::Matrix2d::Identity()) {
                        glColor3f(0.0, 0.0, 1.0);
                        drawCovariance(v->covariance(), 0.0);
                    }
                    glPopMatrix();
                }
            }

//   for(unsigned int i = 0; i < _ssa_graph->_meshEdges.size(); ++i){
//      EdgePointXYCovPointXYCov*& edge = _ssa_graph->_meshEdges[i];
//      VertexPointXYCov* from = dynamic_cast<VertexPointXYCov* >(edge->vertices()[0]);
//      if(from->parentVertex()->id() != selectedName())
//         continue;
// //     //draw covariance ellipsoid
// //     glPushMatrix();
// //       glColor3f(0.0, 1.0, 0.0);
// //       Vector2d& lambda = from->lambda();
// //       Matrix4d transf = from->covTransform();
// //       glMatrixMode(GL_MODELVIEW);
// //       glMultMatrixd(transf.data());
// //       drawEllipsoid(lambda(0), lambda(1), lambda(2));
// //     glPopMatrix();
//
//   }

        glEndList();
        return lid;
    };

    void SSA2DglWidget::drawEllipse(double l1, double l2) {
        float x, y;
        glBegin(GL_LINE_LOOP);
        for (double i = -M_PI; i <= M_PI; i += .1) {
            x = l1 * sin(i);
            y = l2 * cos(i);
            glVertex3f(x, y, _height);
        }
        glEnd();
    }

    void SSA2DglWidget::drawCovariance(Matrix2d &cov, double angle = 0.0) {
        const double &a = cov(0, 0);
        const double &b = cov(0, 1);
        const double &d = cov(1, 1);
        /* get eigen-values */
        double D = a * d - b * b; // determinant of the matrix
        double T = a + d;       // Trace of the matrix
        double h = sqrt(0.25 * (T * T) - D);
        double lambda1 = 0.5 * T + h;  // solving characteristic polynom using p-q-formula
        double lambda2 = 0.5 * T - h;
        double theta = 0.5 * atan2(2.0 * b, a - d);
        lambda1 = 3.0 * sqrt(lambda1); //3.0
        lambda2 = 3.0 * sqrt(lambda2);

        glRotatef(rad2deg(theta + angle), 0.0, 0.0, 1.0);
        drawEllipse(lambda1, lambda2);
    }

    GLint SSA2DglWidget::Gen3DObjectList_mesh() {
        GLint lid = glGenLists(1);
        glNewList(lid, GL_COMPILE);
        glPointSize(_pointSize * 100);

        if (_ssa_graph)
            for (unsigned int i = 0; i < _ssa_graph->_edges_surface_mesh.size(); ++i) {
                glColor3f(0.0, 0.0, 1.0);
                EdgePointXYCovPointXYCov *&edge = _ssa_graph->_edges_surface_mesh[i];
                VertexPointXYCov *from = dynamic_cast<VertexPointXYCov * >(edge->vertices()[0]);
                VertexPointXYCov *to = dynamic_cast<VertexPointXYCov * >(edge->vertices()[1]);
//      if(from->parentVertex()->id() != selectedName() && to->parentVertex()->id() != selectedName())
//         continue;
                glBegin(GL_LINES);
                glVertex3f(from->estimate()[0], from->estimate()[1], 0.0f);
                glVertex3f(to->estimate()[0], to->estimate()[1], 0.0f);
                glEnd();
            }

        glEndList();
        return lid;
    };


    void SSA2DglWidget::Gen3DObjectList_update() {
        if (_poseVertices != 0)
            glDeleteLists(_poseVertices, 1);
        if (_obsVertices != 0)
            glDeleteLists(_obsVertices, 1);
        if (_normals != 0)
            glDeleteLists(_normals, 1);
//   if(_mesh != 0)
//     glDeleteLists(_mesh, 1);

        _poseVertices = Gen3DObjectList_poseVertices();
        _obsVertices = Gen3DObjectList_obsVertices();
        _normals = Gen3DObjectList_normals();
//   _mesh = Gen3DObjectList_mesh();
    }

    void SSA2DglWidget::Gen3DObjectList_updateSelection() {
        glDeleteLists(_normals, 1);
        glDeleteLists(_mesh, 1);

        _normals = Gen3DObjectList_normals();
        _mesh = Gen3DObjectList_mesh();
    }

    void SSA2DglWidget::saveSnapshotCustom() {
        cerr << __PRETTY_FUNCTION__ << ": not yet implemented." << endl;
        QDateTime current;
        QString fileName = "snapshot-";
        fileName.append(current.currentDateTime().toString("dd_MM_yy-hh_mm_ss"));

        QString stateFileName = fileName;
        stateFileName.append(".xml");

        QString snapshotFileName = fileName;
        snapshotFileName.append(".png");
        setSnapshotFileName(snapshotFileName);
        setSnapshotFormat("PNG");
        setSnapshotQuality(99);
        saveSnapshot(snapshotFileName, true);

        setStateFileName(stateFileName);
        saveStateToFile();
    }


    void SSA2DglWidget::saveSnapshotVideo() {
        QDateTime current;
        char fileName[2048];
        sprintf(fileName, "snapshot-%03d.png", snapshotCounter());

        QString snapshotFileName = fileName;
        setSnapshotFileName(snapshotFileName);
        setSnapshotFormat("PNG");
        setSnapshotQuality(99);
        saveSnapshot(snapshotFileName, true);
        int i = snapshotCounter();
        setSnapshotCounter(++i);
    }

}