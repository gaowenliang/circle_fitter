#include "fitcircle.h"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int edgeThresh    = 1;
int dilation_elem = 0;
int dilation_size = 0;
int h_range       = 0;
int p1            = 0;
int p2            = 0;

std::vector< cv::Mat > hsvs;
cv::Mat filter_img;
int intensity_seed;
bool is_seed = false;

cv::Mat image_right, image_dst, dilation_dst;
bool first          = true;
bool is_scaled      = false;
double length_pixel = 0;

void
showIMG( std::string name, cv::Mat img )
{
    cv::namedWindow( name, cv::WINDOW_NORMAL );
    cv::imshow( name, img );
}

FitCircle* csolve;

void
Dilation( int, void* )
{
    int dilation_type;

    if ( dilation_elem == 0 )
    {
        dilation_type = MORPH_RECT;
    }
    else if ( dilation_elem == 1 )
    {
        dilation_type = MORPH_CROSS;
    }
    else if ( dilation_elem == 2 )
    {
        dilation_type = MORPH_ELLIPSE;
    }

    Mat element = getStructuringElement( dilation_type,
                                         Size( 2 * dilation_size + 1, 2 * dilation_size + 1 ),
                                         Point( dilation_size, dilation_size ) );
    dilate( image_right, dilation_dst, element );

    showIMG( "Dilation Demo", dilation_dst );

    cv::Mat image_hsv;
    cv::cvtColor( dilation_dst, image_hsv, COLOR_BGR2HSV );
    // showIMG( "image_hsv", image_hsv );

    cv::split( image_hsv, hsvs );
    // showIMG( "image_h", hsvs[0]);
    // showIMG( "image_s", hsvs[1]);
    // showIMG( "image_v", hsvs[2]);

    cv::Mat tmp255( image_hsv.rows, image_hsv.cols, CV_8UC1, cv::Scalar( 255 ) );
    cv::Mat inves_h = tmp255 - hsvs[0];
    // showIMG( "inves_h", inves_h);

    cv::Mat mix_sv = ( 0.33 * hsvs[1] + 0.33 * hsvs[2] + 0.33 * inves_h );
    showIMG( "mix_sv", mix_sv );
    if ( is_seed )
    {
        cv::inRange( mix_sv, intensity_seed - h_range, intensity_seed + h_range, filter_img );
        showIMG( "filter_img", filter_img );

        if ( 0 )
        {
            std::vector< cv::Point2f > samples;
            for ( int index_r = 0; index_r < filter_img.rows; ++index_r )
                for ( int index_c = 0; index_c < filter_img.cols - 1; ++index_c )
                {
                    if ( filter_img.at< uchar >( index_r, index_c )
                         < filter_img.at< uchar >( index_r, index_c + 1 ) )
                    {
                        samples.push_back( cv::Point2f( index_c + 1, index_r ) );
                        break;
                    }
                }

            std::cout << " pt " << samples.size( ) << "\n";
            image_right.copyTo( image_dst );

            csolve = new FitCircle( );
            csolve->readin_points( samples );
            csolve->solve( );

            int x = csolve->getX( );
            int y = csolve->getY( );
            int R = csolve->getR( );

            Point center( x, y );
            float radius = R;

            circle( image_dst, center, 5, Scalar( 0, 255, 255 ), -1, 8, 0 );
            circle( image_dst, center, radius, Scalar( 0, 255, 255 ), 4, 8, 0 );

            std::cout << "radius " << radius << "\n";
            if ( is_scaled )
            {

                double r_mm = radius * 50 / length_pixel;
                std::cout << "radius " << radius << " pixels"
                          << "\n";
                std::cout << "radius " << r_mm << " mm"
                          << "\n";
                std::cout << "k " << 1 / r_mm << " mm^-1"
                          << "\n";
            }
        }

        if ( 1 )
        {
            Canny( filter_img, filter_img, 3, 9, 3 );

            cv::Mat blur_img;
            GaussianBlur( filter_img, blur_img, Size( 9, 9 ), 2, 2 );
            showIMG( "blur_img", blur_img );

            vector< Vec3f > circles;
            HoughCircles( blur_img, circles, CV_HOUGH_GRADIENT, 0.5, 30, 100, p2 + 1, 0, 0 );
            std::cout << "\n"
                      << "find circles " << circles.size( ) << "\n";

            image_right.copyTo( image_dst );
            for ( size_t i = 0; i < circles.size( ); i++ )
            {
                Point center( cvRound( circles[i][0] ), cvRound( circles[i][1] ) );
                float radius = cvRound( circles[i][2] );

                circle( image_dst, center, 5, Scalar( 0, 255, 255 ), -1, 8, 0 );
                circle( image_dst, center, radius, Scalar( 0, 255, 255 ), 4, 8, 0 );

                std::cout << "radius " << radius << "\n";
                if ( is_scaled )
                {

                    double r_mm = radius * 50 / length_pixel;
                    std::cout << "radius " << radius << " pixels"
                              << "\n";
                    std::cout << "radius " << r_mm << " mm"
                              << "\n";
                    std::cout << "k " << 1 / r_mm << " mm^-1"
                              << "\n";
                }
            }
        }
        showIMG( "src", image_dst );
    }

    // std::cout << "dilation_size " << dilation_size << "\n";
    if ( first )
        first = false;
}

