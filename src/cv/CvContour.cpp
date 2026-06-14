#include "CvContour.hpp"
#include "CvRegion.hpp"
#include <opencv2/imgproc.hpp>


/////////////////////////////////////////////////////////////////////////

static int orientation(
    const cv::Point2f& p,
    const cv::Point2f& q,
    const cv::Point2f& r)
{
    double val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

    if (std::abs(val) < 1e-9) return 0;  // collinear

    return val > 0 ? 1 : 2;  // 1 = clockwise, 2 = counterclockwise
}

static bool onSegment(
    const cv::Point2f& p,
    const cv::Point2f& q,
    const cv::Point2f& r)
{
    return std::min(p.x, r.x) <= q.x &&
        q.x <= std::max(p.x, r.x) &&
        std::min(p.y, r.y) <= q.y &&
        q.y <= std::max(p.y, r.y);
}

static bool doIntersect(
    const cv::Point2f& p1, const cv::Point2f& q1,
    const cv::Point2f& p2, const cv::Point2f& q2)
{
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // General case: proper crossing
    if (o1 != o2 && o3 != o4) return true;

    // Special collinear cases (overlap counts as intersection)
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false;
}

static double sqDist(
    const cv::Point2f& a,
    const cv::Point2f& b)
{
    cv::Point2f d = a - b;

    return d.x * d.x + d.y * d.y;
}

/////////////////////////////////////////////////////////////////////////

CvContour::CvContour(std::vector<cv::Point2f> pts) : pts_(std::move(pts))
{
}

CvContour::CvContour(const std::vector<cv::Point>& pts)
{
    for (const auto& p : pts)
    {
        pts_.emplace_back(
            static_cast<int>(p.x),
            static_cast<int>(p.y)
        );
    }
}

/////////////////////////////////////////////////////////////////////////

CvContour CvContour::join(std::vector<std::vector<cv::Point2f>> pts)
{
    std::vector<cv::Point2f> points;

    for (auto& vec : pts)
    {
        points.insert(points.end(),
            std::make_move_iterator(vec.begin()),
            std::make_move_iterator(vec.end())
        );
    }

	return CvContour(std::move(points));
}

CvContour CvContour::join(const std::vector<CvContour>& contours)
{
    std::vector<cv::Point2f> points;

    for (const auto& c : contours)
    {
        const std::vector<cv::Point2f>& cPts = c.points();

        points.insert(
            points.end(),
            cPts.begin(),
            cPts.end()
        );
    }

    return CvContour(std::move(points));
}

/////////////////////////////////////////////////////////////////////////

cv::Rect CvContour::boundingBox() const
{
    return cv::boundingRect(pts_);
}

double CvContour::length(bool closed) const
{
    if (pts_.size() < 2)
        return 0.0;

    return cv::arcLength(pts_, closed);
}

double CvContour::area() const
{
    if (pts_.size() < 3)
        return 0.0;

    return cv::contourArea(pts_);
}

cv::Point2f CvContour::centroid() const
{
    auto M = cv::moments(pts_);

    if (M.m00 == 0) 
        return { 0, 0 };

    return { (float)(M.m10 / M.m00), (float)(M.m01 / M.m00) };
}

bool CvContour::isClosed(float eps) const
{
    if (pts_.size() < 2)
        return false;

    return cv::norm(pts_.front() - pts_.back()) < eps;
}

bool CvContour::hasSelfIntersection() const
{
    if (pts_.size() < 4)
        return false;

    const bool is_closed = isClosed();
    size_t n = pts_.size();
    size_t num_edges = is_closed ? n : (n - 1);

    for (size_t i = 0; i < num_edges; ++i)
    {
        cv::Point2f a = pts_[i];
        size_t i_next = (i + 1) % (is_closed ? n : n);
        cv::Point2f b = pts_[i_next];

        for (size_t j = i + 2; j < num_edges; ++j)
        {
            // Skip the pair of adjacent edges that share a vertex (only relevant for closed contours)
            if (is_closed && i == 0 && j == num_edges - 1)
                continue;

            cv::Point2f c = pts_[j];
            size_t j_next = (j + 1) % (is_closed ? n : n);
            cv::Point2f d = pts_[j_next];

            if (doIntersect(a, b, c, d))
                return true;
        }
    }

    return false;
}

bool CvContour::diameter(
    double* diameter,
    cv::Point2f* p1,
    cv::Point2f* p2) const
{
    // Handle degenerate input
    if (pts_.empty())
    {
		if (diameter) *diameter = 0.0;

		if (p1) *p1 = cv::Point2f(0, 0);

		if (p2) *p2 = cv::Point2f(0, 0);

        return false;
    }

    // Copy and close contour if necessary (standard for diameter computation)
    std::vector<cv::Point2f> pts = pts_;
    if (!isClosed())
        pts.push_back(pts.front());

    // Check for self-intersection
    // if (hasSelfIntersection())
    // {
    // }

    // Compute convex hull
    std::vector<cv::Point2f> hull;
    cv::convexHull(pts, hull, false);

    if (hull.size() == 1)
    {
        if (diameter) *diameter = 0.0;

        if (p1) *p1 = hull.front();

        if (p2) *p2 = hull.front();

        return true;
    }
    else if (hull.size() == 2)
    {
        if (diameter) *diameter = cv::norm(p1 - p2);

        if (p1) *p1 = hull[0];

        if (p2) *p2 = hull[1];

        return true;
    }

    // Rotating calipers
    // Finds the true maximum vertex-to-vertex distance on the convex hull.
    size_t k = 1;
    double maxDistSq = 0.0;
    for (size_t i = 0; i < hull.size(); ++i)
    {
        // Advance k while distance from hull[i] keeps increasing (monotonic on convex hull)
        while (sqDist(hull[i], hull[(k + 1) % hull.size()]) > sqDist(hull[i], hull[k]))
        {
            k = (k + 1) % hull.size();
        }

        double distSq = sqDist(hull[i], hull[k]);
        if (distSq > maxDistSq)
        {
            maxDistSq = distSq;

            if (p1) *p1 = hull[i];

            if (p2) *p2 = hull[k];
        }
    }

    if (diameter) *diameter = std::sqrt(maxDistSq);

    return true;
}

CvRegion CvContour::toRegion(bool filled) const
{
    return CvRegion::fromContours({ *this }, filled);
}

const std::vector<cv::Point2f>& CvContour::points() const
{
    return pts_;
}
