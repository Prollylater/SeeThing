#ifndef ALG_PARAM
#define ALG_PARAM

/// @brief
struct SlicParameter;
struct LabParam;

struct LabParam
{
    int m_divstr;
    bool m_divbool;
    int m_XYZconv;
    LabParam() : m_divstr(10), m_XYZconv(3), m_divbool(true){};
    LabParam(int divstr, int XYZconv, bool divbool) : m_divstr(divstr), m_XYZconv(XYZconv), m_divbool(divbool){};
};

struct SlicParameter
{
    int m_K;
    int m_m;
    LabParam m_labparam;

    SlicParameter() : m_K(100), m_m(), m_labparam(){};
    SlicParameter(int K, int m, int divstr, int XYZconv, bool divbool) : m_K(K), m_m(m), m_labparam(divstr, XYZconv, divbool){};
};

struct SeedRGParameter
{
    bool random;
};

#endif