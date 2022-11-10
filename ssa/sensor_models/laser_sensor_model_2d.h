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

#ifndef __SSA_LASER_SENSOR_MODEL_2D__
#define __SSA_LASER_SENSOR_MODEL_2D__

#include <Eigen/Core>

#include "ssa/core/ssa_graph_2d.h"

namespace ssa {

    struct LaserSensorParams {

        inline LaserSensorParams() {
            maxRange = 80.0;
            angularResolution = DEG2RAD(0.5);
            sensorPrecision = 0.01;
            scale = 0.5;
        }

        double maxRange;
        double angularResolution;
        double sensorPrecision;
        double scale;
    };

    class LaserSensorModel2D {

    public:
        LaserSensorModel2D();

        ~LaserSensorModel2D();

        //--------------------------
        // Laser sensor model
        //--------------------------
        static void applySensorModel(SparseSurfaceAdjustmentGraph2D &ssaGraph, LaserSensorParams &params);

        static void
        setInformationForVertexPoint(EdgeSE2PointXYCov *&edge, VertexPointXYCov *&point, LaserSensorParams &params);

    };

}
#endif