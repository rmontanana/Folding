// ***************************************************************
// SPDX-FileCopyrightText: Copyright 2024 Ricardo Montañana Gómez
// SPDX-FileType: SOURCE
// SPDX-License-Identifier: MIT
// ***************************************************************

#pragma once
#include <torch/torch.h>
#include <algorithm>
#include <map>
#include <random> 
#include <vector>
#include <folding_config.h>
namespace folding {
    class Fold {
    public:
        inline Fold(int k, int n, int seed = -1) : k(k), n(n), seed(seed)
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
        int n;
        int seed;
        std::mt19937 random_seed;
    };
    class KFold : public Fold {
    public:
        inline KFold(int k, int n, int seed = -1) : Fold(k, n, seed), indices(std::vector<int>(n))
        {
            std::iota(begin(indices), end(indices), 0); // fill with 0, 1, ..., n - 1
            shuffle(indices.begin(), indices.end(), random_seed);
        }
        inline std::pair<std::vector<int>, std::vector<int>> getFold(int nFold) override
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
    private:
        std::vector<int> indices;
    };
    class StratifiedKFold : public Fold {
    public:
        inline StratifiedKFold(int k, const std::vector<int>& y, int seed = -1, bool quiet = true) : Fold(k, y.size(), seed)
        {
            this->y = y;
            n = y.size();
            this->quiet = quiet;
            build();
        }
        inline StratifiedKFold(int k, torch::Tensor& y, int seed = -1, bool quiet = true) : Fold(k, y.numel(), seed)
        {
            n = y.numel();
            this->y = std::vector<int>(y.data_ptr<int>(), y.data_ptr<int>() + n);
            this->quiet = quiet;
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
        bool quiet = true; // Enable or disable warning messages
        void build()
        {
            stratified_indices = std::vector<std::vector<int>>(k);
            // Compute class counts and indices
            auto class_indices = std::map<int, std::vector<int>>();
            for (auto i = 0; i < n; ++i) {
                class_indices[y[i]].push_back(i);
            }
            // Assign indices to folds
            for (auto& [label, indices] : class_indices) {
                shuffle(indices.begin(), indices.end(), random_seed);
                int num_samples = indices.size();
                int num_samples_to_take = num_samples / k;
                int remainder_samples_to_take = num_samples % k;
                if (num_samples_to_take == 0) {
                    if (!quiet)
                        std::cerr << "Warning! The number of samples in class " << label << " (" << num_samples
                        << ") is less than the number of folds (" << k << ")." << std::endl;
                    faulty = true;
                }
                int start = 0;
                if (num_samples_to_take > 0) {
                    for (auto fold = 0; fold < k; ++fold) {
                        auto it = next(class_indices[label].begin() + start, num_samples_to_take);
                        move(indices.begin() + start, it, back_inserter(stratified_indices[fold]));
                        start += num_samples_to_take;
                    }
                }
                if (remainder_samples_to_take > 0) {
                    auto chosen = std::vector<int>(k);
                    std::iota(chosen.begin(), chosen.end(), 0);
                    std::shuffle(chosen.begin(), chosen.end(), random_seed);
                    chosen.resize(remainder_samples_to_take);
                    for (auto fold : chosen) {
                        stratified_indices[fold].push_back(indices.at(start++));
                    }
                }
            }
        }
    };
}