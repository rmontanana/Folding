// ***************************************************************
// SPDX-FileCopyrightText: Copyright 2024 Ricardo Montañana Gómez
// SPDX-FileType: SOURCE
// SPDX-License-Identifier: MIT
// ***************************************************************

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "TestUtils.h"
#include "folding.hpp"
#include <folding_config.h>

TEST_CASE("Version Test", "[Folding]")
{
    std::string actual_version = FOLDING_VERSION;
    auto data = std::vector<int>(100);
    folding::StratifiedKFold stratified_kfold(5, data, 17);
    REQUIRE(stratified_kfold.version() == actual_version);
    folding::KFold kfold(5, 100, 19);
    REQUIRE(kfold.version() == actual_version);
}

TEST_CASE("KFold Test", "[Folding]")
{
    // Initialize a KFold object with k=3,5,7,10 and a seed of 19.
    std::string file_name = GENERATE("iris", "diabetes", "glass", "mfeat-fourier");
    auto raw = RawDatasets(file_name, true);
    INFO("File Name: " << file_name);
    int nFolds = GENERATE(3, 5, 7, 10);
    INFO("Number of Folds: " << nFolds);
    folding::KFold kfold(nFolds, raw.nSamples, 19);
    int number = raw.nSamples * (kfold.getNumberOfFolds() - 1) / kfold.getNumberOfFolds();

    SECTION("Number of Folds")
    {
        REQUIRE(kfold.getNumberOfFolds() == nFolds);
    }
    SECTION("Fold Test counts")
    {
        // Test each fold's size and contents.
        for (int fold = 0; fold < nFolds; ++fold) {
            auto [train_indices, test_indices] = kfold.getFold(fold);
            // Store the indices
            auto fname = "kfold_" + file_name + "_" + std::to_string(nFolds) + "_" + std::to_string(fold) + ".csv";
            auto indices = train_indices;
            indices.insert(indices.end(), test_indices.begin(), test_indices.end());
            // CSVFiles::write_csv(fname, indices);
            auto expected_indices = CSVFiles::read_csv(fname);
            CHECK(indices == expected_indices);
            bool result = train_indices.size() == number || train_indices.size() == number + 1;
            REQUIRE(result);
            REQUIRE(train_indices.size() + test_indices.size() == raw.nSamples);
        }
    }
    SECTION("Duplicates & overlappings")
    {
        // Check that there are not duplicate samples in the training and test sets.
        for (int fold = 0; fold < nFolds; ++fold) {
            auto [train, test] = kfold.getFold(fold);
            auto train_ = train;
            auto test_ = test;
            sort(train.begin(), train.end());
            train.erase(unique(train.begin(), train.end()), train.end());
            sort(test.begin(), test.end());
            test.erase(unique(test.begin(), test.end()), test.end());
            REQUIRE(train.size() == train_.size());
            REQUIRE(test.size() == test_.size());
            for (int i = 0; i < train.size(); i++) {
                for (int j = 0; j < test.size(); j++) {
                    REQUIRE(train[i] != test[j]);
                }
            }
        }
    }
}