int times;
cv::Point2d pt_left, pt_right;
bool pt_left_get = false, pt_right_get = false;

void
OnMouseAction( int event, int x, int y, int flags, void* ustc )
{
    if ( event == CV_EVENT_LBUTTONDOWN )
    {
        times++;
        cout << "left down " << times << " | " << x << " " << y << endl;
        pt_left.x   = x;
        pt_left.y   = y;
        pt_left_get = true;
    }
    if ( event == CV_EVENT_LBUTTONUP )
    {
        cout << "left up " << endl;
    }
    if ( event == CV_EVENT_RBUTTONDOWN )
    {
        times++;
        cout << "right down " << times << " | " << x << " " << y << endl;
        pt_right.x   = x;
        pt_right.y   = y;
        pt_right_get = true;

        if ( pt_left_get && pt_right_get )
        {
            int dx       = pt_left.x - pt_right.x;
            int dy       = pt_left.y - pt_right.y;
            length_pixel = sqrt( dx * dx + dy * dy );
            cout << "length_pixel " << length_pixel << endl;
            cout << length_pixel / 50 << " pixel/mm " << endl;
            is_scaled = true;
        }
    }
    if ( event == CV_EVENT_RBUTTONUP )
    {
        cout << "right up " << endl;
    }
}

void
OnMouseAction2( int event, int x, int y, int flags, void* ustc )
{
    if ( event == CV_EVENT_LBUTTONDBLCLK )
    {
        intensity_seed = hsvs.at( 1 ).at< uchar >( x, y );
        is_seed        = true;
        cout << "LBUTTONDBLCLK " << times << " | " << x << " " << y << endl;
        cout << "intensity_seed " << intensity_seed << endl;
    }
}

void
onTrackbar( int, void* )
{
    if ( first )
        return;
}

int
main( )
{

    cv::Mat image = imread( "/home/gao/test/color/p035.jpg", IMREAD_UNCHANGED );
    image_right   = image( cv::Rect( image.cols / 2, 0, image.cols / 2, image.rows ) );

    namedWindow( "src", WINDOW_NORMAL );
    showIMG( "src", image_right );
    waitKey( 0 );

    cv::namedWindow( "mix_sv", cv::WINDOW_NORMAL );
    setMouseCallback( "src", OnMouseAction );
    setMouseCallback( "mix_sv", OnMouseAction2 );

    int const max_r_range     = 300;
    int const max_h_range     = 50;
    int const max_kernel_size = 21;
    int const max_elem        = 2;
    // createTrackbar( "Canny Threshold", "src", &edgeThresh, 100, onTrackbar );
    createTrackbar( "Element:", "src", &dilation_elem, max_elem, Dilation );
    createTrackbar( "Kernel", "src", &dilation_size, max_kernel_size, Dilation );
    createTrackbar( "HSV Range", "src", &h_range, max_h_range, Dilation );
    createTrackbar( "p2", "src", &p2, max_r_range, Dilation );

    onTrackbar( 0, 0 );
    Dilation( 0, 0 );

    waitKey( 0 );

    return 0;
}
