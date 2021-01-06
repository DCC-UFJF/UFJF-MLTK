#ifndef OVERSAMPLING_HPP_INCLUDED
#define OVERSAMPLING_HPP_INCLUDED
#pragma once

#include "Data.hpp"
#include "DistanceMetric.hpp"
#include <random>

namespace mltk{    
    /**
     * \brief Base class for the implementation of over sampling methods.
     */
    template < typename T, typename Callable = metrics::dist::Euclidean< T > >
    class OverSampling{
    protected:
        Callable distance_metric;
    public:
        OverSampling()=default;
        explicit OverSampling(Callable dist_metric): distance_metric(dist_metric) {}
        virtual Data< T > operator()(Data< T > &data) = 0;
    };

    /**
     * \brief Functor for the implementation of the SMOTE over sampling algorithm.
     */
    template < typename T, typename Callable = metrics::dist::Euclidean< T > >
    class SMOTE: public OverSampling< T, Callable > {
    private:
        /// Seed used for randomization
        size_t seed = 0;
        /// Number k of neighbors being considered
        size_t k = 5;
        /// Replication factor
        double r = 0.1;

    public:
        explicit SMOTE(size_t k = 1, double r = 0.1, size_t seed = 0, Callable dist_metric = Callable())
        : k(k), r(r), seed(seed), OverSampling<T>(dist_metric) {}

        Data< T > operator()(Data< T > &data) override {
            std::random_device rd;
            seed = (seed > 0)?seed:rd();
            std::mt19937 generator(seed);
            std::uniform_real_distribution<double> distribution(0.0,1.0); 
            // number of generated artificial points
            size_t n_apoints = r * data.getSize();
            // find the minority class
            auto classes = data.getClasses();
            auto class_distribution = data.getClassesDistribution();
            int min_class = classes[std::min_element(class_distribution.begin(), class_distribution.end()) - class_distribution.begin()];
            // copy all the points from the minority class to the Z set
            std::vector<int> class_copy = {min_class};
            std::vector<SamplePointer< T > > artificial_data;
            Data< T > Z;
            Z.classesCopy(data, class_copy);
            
            // iterate through all the elements from the Z set
            for(auto z = Z.begin(); z != Z.end(); ++z){
                std::vector<std::pair<size_t, double> > distance(Z.getSize());
                std::vector<SamplePointer< T > > k_neighbors(k);
                size_t id = 0;
                auto _z = *(*z);

                // compute the metrics from a point to the rest
                std::transform(Z.begin(), Z.end(), distance.begin(), [this, &_z, &id](auto p){
                    id++;
                    return std::make_pair(id, this->distance_metric(_z, *p));
                });

                // sort the distances in increasing metrics order
                std::sort(distance.begin(), distance.end(), [](auto &d1, auto &d2){
                    return d1.second < d2.second;
                });

                // remove points with equal metrics or duplicate ids
                distance.erase(std::unique(distance.begin(), distance.end(), [](auto &d1, auto &d2){
                    return (d1.second == d2.second) || ((d1.first == d2.first));
                }), distance.end());

                distance.erase(std::remove_if(distance.begin(), distance.end(), [](auto &d){
                    return d.second == 0;
                }), distance.end());

                // get the k neighbors
                for(size_t i = 0; (i < k) && (i < distance.size()); i++){
                    k_neighbors[i] = data[distance[i].first-1];
                }

                // create the artificial points and insert them to the dataset
                for(auto p = k_neighbors.begin(); p != k_neighbors.end(); p++){
                    if(!*p) continue;
                    auto _k = *(*p);
                    Point< T > s(_z.size(), 0.0, 0);
                    double alpha = distribution(generator);
                    
                    s = _z + alpha * (_z - _k);
                    s.Y() = min_class;

                    artificial_data.push_back(mltk::make_point< T >(s));
                }
            }

            std::shuffle(artificial_data.begin(), artificial_data.end(), std::default_random_engine(seed));
            for (size_t i = 0; i < artificial_data.size(); i++) {
                data.insertPoint(artificial_data[i]);
            }
            return data;
        }
    }; 

    /**
     * \brief Functor for the implementation of the Borderline SMOTE 1 over sampling algorithm.
     */
    template < typename T, typename Callable = metrics::dist::Euclidean< T > >
    class BorderlineSMOTEOne: public OverSampling< T, Callable > {
    private:
        /// Seed used for randomization
        size_t seed = 0;
        /// Number k of neighbors being considered
        size_t k = 5;
        /// Size of the M set
        size_t m = 5;
        /// Replication factor
        double r = 0.1;
        
    public:
        explicit BorderlineSMOTEOne(size_t k = 1, double r = 0.1, size_t m = 1, size_t seed = 0, Callable dist_metric = Callable())
        : k(k), r(r), m(m), seed(seed), OverSampling<T>(dist_metric) {}

        Data< T > operator()(Data< T > &data) override {
            // Find the majority class
            auto classes = data.getClasses();
            auto class_distribution = data.getClassesDistribution();
            int maj_class = classes[std::max_element(class_distribution.begin(), class_distribution.end()) - class_distribution.begin()];
            // Initialize the danger subset
            Data< T > danger_subset;
            std::set<size_t> danger_ids;

            // iterate through all the elements from the Z set
            for(auto z = data.begin(); z != data.end(); ++z){
                std::vector<std::pair<size_t, double> > distance(data.getSize());
                std::vector<SamplePointer< T > > M(m);
                size_t id = 0;
                auto _z = (*z);

                // compute the euclidean metrics from a point to the rest
                std::transform(data.begin(), data.end(), distance.begin(), [this, &_z, &id](auto p){
                    id++;
                    return std::make_pair(id, this->distance_metric(*_z, *p));
                });

                // sort the distances in increasing metrics order
                std::sort(distance.begin(), distance.end(), [](auto &d1, auto &d2){
                    return d1.second < d2.second;
                });
                
                // remove points with equal metrics or duplicate ids
                distance.erase(std::unique(distance.begin(), distance.end(), [](auto &d1, auto &d2){
                    return (d1.second == d2.second) || ((d1.first == d2.first));
                }), distance.end());

                distance.erase(std::remove_if(distance.begin(), distance.end(), [](auto &d){
                    return d.second == 0;
                }), distance.end());

                // get the m neighbors
                for(size_t i = 0; (i < m) && (i < distance.size()); i++){
                    M[i] = data[distance[i].first-1];
                }
                
                // set m' as the number of points on the majority class set on m neighbors
                size_t m_ = std::count_if(M.begin(), M.end(), [&maj_class](auto &p){
                    return p->Y() == maj_class;
                });
                
                // insert the point to the danger subset if m/2 <= m' < m
                if(m_ >= (m/2) && m_ < m){
                    danger_ids.insert(_z->Id());
                    danger_subset.insertPoint(_z);
                }
            }

            // apply SMOTE algorithm to the danger subset
            if(danger_subset.getSize() > 0){
                SMOTE< T > smote(k, r, seed);
                smote(danger_subset);
            }

            for(auto p = danger_subset.begin(); p != danger_subset.end(); p++){
                auto _p = (*p);

                if(danger_ids.find(_p->Id()) != danger_ids.end()){
                    data.insertPoint(_p);
                }
            }

            return data;
        }
    };
}

#endif //OVERSAMPLING_HPP_INCLUDED