#include <iostream>
#include "Mat.h"


int main(){

Mat<float> a(2,2,2);
Mat<int> b(2,2,1);   
Mat<int> c(4,4,2);   


    a.atChannel(0, 0,1) = 5.4f;
    a.atChannel(0, 0,0) = 0.4f;
    a.atChannel(1, 1,0) = 3.4f;
    a.atChannel(1, 1,1) = 0.4f;
    a.atChannel(1, 0,1) = 2.4f;
    a.atChannel(1, 0,0) = 1.4f;
    a.atChannel(0, 1,1) = 2.4f;
    a.atChannel(0, 1,0) = 1.4f;
    std::cout<< a<<std::endl;

    a.at(0, 1)[0] = 12.4f;
    a.at(0, 1)[1] = 32.4f;
    std::cout<< a<<std::endl;

    b.atChannel(0, 0,0) = 3;
    b.atChannel(0, 1,0) = 1;
    b.atChannel(1, 1,0) = 4;
    b.atChannel(1, 0,0) =1;
    std::cout<< b<<std::endl;;

    c.zeros();
    std::cout<< c<<std::endl;;


    
return 0;
}