#ifndef ALG_PARAM
#define ALG_PARAM

/// @brief
struct SlicParameter
{
    int m_K;
    int m_m;
    int m_divstr;
    int m_XYZconv;
    bool m_divbool;

    SlicParameter() : m_K(100), m_m(), m_divstr(10), m_XYZconv(3), m_divbool(true){};
    SlicParameter(int K, int m, int divstr, int XYZconv, bool divbool) : m_K(K), m_m(), m_divstr(divstr), m_XYZconv(XYZconv), m_divbool(divbool){};
}

;

struct SeedRGParameter
{
    bool random;
};

#endif