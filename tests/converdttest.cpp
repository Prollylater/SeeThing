#include <iostream>
#include "Mat.h"


int main(){

Mat<float> a(2,2,2);

    a.atChannel(0, 0,1) = 5.4f;
    a.atChannel(0, 0,0) = 0.4f;
    a.atChannel(1, 1,0) = 3.4f;
    a.atChannel(1, 1,1) = 0.4f;
    a.atChannel(1, 0,1) = 2.4f;
    a.atChannel(1, 0,0) = 1.4f;
    a.atChannel(0, 1,1) = 2.4f;
    a.atChannel(0, 1,0) = 1.4f;
    
    a.at(0, 1)[0] = 12.4f;
    a.at(0, 1)[1] = 32.4f;
    std::cout<< a<<std::endl;
    Mat<int> b = a.convertDt<int>();



    std::cout<< b<<std::endl;;

   

    
return 0;
}