// Sparse Surface Optimization 2D
// Copyright (C) 2011 M. Ruhnke, R. Kuemmerle, G. Grisetti, W. Burgard
// 
// SSA2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// SSA2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __SSA_LOCAL_REFINEMENT__
#define __SSA_LOCAL_REFINEMENT__

#include <vector>
#include "ssa/core/ssa_graph.h"
#include "ssa/core/parameter.h"
#include "correspondence_rejection.h"

namespace ssa {

    template<typename EdgeType1, typename EdgeType2, typename EdgeType3>
    class LocalRefinement {

        typedef EdgeType1 SLAMEdgeType;
        typedef EdgeType2 SensorEdgeType;
        typedef EdgeType3 DataAssociationEdgeType;

        typedef typename SensorEdgeType::VertexXiType PoseVertex;
        static const int Di = SensorEdgeType::Dimension;
        typedef Eigen::Matrix<double, Di, Di> PoseMatrix;

        typedef typename SensorEdgeType::VertexXjType PointVertex;
        static const int Dj = SensorEdgeType::Dimension;
        typedef Eigen::Matrix<double, Dj, 1> PointVector;
        typedef Eigen::Matrix<double, Dj, Dj> PointMatrix;


    public:
        LocalRefinement();

        ~LocalRefinement();

        /** \brief creates data association edges for all available scan pairs  */
        static void apply(SparseSurfaceAdjustmentGraphT<EdgeType1, EdgeType2, EdgeType3> &graph,
                          SparseSurfaceAdjustmentParams &params, int level);

        /** \brief creates an edge between reference and correspondence */
        static void refine(EdgeType3 *correspondence, std::vector<PointVertex *> &neighbors);

    };
}

#include "local_refinement.hpp"

#endif
