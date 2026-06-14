#pragma once
#include <opencv2/core/types.hpp>


// Forward declarations
class CvRegion;


/**
 * @brief Represents a geometric sequence of 2D points.
 *
 * It provides vector-based geometric analysis.
 */
class CvContour
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /** @brief Default constructor creating an empty contour. */
    CvContour() = default;

    /** @brief Construct from a vector of floating-point points. */
    explicit CvContour(std::vector<cv::Point2f> pts);

    /** @brief Construct from a vector of integer points. */
    explicit CvContour(const std::vector<cv::Point>& pts);

    ~CvContour() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Merges multiple CvContour objects into one.
     * @param pts A vector of point sequences to join.
     */
    static CvContour join(std::vector<std::vector<cv::Point2f>> pts);

    /**
     * @brief Merges multiple CvContour objects into one.
     * @param contours The source contours to be joined.
     */
    static CvContour join(const std::vector<CvContour>& contours);


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the rectangle of the smallest upright bounding box for the contour. */
    cv::Rect boundingBox() const;

    /**
     * @brief Calculates the total length of the contour.
     * @param closed Whether to consider the distance between the last and first point.
     */
    double length(bool closed = true) const;

    /**
     * @brief Calculates the signed area of the contour using the Green's theorem.
     * @note Only valid for closed, non-self-intersecting contours.
     */
    double area() const;

    /** @brief Calculates the center of mass (centroid) of the contour points. */
    cv::Point2f centroid() const;

    /**
     * @brief Checks if the start and end points are within a specific distance.
     * @param eps The maximum distance threshold to consider the contour closed.
     */
    bool isClosed(float eps = 1e-3f) const;

    /** @brief Checks if any segments of the contour cross each other. */
    bool hasSelfIntersection() const;

    /**
     * @brief Finds the two most distant points on the contour (Rotating Calipers).
     * @param[out] p1 First point of the maximum diameter.
     * @param[out] p2 Second point of the maximum diameter.
     * @param[out] diameter The distance between p1 and p2.
     * @return true if the diameter was successfully calculated.
     */
    bool diameter(
        double* diameter,
        cv::Point2f* p1 = nullptr,
        cv::Point2f* p2 = nullptr
    ) const;

    /**
     * @brief Rasterizes the contour into a binary mask region.
     * @param filled If true, the interior of the contour is filled (requires a closed contour).
     * @return A CvRegion object representing the rasterized contour.
     */
    CvRegion toRegion(bool filled) const;

    /** @brief Read-only access to the underlying point vector. */
    const std::vector<cv::Point2f>& points() const;

private:
    std::vector<cv::Point2f> pts_;
};
