#pragma once
#include <torch/torch.h>
#include <algorithm>
#include <map>
#include <random> 
#include <vector>
namespace folding {
    class Fold {
    protected:
        int k;
        int n;
        int seed;
        std::default_random_engine random_seed;
    public:
        Fold(int k, int n, int seed = -1);
        virtual std::pair<std::vector<int>, std::vector<int>> getFold(int nFold) = 0;
        virtual ~Fold() = default;
        int getNumberOfFolds() { return k; }
    };
    class KFold : public Fold {
    private:
        std::vector<int> indices;
    public:
        KFold(int k, int n, int seed = -1);
        std::pair<std::vector<int>, std::vector<int>> getFold(int nFold) override;
    };
    class StratifiedKFold : public Fold {
    private:
        std::vector<int> y;
        std::vector<std::vector<int>> stratified_indices;
        void build();
        bool faulty = false; // Only true if the number of samples of any class is less than the number of folds.
    public:
        StratifiedKFold(int k, const std::vector<int>& y, int seed = -1);
        StratifiedKFold(int k, torch::Tensor& y, int seed = -1);
        std::pair<std::vector<int>, std::vector<int>> getFold(int nFold) override;
        bool isFaulty() { return faulty; }
    };
    Fold::Fold(int k, int n, int seed) : k(k), n(n), seed(seed)
    {
        std::random_device rd;
        random_seed = std::default_random_engine(seed == -1 ? rd() : seed);
        std::srand(seed == -1 ? time(0) : seed);
    }
    KFold::KFold(int k, int n, int seed) : Fold(k, n, seed), indices(std::vector<int>(n))
    {
        std::iota(begin(indices), end(indices), 0); // fill with 0, 1, ..., n - 1
        shuffle(indices.begin(), indices.end(), random_seed);
    }
    std::pair<std::vector<int>, std::vector<int>> KFold::getFold(int nFold)
    {
        if (nFold >= k || nFold < 0) {
            throw std::out_of_range("nFold (" + std::to_string(nFold) + ") must be less than k (" + std::to_string(k) + ")");
        }
        int nTest = n / k;
        auto train = std::vector<int>();
        auto test = std::vector<int>();
        for (int i = 0; i < n; i++) {
            if (i >= nTest * nFold && i < nTest * (nFold + 1)) {
                test.push_back(indices[i]);
            } else {
                train.push_back(indices[i]);
            }
        }
        return { train, test };
    }
    StratifiedKFold::StratifiedKFold(int k, torch::Tensor& y, int seed) : Fold(k, y.numel(), seed)
    {
        n = y.numel();
        this->y = std::vector<int>(y.data_ptr<int>(), y.data_ptr<int>() + n);
        build();
    }
    StratifiedKFold::StratifiedKFold(int k, const std::vector<int>& y, int seed)
        : Fold(k, y.size(), seed)
    {
        this->y = y;
        n = y.size();
        build();
    }
    void StratifiedKFold::build()
    {
        stratified_indices = std::vector<std::vector<int>>(k);
        int fold_size = n / k;

        // Compute class counts and indices
        auto class_indices = std::map<int, std::vector<int>>();
        std::vector<int> class_counts(*max_element(y.begin(), y.end()) + 1, 0);
        for (auto i = 0; i < n; ++i) {
            class_counts[y[i]]++;
            class_indices[y[i]].push_back(i);
        }
        // Shuffle class indices
        for (auto& [cls, indices] : class_indices) {
            shuffle(indices.begin(), indices.end(), random_seed);
        }
        // Assign indices to folds
        for (auto label = 0; label < class_counts.size(); ++label) {
            auto num_samples_to_take = class_counts.at(label) / k;
            if (num_samples_to_take == 0) {
                std::cerr << "Warning! The number of samples in class " << label << " (" << class_counts.at(label)
                    << ") is less than the number of folds (" << k << ")." << std::endl;
                faulty = true;
                continue;
            }
            auto remainder_samples_to_take = class_counts[label] % k;
            for (auto fold = 0; fold < k; ++fold) {
                auto it = next(class_indices[label].begin(), num_samples_to_take);
                move(class_indices[label].begin(), it, back_inserter(stratified_indices[fold]));  // ##
                class_indices[label].erase(class_indices[label].begin(), it);
            }
            auto chosen = std::vector<bool>(k, false);
            while (remainder_samples_to_take > 0) {
                int fold = (rand() % static_cast<int>(k));
                if (chosen.at(fold)) {
                    continue;
                }
                chosen[fold] = true;
                auto it = next(class_indices[label].begin(), 1);
                stratified_indices[fold].push_back(*class_indices[label].begin());
                class_indices[label].erase(class_indices[label].begin(), it);
                remainder_samples_to_take--;
            }
        }
    }
    std::pair<std::vector<int>, std::vector<int>> StratifiedKFold::getFold(int nFold)
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
}