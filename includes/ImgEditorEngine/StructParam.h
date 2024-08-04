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
    float m_threshold;  // Separate threshold for Regions
    float m_adaptation; // Separate threshold for Regions

    int m_seednb;
    int m_comp_algorithm;
    bool m_random;
    bool m_divide;
    bool m_rg_type; //Condition whether or not the region growing will be "adaptative"

    SeedRGParameter() : m_threshold(5.0f), m_adaptation(50), m_seednb(100), m_comp_algorithm(0),
                        m_random(true), m_divide(true), m_rg_type(true){};
    SeedRGParameter(
        float threshold, float adaptation, int seednb,
        int comp_algorithm,
        bool random, bool divide, bool rg_type)
        : m_threshold(threshold), m_adaptation(adaptation), m_seednb(seednb),
          m_comp_algorithm(comp_algorithm),
          m_random(random), m_divide(divide), m_rg_type(rg_type){};
};

/*
int testregiongrowing(int argc, char **argv)
{

    if (img.getChannels() == 3)
    {


        std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

        std::cout << "growing A " << std::endl;

        std::cout << "growing A done" << std::endl;
        std::chrono::high_resolution_clock::time_point stop = std::chrono::high_resolution_clock::now();

        durationa = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    }

    int taillerega = 0;
    for (int i = 0; i < regions.size(); i++) {
        //std::cout << regions[i].getSizePix() << "regions a with is first being " <<std::endl;

        // std::cout<<regions[i].pixels[0] -> getId() << regions[i].getMeanIntensity()<< std::endl;
        taillerega += regions[i].getSizePix();
    }
    float percentSegmenteda = taillerega * 100 / (img.getRows() * img.getCols());

    std::cout << percentSegmenteda << " de l'image à été segementé en " << durationa.count() << " millisecondes" << std::endl;
}
*/
#endif