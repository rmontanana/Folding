#pragma once
#include <torch/torch.h>
#include <algorithm>
#include <map>
#include <random> 
#include <vector>
namespace folding {
    const std::string FOLDING_VERSION = "1.1.0";
    class Fold {
    public:
        inline Fold(int k, int m, int seed = -1) : k(k), m(m), seed(seed)
        {
            std::random_device rd;
            random_seed = std::mt19937(seed == -1 ? rd() : seed);
            std::srand(seed == -1 ? time(0) : seed);
        }
        virtual std::pair<std::vector<int>, std::vector<int>> getFold(int nFold) = 0;
        virtual ~Fold() = default;
        std::string version() { return FOLDING_VERSION; }
        int getNumberOfFolds() { return k; }
    protected:
        int k;
        int m;
        int seed;
        std::mt19937 random_seed;
    };
    class KFold : public Fold {
    public:
        inline KFold(int k, int m, int seed = -1) : Fold(k, m, seed), indices(std::vector<int>(m))
        {
            std::iota(begin(indices), end(indices), 0); // fill with 0, 1, ..., n - 1
            std::shuffle(indices.begin(), indices.end(), random_seed);
        }
        inline std::pair<std::vector<int>, std::vector<int>> getFold(int nFold) override
        {
            if (nFold >= k || nFold < 0) {
                throw std::out_of_range("nFold (" + std::to_string(nFold) + ") must be less than k (" + std::to_string(k) + ")");
            }
            int nTest = m / k;
            auto train = std::vector<int>();
            auto test = std::vector<int>();
            for (int i = 0; i < m; i++) {
                if (i >= nTest * nFold && i < nTest * (nFold + 1)) {
                    test.push_back(indices[i]);
                } else {
                    train.push_back(indices[i]);
                }
            }
            return { train, test };
        }
    private:
        std::vector<int> indices;
    };
    class StratifiedKFold : public Fold {
    public:
        inline StratifiedKFold(int k, const std::vector<int>& y, int seed = -1) : Fold(k, y.size(), seed)
        {
            m = y.size();
            this->y = y;
            build();
        }
        inline StratifiedKFold(int k, torch::Tensor& y, int seed = -1) : Fold(k, y.numel(), seed)
        {
            m = y.numel();
            this->y = std::vector<int>(y.data_ptr<int>(), y.data_ptr<int>() + m);
            build();
        }

        inline std::pair<std::vector<int>, std::vector<int>> getFold(int nFold) override
        {
            if (nFold >= k || nFold < 0) {
                throw std::out_of_range("nFold (" + std::to_string(nFold) + ") must be less than k (" + std::to_string(k) + ")");
            }
            std::vector<int> test_indices = stratified_indices[nFold];
            std::vector<int> train_indices;
            for (int i = 0; i < k; ++i) {
                if (i == nFold) continue;
                train_indices.insert(train_indices.end(), stratified_indices[i].begin(), stratified_indices[i].end());
            }
            return { train_indices, test_indices };
        }
        inline bool isFaulty() { return faulty; }
    private:
        std::vector<int> y;
        std::vector<std::vector<int>> stratified_indices;
        bool faulty = false; // Only true if the number of samples of any class is less than the number of folds.
        void build()
        {
            stratified_indices = std::vector<std::vector<int>>(k);
            // Compute class counts and indices
            auto class_indices = std::map<int, std::vector<int>>();
            std::vector<int> class_counts(*max_element(y.begin(), y.end()) + 1, 0);
            for (auto i = 0; i < m; ++i) {
                class_counts[y[i]]++;
                class_indices[y[i]].push_back(i);
            }
            // Assign indices to folds
            for (auto [label, indices] : class_indices) {
                shuffle(indices.begin(), indices.end(), random_seed);
                int num_samples = indices.size();
                int samples_per_fold = num_samples / k;
                int remainder_samples_to_take = num_samples % k;
                if (samples_per_fold == 0) {
                    std::cerr << "Warning! The number of samples in class " << label << " (" << num_samples
                        << ") is less than the number of folds (" << k << ")." << std::endl;
                    faulty = true;
                }
                int start = 0;
                // auto chosen2 = std::vector<int>(k);
                // if (remainder_samples_to_take > 0) {
                //     iota(chosen2.begin(), chosen2.end(), 0);
                //     shuffle(chosen2.begin(), chosen2.end(), random_seed);


                // }
                if (samples_per_fold != 0) {
                    for (auto fold = 0; fold < k; ++fold) {
                        // auto it = next(indices.begin() + start, samples_per_fold);
                        // move(indices.begin() + start, it, back_inserter(stratified_indices[fold]));
                        auto it = next(class_indices[label].begin(), samples_per_fold);
                        move(class_indices[label].begin(), it, back_inserter(stratified_indices[fold]));
                        start += samples_per_fold;
                        class_indices[label].erase(class_indices[label].begin(), it);
                    }
                }
                auto chosen = std::vector<bool>(k, false);
                while (remainder_samples_to_take > 0) {
                    int fold = (rand() % static_cast<int>(k));
                    if (chosen.at(fold)) {
                        continue;
                    }
                    chosen[fold] = true;
                    // auto it = next(indices.begin() + start, 1);
                    auto it = next(indices.begin(), 1);
                    stratified_indices[fold].push_back(class_indices[label][0]);
                    start++;
                    class_indices[label].erase(class_indices[label].begin(), it);
                    remainder_samples_to_take--;
                }
            }
        }
    };
}