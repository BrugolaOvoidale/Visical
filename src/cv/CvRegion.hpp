#pragma once
#include <opencv2/core/mat.hpp>


// Forward declarations
class CvContour;


/**
 * @brief Represents a binary region using a compact representation based on an OpenCV mask.
 *
 * A CvRegion models a binary region using a compact CV_8UC1 OpenCV mask
 * together with a bounding box defining its position in a global image coordinate system.
 *
 * Unlike full-image masks, CvRegion stores only the minimal enclosing region
 * (bounding box + local mask), making it efficient for sparse geometries and
 * large images.
 *
 * The class supports:
 *  - Creation from geometric primitives (contours, polygons, point sets)
 *  - Region-based logical and morphological operations
 *  - Connected component analysis
 *  - Conversion between local region space and full image space
 *
 * Coordinate model:
 *  - mask() is expressed in local coordinates (origin = bounding box top-left)
 *  - boundingBox() defines the region position in the global image space
 */
class CvRegion
{
public:
	//-----------------------------------------------------------------------------
    // Enums
    //-----------------------------------------------------------------------------

    /**
     * @brief Defines how contour boundaries are interpreted when extracted
     *        from the region mask.
     */
    enum class ContourMode {
        CENTER,         ///< Centers of border pixels
        BORDER,         ///< Outer border of border pixels
        BORDER_HOLES    ///< Outer border + holes
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructor from a a shallow copy of the input matrix.
	 * @throws std::invalid_argument if the input matrix is not CV_8UC1 or contains values other than 0 and 255.
     */
    CvRegion(
        const cv::Mat& m,
        const cv::Rect& bbox
    );

    /** @brief Copy constructor. Performs a shallow copy of the underlying cv::Mat. */
    CvRegion(const CvRegion& other);

    /** @brief Returns a deep copy of the region and its underlying mask. */
    CvRegion clone() const;

    ~CvRegion() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Creates a region from a single polygon.
     * @param polygon A sequence of points defining the vertices.
     * @return A CvRegion containing the filled polygon.
     */
    static CvRegion fromPolygon(const std::vector<cv::Point>& polygon);

    /**
     * @brief Creates a region from a set of points.
     * @param points Floating-point coordinates to be marked in the mask.
     */
    static CvRegion fromPoints(const std::vector<cv::Point2f>& points);

    /**
     * @brief Creates a region from multiple sets of points.
     * @param points A vector of point vectors.
     */
    static CvRegion fromPoints(const std::vector<std::vector<cv::Point2f>>& points);

    /**
     * @brief Creates a rectangular region.
     * @param row1 Starting row index.
     * @param col1 Starting column index.
     * @param row2 Ending row index (inclusive).
     * @param col2 Ending column index (inclusive).
     */
    static CvRegion fromRectangle(
        int row1, int col1,
        int row2, int col2
    );

    /**
     * @brief Creates a region from a list of contours.
     * @param contours The source contours.
     * @param filled If true, the interior of the contours is filled; otherwise, only boundaries are drawn.
     */
    static CvRegion fromContours(
        const std::vector<CvContour>& contours,
        bool filled
    );
    /** @overload */
    static CvRegion fromContours(
        const std::vector<std::vector<CvContour>>& contours,
        bool filled
    );


    //-----------------------------------------------------------------------------
    // Logical operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the intersection of two regions. Masks must be the same size. */
    CvRegion operator&(const CvRegion& other) const;

    /** @brief Returns the union of two regions. Masks must be the same size. */
    CvRegion operator|(const CvRegion& other) const;

    /** @brief Returns the set difference (this NOT other). Masks must be the same size. */
    CvRegion operator-(const CvRegion& other) const;


    //-----------------------------------------------------------------------------
	// Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the dimensions of the mask. */
    cv::Size size() const;

    /** @brief Returns the smallest upright bounding rectangle containing all non-zero pixels. */
    const cv::Rect& boundingBox() const;

    /**
     * @brief Calculates the area and the center of mass (centroid).
     * @param[out] row Pointer to store the centroid Y-coordinate.
     * @param[out] col Pointer to store the centroid X-coordinate.
     * @return The total number of non-zero pixels (area).
     */
    double areaCenter(
        double* row = nullptr,
        double* col = nullptr
    ) const;

    /** @brief Returns the number of disconnected blobs in the mask. */
    int countConnectedComponents() const;

    /** @brief Splits the mask into a vector of regions, one for each connected component. */
    std::vector<CvRegion> connectedComponents() const;

    /** @brief Computes the mean diameter of the region based on its area. */
    double diameterRegionMean() const;

    /**
     * @brief Performs a morphological dilation using a circular kernel.
     * @param radius The radius of the circular structuring element.
     */
    CvRegion dilationCircle(double radius) const;

    /**
     * @brief Performs a morphological dilation using a rectangular kernel.
     * @param width The width of the rectangular structuring element.
     * @param height The height of the rectangular structuring element.
     */
    CvRegion dilationRectangle(
        double width,
        double height
    ) const;

    /**
     * @brief Performs a morphological erosion using a rectangular kernel.
     * @param width The width of the rectangular structuring element.
     * @param height The height of the rectangular structuring element.
     */
    CvRegion erosionRectangle(
        double width,
        double height
    ) const;

    /**
     * @brief Extracts contours from the binary mask.
     * @param mode The interpretation mode for boundaries.
     */
    std::vector<CvContour> toContours(ContourMode mode = ContourMode::BORDER) const;

    /** @brief Returns a list of all pixel coordinates that are part of the region. */
    const std::vector<cv::Point>& points() const;

    /** @brief Saves the mask to a file. @return true if successful. */
    bool write(const std::string& path) const;

    /** @brief Access the underlying OpenCV CV_8UC1 mask. */
    const cv::Mat& mask() const;

    /**
     * @brief Expands the region into a full-size binary mask.
     *
     * Converts the compact representation (local mask + bounding box)
     * into a full-image binary mask aligned to the given image size.
     *
     * The region is placed into the output image using its bounding box,
     * where:
     *   - mask_(0,0) corresponds to boundingBox().tl()
     *   - all pixels outside the region are set to zero
     * 
     * This operation materializes the region into full raster space
     *       and is therefore more expensive than operations on the compact form.
     * 
     * @param imageSize Target full image size (width, height)
     * @return CV_8UC1 binary mask in global image coordinates
     *
     */
    cv::Mat toMask(cv::Size imageSize) const;

private:
    // Private constructor for internal use
    CvRegion() = default;

    // Resets all cached analysis data. Must be called if mask_ is modified
    void invalidateCache();

    void computeConnectedComponents() const;

    void computeRegionPoints() const;

private:
    // compact local binary mask, CV_8UC1
    cv::Mat mask_;
    // global coordinates of mask_
    cv::Rect boundingBox_;

    // cached labels image
    mutable cv::Mat cachedLabels_;
    mutable cv::Mat cachedStats_;
    mutable cv::Mat cachedCentroids_;
    mutable int cachedNLabels_ = -1;

    // whether cache is valid
    mutable bool cacheLabelsValid_ = false;
    
    // cached non-zero mask points
    mutable std::vector<cv::Point> cachedPoints_;
    mutable bool cachePtsValid_ = false;
};
