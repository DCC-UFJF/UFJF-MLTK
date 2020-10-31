#include <iostream>
#include <UFJF-MLTK/Regressor/include/LMS.hpp>
#include <UFJF-MLTK/Validation/include/Validation.hpp>
#include <UFJF-MLTK/Visualize/include/Visualization.hpp>
#include <UFJF-MLTK/Core/include/Data.hpp>

int main(int argc, char *argv[]){
    Data<double> data;

    data.load("iris.data");

    std::cout << "Dataset size: " << data.getSize() << std::endl;
    std::cout << "Dataset dimension: " << data.getDim() << std::endl;
    std::cout << std::endl;
    // for(auto it = data.begin(); it != data.end(); it++){
    //     std::cout << *(*it) << std::endl;
    // }
    // std::cout << std::endl;
    auto distribution = data.getClassesDistribution();

    for(auto dist: distribution){
        std::cout << dist << std::endl;
    }

    std::cout << std::endl;
    for(int i = 0; i < 10; i++){
        auto p = Point<double>(data.getDim(), 0);
        p.y = (i%5 == 0)?-1:5;
        data.insertPoint(p);
    }

    for(int i = 0; i < 10; i++){
        auto p = Point<double>(data.getDim(), 0);
        p.y = -1;
        data.insertPoint(p);
    }
    // std::cout << data << std::endl;
    // std::cout << std::endl;


    distribution = data.getClassesDistribution();

    for(auto dist: distribution){
        std::cout << dist << std::endl;
    }
    std::cout << std::endl;
    auto class_names = data.getClassNames();
    for(auto class_name: class_names){
        std::cout << class_name << std::endl;
    }
}