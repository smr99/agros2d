// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#ifndef SCENEVIEW_POST_H
#define SCENEVIEW_POST_H

#undef signals
#include <deal.II/base/point.h>
#include <deal.II/numerics/data_out.h>
#define signals public


#include "util.h"
#include "sceneview_common.h"

template <typename Scalar> class SceneSolution;
template <typename Scalar> class MultiArray;

class MultiArrayDeal;

class ParticleTracing;
class FieldInfo;

struct PostTriangle
{
    PostTriangle(dealii::Point<2> a, dealii::Point<2> b, dealii::Point<2> c,
                 double va, double vb, double vc)
    {
        this->vertices[0] = a;
        this->vertices[1] = b;
        this->vertices[2] = c;

        this->values[0] = va;
        this->values[1] = vb;
        this->values[2] = vc;
    }

    dealii::Point<2> vertices[3];
    double values[3];
};

class DataPostprocessor : public dealii::DataOut<2>
{
public:
    DataPostprocessor();

    void compute_nodes(QList<PostTriangle> &scalarValues);

    virtual typename dealii::DataOut<2>::cell_iterator first_cell();
    virtual typename dealii::DataOut<2>::cell_iterator next_cell(const typename dealii::DataOut<2>::cell_iterator &old_cell);

    inline double min() const { return m_min; }
    inline double max() const { return m_max; }

private:
    double m_min;
    double m_max;

    std::vector<int> m_subdomains;

    void compute_node(dealii::Point<2> &node, const dealii::DataOutBase::Patch<2> *patch,
                      const unsigned int xstep, const unsigned int ystep, const unsigned int zstep,
                      const unsigned int n_subdivisions);
};

class PostHermes : public QObject
{
    Q_OBJECT

public:
    PostHermes();
    ~PostHermes();

    // mesh
    inline Hermes::Hermes2D::Views::Linearizer *linInitialMeshView() { return m_linInitialMeshView; }
    inline Hermes::Hermes2D::Views::Linearizer *linSolutionMeshView() { return m_linSolutionMeshView; }

    // order view
    Hermes::Hermes2D::Views::Orderizer *ordView() { return m_orderView; }

    // contour
    inline QList<PostTriangle> &contourValues() { return m_contourValues; }

    // scalar view
    inline QList<PostTriangle> &scalarValues() { return m_scalarValues; }

    // vector view
    inline Hermes::Hermes2D::Views::Vectorizer *vecVectorView() { return m_vecVectorView; }
    inline QList<PostTriangle> &vectorXValues() { return m_vectorXValues; }
    inline QList<PostTriangle> &vectorYValues() { return m_vectorYValues; }

    // vtk dealii view
    QString scalarViewDeal();

    std::shared_ptr<DataPostprocessor> viewScalarFilter(Module::LocalVariable physicFieldVariable,
                                                        PhysicFieldVariableComp physicFieldVariableComp);

    // view
    inline FieldInfo* activeViewField() const { return m_activeViewField; } // assert(m_activeViewField);
    void setActiveViewField(FieldInfo* fieldInfo);

    inline int activeTimeStep() const { return m_activeTimeStep; }
    void setActiveTimeStep(int ts);

    inline int activeAdaptivityStep() const { return m_activeAdaptivityStep; }
    void setActiveAdaptivityStep(int as);

    inline SolutionMode activeAdaptivitySolutionType() const { return m_activeSolutionMode; }
    void setActiveAdaptivitySolutionType(SolutionMode st) { m_activeSolutionMode = st; }

    MultiArray<double> activeMultiSolutionArray();
    MultiArrayDeal activeMultiSolutionArrayDeal();

    inline bool isProcessed() const { return m_isProcessed; }

signals:
    void processed();

public slots:
    void refresh();
    void clear();
    void clearView();

private:
    bool m_isProcessed;

    // initial mesh
    Hermes::Hermes2D::Views::Linearizer *m_linInitialMeshView;

    // solution mesh
    Hermes::Hermes2D::Views::Linearizer *m_linSolutionMeshView;

    // order view
    Hermes::Hermes2D::Views::Orderizer *m_orderView;

    // vector view
    Hermes::Hermes2D::Views::Vectorizer *m_vecVectorView; // vectorizer for vector view

    // contour
    QList<PostTriangle> m_contourValues;
    // scalar view
    QList<PostTriangle> m_scalarValues;
    // vector view
    QList<PostTriangle> m_vectorXValues;
    QList<PostTriangle> m_vectorYValues;

    // view
    FieldInfo *m_activeViewField;
    int m_activeTimeStep;
    int m_activeAdaptivityStep;
    SolutionMode m_activeSolutionMode;


private slots:
    void processMeshed();
    void processSolved();

    void processInitialMesh();
    void processSolutionMesh();
    void processOrder();

    void processRangeContour();
    void processRangeScalar();
    void processRangeVector();

    virtual void clearGLLists() {}

    void problemMeshed();
    void problemSolved();
};

class SceneViewPostInterface : public SceneViewCommon
{
    Q_OBJECT

public:
    SceneViewPostInterface(PostHermes *postHermes, QWidget *parent = 0);

    inline PostHermes *postHermes() { assert(m_postHermes); return m_postHermes; }

protected:
    double m_texScale;
    double m_texShift;

    GLuint m_textureScalar;

    PostHermes *m_postHermes;

    virtual void initializeGL();

    void paintScalarFieldColorBar(double min, double max);

    // palette
    const double *paletteColor(double x) const;
    const double *paletteColorOrder(int n) const;
    void paletteCreate();

protected slots:
    virtual void clearGLLists() {}
};

#endif // SCENEVIEW_POST_H