TEST_CASE("StratifiedKFold Test", "[Folding]")
{
    // Initialize a StratifiedKFold object with k=3, using the y std::vector, and a seed of 17.
    std::string file_name = GENERATE("iris", "diabetes", "glass", "mfeat-fourier");
    INFO("File Name: " << file_name);
    int nFolds = GENERATE(3, 5, 7, 10);
    INFO("Number of Folds: " << nFolds);
    auto raw = RawDatasets(file_name, true);
    folding::StratifiedKFold stratified_kfoldt(nFolds, raw.yt, 17);
    folding::StratifiedKFold stratified_kfoldv(nFolds, raw.yv, 17);
    int number = raw.nSamples * (stratified_kfoldt.getNumberOfFolds() - 1) / stratified_kfoldt.getNumberOfFolds();

    SECTION("Stratified Number of Folds")
    {
        REQUIRE(stratified_kfoldt.getNumberOfFolds() == nFolds);
    }
    SECTION("Stratified Fold samples counting")
    {
        // Test each fold's size and contents.
        for (int fold = 0; fold < nFolds; ++fold) {
            auto [train_indicest, test_indicest] = stratified_kfoldt.getFold(fold);
            auto [train_indicesv, test_indicesv] = stratified_kfoldv.getFold(fold);
            REQUIRE(train_indicest == train_indicesv);
            REQUIRE(test_indicest == test_indicesv);
            // Store the indices
            auto fname = "stratkfold_" + file_name + "_" + std::to_string(nFolds) + "_" + std::to_string(fold) + ".csv";
            auto indices = train_indicesv;
            indices.insert(indices.end(), test_indicesv.begin(), test_indicesv.end());
            // CSVFiles::write_csv(fname, indices);
            auto expected_indices = CSVFiles::read_csv(fname);
            // CHECK(indices == expected_indices);
            // In the worst case scenario, the number of samples in the training set is number + raw.classNumStates
            // because in that fold can come one remainder sample from each class.
            REQUIRE(train_indicest.size() <= number + raw.classNumStates);
            // If the number of samples in any class is less than the number of folds, then the fold is faulty.
            // and the number of samples in the training set + test set will be less than nSamples 
            if (!stratified_kfoldt.isFaulty()) {
                REQUIRE(train_indicest.size() + test_indicest.size() == raw.nSamples);
            } else {
                REQUIRE(train_indicest.size() + test_indicest.size() <= raw.nSamples);
            }
        }
    }
    SECTION("Stratified Fold label counting")
    {
        auto counts = std::vector<int>(raw.classNumStates, 0);
        for (auto i = 0; i < raw.nSamples; ++i) {
            counts[raw.yt[i].item<int>()]++;
        }
        auto counts_train = map<int, std::vector<int>>();
        auto counts_test = map<int, std::vector<int>>();
        // Initialize the counts per Fold
        for (int i = 0; i < nFolds; ++i) {
            counts_train[i] = std::vector<int>(raw.classNumStates, 0);
            counts_test[i] = std::vector<int>(raw.classNumStates, 0);
        }
        // Check fold and compute counts of each fold
        for (int fold = 0; fold < nFolds; ++fold) {
            auto [train_indicest, test_indicest] = stratified_kfoldt.getFold(fold);
            auto [train_indicesv, test_indicesv] = stratified_kfoldv.getFold(fold);
            auto train_t = torch::tensor(train_indicest);
            auto ytrain = raw.yt.index({ train_t });
            for (const auto& idx : train_indicest) {
                counts_train[fold][raw.yt[idx].item<int>()]++;
            }
            for (const auto& idx : test_indicest) {
                counts_test[fold][raw.yt[idx].item<int>()]++;
            }
        }
        // Check that the different folds have the same number of samples of each class in train
        for (int fold = 0; fold < nFolds - 1; ++fold) {
            for (int j = fold + 1; j < nFolds; ++j) {
                for (int k = 0; k < raw.classNumStates; ++k) {
                    REQUIRE(std::abs(counts_train.at(fold).at(k) - counts_train.at(j).at(k)) <= 1);
                }
            }
        }
        // Check that the different folds have the same number of samples of each class in tests
        for (int fold = 0; fold < nFolds - 1; ++fold) {
            for (int j = fold + 1; j < nFolds; ++j) {
                for (int k = 0; k < raw.classNumStates; ++k) {
                    REQUIRE(std::abs(counts_test.at(fold).at(k) - counts_test.at(j).at(k)) <= 1);
                }
            }
        }
        // Check that the sum of the counts of each class in the training and test sets is equal to the total count of that class.
        for (int fold = 0; fold < nFolds; ++fold) {
            for (int k = 0; k < raw.classNumStates; ++k) {
                REQUIRE(counts.at(k) == (counts_train.at(fold).at(k) + counts_test.at(fold).at(k)));
            }
        }
    }
    SECTION("Duplicates & overlappings")
    {
        // Check that there are not duplicate samples in the training and test sets.
        for (int fold = 0; fold < nFolds; ++fold) {
            auto [train, test] = stratified_kfoldt.getFold(fold);
            auto train_ = train;
            auto test_ = test;
            sort(train.begin(), train.end());
            train.erase(unique(train.begin(), train.end()), train.end());
            sort(test.begin(), test.end());
            test.erase(unique(test.begin(), test.end()), test.end());
            REQUIRE(train.size() == train_.size());
            REQUIRE(test.size() == test_.size());
            for (int i = 0; i < train.size(); i++) {
                for (int j = 0; j < test.size(); j++) {
                    REQUIRE(train[i] != test[j]);
                }
            }
        }
    }
}
TEST_CASE("Stratified KFold quiet parameter", "[Folding]")
{
    auto raw = RawDatasets("glass", true);
    std::string expected = "Warning! The number of samples in class 2 (9) is less than the number of folds (10).\n";

    SECTION("With vectors")
    {
        // Redirect cerr to a stringstream
        std::streambuf* originalCerrBuffer = std::cerr.rdbuf();
        std::stringstream capturedOutput;
        std::cerr.rdbuf(capturedOutput.rdbuf());
        // StratifiedKFold with quiet parameter set to false
        folding::StratifiedKFold stratified_kfold(10, raw.yv, 17, false);
        // Restore the original cerr buffer
        std::cerr.rdbuf(originalCerrBuffer);
        // Check the captured output
        REQUIRE(capturedOutput.str() == expected);
        REQUIRE(stratified_kfold.isFaulty());
    }
    SECTION("With tensors")
    {
        // Redirect cerr to a stringstream
        std::streambuf* originalCerrBuffer = std::cerr.rdbuf();
        std::stringstream capturedOutput;
        std::cerr.rdbuf(capturedOutput.rdbuf());
        // StratifiedKFold with quiet parameter set to false
        folding::StratifiedKFold stratified_kfold(10, raw.yt, 17, false);
        // Restore the original cerr buffer
        std::cerr.rdbuf(originalCerrBuffer);
        // Check the captured output
        REQUIRE(capturedOutput.str() == expected);
        REQUIRE(stratified_kfold.isFaulty());
    }
}