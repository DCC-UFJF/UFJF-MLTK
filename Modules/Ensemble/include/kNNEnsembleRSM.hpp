//
// Created by mateuscmarim on 20/01/2021.
//

#ifndef UFJF_MLTK_KNNENSEMBLE_HPP
#define UFJF_MLTK_KNNENSEMBLE_HPP

#include "Classifier.hpp"
#include "Ensemble.hpp"
#include "KNNClassifier.hpp"
#include "Sampling.hpp"

namespace mltk {
namespace ensemble {
        template<typename T>
        class kNNEnsembleRSM : public Ensemble<T>, public classifier::Classifier<T> {
        private:
            size_t k;
            double r = 0.5;
            std::vector<std::vector<size_t>> subspaces;
        public:
            kNNEnsembleRSM() = default;
            kNNEnsembleRSM(const Data<T> &samples, size_t _k, double _r): k(_k), r(_r) {
                this->samples = make_data<T>(samples);
                this->learners.resize(7);
                this->learners[0] = std::make_shared<classifier::KNNClassifier<T, metrics::dist::Euclidean<T>>>(k);
                this->learners[1] = std::make_shared<classifier::KNNClassifier<T, metrics::dist::Lorentzian<T>>>(k);
                this->learners[2] = std::make_shared<classifier::KNNClassifier<T, metrics::dist::Cosine<T>>>(k);
                this->learners[3] = std::make_shared<classifier::KNNClassifier<T, metrics::dist::Bhattacharyya<T>>>(k);
                this->learners[4] = std::make_shared<classifier::KNNClassifier<T, metrics::dist::Pearson<T>>>(k);
                this->learners[5] = std::make_shared<classifier::KNNClassifier<T, metrics::dist::KullbackLeibler<T>>>(k);
                this->learners[6] = std::make_shared<classifier::KNNClassifier<T, metrics::dist::Hassanat<T>>>(k);

                RSM<double> rsm(r, this->samples->getDim(), this->seed);
                size_t samp_size = this->samples->getSize() / this->learners.size();
                for (size_t i = 0; i < this->learners.size(); i++) {
                    auto feats = rsm(*this->samples);
                    subspaces.push_back(feats);
                    this->learners[i]->setSamples(this->samples->selectFeatures(feats).sampling(samp_size, true, this->seed));
                    this->learners[i]->train();
                }
            }

            bool train() override{

                return true;
            }

            double evaluate(const Point<T> &p, bool raw_value = false) override {
                auto classes = this->samples->getClasses();
                Point<int> votes(classes.size());
                for (size_t i = 0; i < this->learners.size(); i++) {
                    int pred = this->learners[i]->evaluate(p.selectFeatures(subspaces[i]));
                    size_t pred_pos = std::find(classes.begin(), classes.end(), pred) - classes.begin();
                    votes[pred_pos]++;
                }
                return classes[std::max_element(votes.X().begin(), votes.X().end()) - votes.X().begin()];
            }

            std::string getFormulationString() override {
                return this->learners[0]->getFormulationString();
            }
        };
    }
}
#endif //UFJF_MLTK_KNNENSEMBLE_HPP
