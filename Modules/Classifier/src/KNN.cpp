//
// Created by mateus558 on 20/03/2020.
//

#include "KNN.hpp"
#include <memory>

template <typename T >
KNN< T >::KNN(std::shared_ptr<Data<T>> _samples, size_t k, std::function<double(std::shared_ptr<Point< T > >, std::shared_ptr<Point< T > >)> _dist_function){
    this->samples = _samples;
    this->k = k;
    this->dist_function = _dist_function;
}

template <typename T >
double KNN< T >::evaluate(Point<T> p) {
    size_t max_index = 0, max_freq = 0, i;
    auto points = this->samples->getPoints();
    std::vector<double> distances(this->samples->getSize());
    std::vector<int> classes = this->samples->getClasses();
    std::vector<size_t> idx(distances.size()), freq(classes.size());
    auto p0 = std::make_shared<Point< T > >(p);

    // fill the index vector
    std::iota(idx.begin(), idx.end(), 0);
    // compute the distance from the sample to be evaluated to the samples vector
    std::transform(points.begin(), points.end(), distances.begin(), [p0, this](const std::shared_ptr<Point< T > > q){
        return this->dist_function(p0, q);
    });
    // sort the index vector by the distance from the sample to be evaluated
    std::stable_sort(idx.begin(), idx.end(), [&distances](size_t i1, size_t i2){
        return distances[i1] < distances[i2];
    });

    // find the most frequent class in the k nearest neighbors
    for(size_t j = 0; j < this->k; j++){
        for (i = 0; i < classes.size(); i++) {
            if (classes[i] == points[idx[j]]->y) {
                freq[i]++;
                if (freq[i] > max_freq) {
                    max_freq = freq[i];
                    max_index = i;
                }
            }
        }
    }
    return classes[max_index];
}

template<typename T>
bool KNN<T>::train() {
    return false;
}


template class KNN<int>;
template class KNN<double>;
template class KNN<float>;
template class KNN<int8_t>;
template class KNN<char>;
template class KNN<long long int>;
template class KNN<short int>;
template class KNN<long double>;
template class KNN<unsigned char>;
template class KNN<unsigned int>;
template class KNN<unsigned short int>;