// Sparse Surface Optimization 
// Copyright (C) 2011 M. Ruhnke, R. Kuemmerle, G. Grisetti, W. Burgard
// 
// SSA is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// SSA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef __SSA_EDGE_POINTXYZCOV_POINTXYZCOV__
#define __SSA_EDGE_POINTXYZCOV_POINTXYZCOV__

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/Eigenvalues>
#include "vertex_point_xyzcov.h"
#include "g2o/types/slam3d/types_slam3d.h"
#include "g2o/core/base_binary_edge.h"

namespace ssa {
    using namespace Eigen;
    using namespace g2o;

    class EdgePointXYZCovPointXYZCov : public BaseBinaryEdge<3, Eigen::Vector3d, VertexPointXYZCov, VertexPointXYZCov> {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

        EdgePointXYZCovPointXYZCov();

        void computeError();

        static bool DataAssociationEdgeComp(EdgePointXYZCovPointXYZCov *i, EdgePointXYZCovPointXYZCov *j) {
            return (i->chi2() < j->chi2());
        }

        virtual bool read(std::istream &is);

        virtual bool write(std::ostream &os) const;

        virtual void initialEstimate(const OptimizableGraph::VertexSet &from, OptimizableGraph::Vertex *to);

        virtual double initialEstimatePossible(const OptimizableGraph::VertexSet &from, OptimizableGraph::Vertex *to) {
            (void) to;
            return (from.count(_vertices[0]) == 1 ? 1.0 : -1.0);
        }

#ifndef NUMERIC_JACOBIAN_THREE_D_TYPES

        virtual void linearizeOplus();

#endif

    };

}
#endif
