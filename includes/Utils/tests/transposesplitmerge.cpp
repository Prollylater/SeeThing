#include <iostream>
#include "Mat.h"


int main(){

//Transposition
Mat<float> a(3,2,2);
   
Mat<float> b(4,2,1);
    a.atChannel(0, 0,1) = 5.4f;
    a.atChannel(0, 0,0) = 0.4f;
    a.atChannel(1, 1,0) = 3.4f;
    a.atChannel(1, 1,1) = 7.4f;
    a.atChannel(1, 0,1) = 2.4f;
    a.atChannel(1, 0,0) = 1.4f;
    a.atChannel(0, 1,1) = 8.4f;
    a.atChannel(0, 1,0) = 1.4f;


    b.atChannel(0, 0,0) = 5.4f;
    b.atChannel(3, 0,0) = 0.4f;
    b.atChannel(1, 1,0) = 3.4f;
    b.atChannel(3, 1,0) = 8.4f;
    b.atChannel(2, 0,0) = 2.4f;
    b.atChannel(1, 0,0) = 6.4f;
    b.atChannel(2, 1,0) = 9.4f;
    b.atChannel(0, 1,0) = 7.4f;
    
    std::cout<< a<<std::endl;
   Mat<float>ab =  a.transpose();
    std::cout<< ab<<std::endl;
   Mat<float>bb =  b.transpose();
    std::cout<< bb<<std::endl;
    std::cout<< b<<std::endl;


//Split

// MErge
std::vector<float> chan1;
std::vector<float> chan2;

std::cout<< "Channel Pirint"<<std::endl;

 a.splitMat(&chan1, &chan2);
 for(float content : chan1){
 std::cout<< content << " " ;
}
 std::cout<<std::endl;
for(float content : chan2){
 std::cout<< content << " " ;}

 std::cout<< "Channel Printed"<<std::endl;

std::cout<< "Merging"<<std::endl;

Mat<float> cab(3,2,2);
std::cout<< cab<<std::endl;
merge(cab,chan1,chan2);
std::cout<< "Merged"<<std::endl;

std::cout<< cab<<std::endl;

std::cout<< "mergeinto"<<std::endl;

std::vector<std::vector<float>>dsds {chan1,chan2};
Mat<float> dsa = mergeInto(dsds);
    std::cout<< dsa<<std::endl;

return 0;
}