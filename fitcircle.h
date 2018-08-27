#ifndef FITCIRCLE_H
#define FITCIRCLE_H

#include "ceres/ceres.h"
#include "eigen3/Eigen/Eigen"
#include "opencv2/opencv.hpp"

class FitCircle
{
    class CircleError
    {
        public:
        CircleError( const cv::Point2f& image_point )
        : image_point_( image_point )
        {
        }

        template< typename T >
        bool operator( )( const T* const circle, T* residuals ) const
        {
            T xc = T( circle[0] );
            T yc = T( circle[1] );
            T R  = T( circle[2] );

            T dx = T( image_point_.x ) - xc;
            T dy = T( image_point_.y ) - yc;

            residuals[0] = R * R - ( dx * dx + dy * dy );

            return true;
        }

        cv::Point2f image_point_;
    };

    public:
    FitCircle( ) {}

    void readin_points( const std::vector< cv::Point2f >& _pts_2 )
    {
        point_num = _pts_2.size( );

        pts_2.clear( );

        std::copy( _pts_2.begin( ), _pts_2.end( ), std::back_inserter( pts_2 ) );
    }

    void solve( )
    {

        // initialize the params to something close to the gt
        double ext[] = { 0, 0, 600 };

        ceres::Problem problem;

        for ( int i = 0; i < point_num; ++i )
        {
            ceres::CostFunction* costFunction
            = new ceres::AutoDiffCostFunction< CircleError, 1, 3 >( new CircleError( pts_2[i] ) );

            problem.AddResidualBlock( costFunction, NULL /* squared loss */, ext );
        }

        ceres::Solver::Options options;
        options.minimizer_progress_to_stdout = true;

        ceres::Solver::Summary summary;
        ceres::Solve( options, &problem, &summary );

        x = ext[0];
        y = ext[1];
        R = ext[2];
    }

    double getX( ) const { return x; }
    double getY( ) const { return y; }
    double getR( ) const { return R; }

    double x, y, R;
    double data[3];
    int point_num;
    std::vector< cv::Point2f > pts_2;
};

#endif // FITCIRCLE_H
