#include "layoutPyramidalBased.hpp"

using namespace IMT;

double LayoutPyramidalBased::UsePlanEquation(double x) const //Use the plan equation to compute the second (non nul) variable knowing the value of x
{
    return -m_canonicTopPlan[3] - x * m_canonicTopPlan[0];
}

Layout::NormalizedFaceInfo LayoutPyramidalBased::From3dToNormalizedFaceInfo(const Coord3dSpherical& sphericalCoord) const
{
    auto f = Faces::Last;
    Coord3dCart inter;
    double minRho = std::numeric_limits<double>::max();


    //Transform to the canonic representation
    Coord3dSpherical p = Rotation(sphericalCoord, -m_yaw, -m_pitch, -m_roll);

    for (auto testF: get_range<LayoutPyramidalBased::Faces>())
    {
        try {
            auto plan = FaceToPlan(testF);
            auto interSphe = IntersectionPlanSpherical(plan, p); //raise exception if no intersection
            if (minRho > interSphe.x && AlmostEqual(interSphe.y, p.y)) //check direction
            {
                minRho = interSphe.x;
                inter = SphericalToCart(interSphe);
                f = testF;
            }
        } catch ( std::logic_error& le )
        { //no intersection with this face
            //continue;
        }
    }
    double normalizedI(0);
    double normalizedJ(0);
    switch (f)
    {
        case Faces::Base:
            normalizedI = inter.y/m_baseEdge+0.5;
            normalizedJ = -inter.z/m_baseEdge+0.5;
            break;
        case Faces::Left:
            normalizedI = (inter.x + m_pyramidHeight)/(m_pyramidHeight+1);
            normalizedJ = -inter.z/(normalizedI*m_baseEdge)+0.5;
            break;
        case Faces::Right:
            normalizedI = (1-inter.x)/(m_pyramidHeight+1);
            normalizedJ = -inter.z/((1-normalizedI)*m_baseEdge)+0.5;
            break;
        case Faces::Top:
            normalizedJ = (inter.x + m_pyramidHeight)/(m_pyramidHeight+1);
            normalizedI = inter.y/(normalizedJ*m_baseEdge)+0.5;
            break;
        case Faces::Bottom:
            normalizedJ = (1-inter.x)/(m_pyramidHeight+1);
            normalizedI = inter.y/((1-normalizedJ)*m_baseEdge)+0.5;
            break;
         case Faces::Black:
         case Faces::Last:
            {
               std::cout << "Error: no intersection detected..." << std::endl;
               return Layout::NormalizedFaceInfo(CoordF(0, 0),static_cast<int>(Faces::Last));
            }
    }
    return Layout::NormalizedFaceInfo(CoordF(normalizedI, normalizedJ),static_cast<int>(f));
}
Coord3dCart LayoutPyramidalBased::FromNormalizedInfoTo3d(const NormalizedFaceInfo& ni) const
{
    auto f = static_cast<Faces>(ni.m_faceId);
    const CoordF& coord (ni.m_normalizedFaceCoordinate);
    const double& normalizedI (coord.x);
    const double& normalizedJ (coord.y);
    switch (f)
    {
    case Faces::Base:
        {
            Coord3dCart inter(1, (normalizedI-0.5)*m_baseEdge, (0.5 - normalizedJ)*m_baseEdge);
            return Rotation(inter, m_yaw, m_pitch, m_roll);
        }
    case Faces::Left:
        {
            double z = -normalizedI*(normalizedJ-0.5) * m_baseEdge;
            double x = (m_pyramidHeight+1.0)*normalizedI-m_pyramidHeight;
            double y = -UsePlanEquation(x);
            return Rotation(Coord3dCart(x,y,z), m_yaw, m_pitch, m_roll);
        }
    case Faces::Top:
        {
            double y = (normalizedI-0.5)*normalizedJ * m_baseEdge;
            double x =  (m_pyramidHeight+1.0)*normalizedJ-m_pyramidHeight;
            double z = UsePlanEquation(x);
            return Rotation(Coord3dCart(x,y,z), m_yaw, m_pitch, m_roll);
        }
    case Faces::Right:
        {
            double z = -((normalizedJ-0.5)*(1-normalizedI)) * m_baseEdge;
            double x = -(m_pyramidHeight+1.0)*normalizedI+1;
            double y = UsePlanEquation(x);
            return Rotation(Coord3dCart(x,y,z), m_yaw, m_pitch, m_roll);
        }
    case Faces::Bottom:
        {
            double y = ((normalizedI-0.5)*(1-normalizedJ)) * m_baseEdge;
            double x = -(m_pyramidHeight+1.0)*normalizedJ+1;
            double z = -UsePlanEquation(x);
            return Rotation(Coord3dCart(x,y,z), m_yaw, m_pitch, m_roll);
        }
    case Faces::Last:
        throw std::invalid_argument("FromNormalizedInfoTo3d: Last is not a valid face");
    case Faces::Black:
        return Coord3dCart(0,0,0);
    }
}