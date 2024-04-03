#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "TestUtils.h"
#include "folding.hpp"

TEST_CASE("Version Test", "[Folding]")
{
    std::string actual_version = { folding_project_version.begin(), folding_project_version.end() };
    auto data = std::vector<int>(100);
    folding::StratifiedKFold stratified_kfold(5, data, 17);
    REQUIRE(stratified_kfold.version() == actual_version);
    folding::KFold kfold(5, 100, 19);
    REQUIRE(kfold.version() == actual_version);
}

TEST_CASE("KFold Test", "[Folding]")
{
    // Initialize a KFold object with k=5 and a seed of 19.
    std::string file_name = GENERATE("iris", "diabetes", "glass");
    auto raw = RawDatasets(file_name, true);
    int nFolds = 5;
    folding::KFold kfold(nFolds, raw.nSamples, 19);
    int number = raw.nSamples * (kfold.getNumberOfFolds() - 1) / kfold.getNumberOfFolds();

    SECTION("Number of Folds")
    {
        REQUIRE(kfold.getNumberOfFolds() == nFolds);
    }
    SECTION("Fold Test")
    {
        // Test each fold's size and contents.
        for (int i = 0; i < nFolds; ++i) {
            auto [train_indices, test_indices] = kfold.getFold(i);
            // Store the indices
            auto fname = "kfold_" + file_name + "_" + std::to_string(i) + ".csv";
            auto indices = train_indices;
            indices.insert(indices.end(), test_indices.begin(), test_indices.end());
            // CSVFiles::write_csv(fname, indices);
            auto expected_indices = CSVFiles::read_csv(fname);
            // CHECK(indices == expected_indices);
            bool result = train_indices.size() == number || train_indices.size() == number + 1;
            REQUIRE(result);
            REQUIRE(train_indices.size() + test_indices.size() == raw.nSamples);
        }
    }
}
map<int, int> counts(std::vector<int> y, std::vector<int> indices)
{
    map<int, int> result;
    for (auto i = 0; i < indices.size(); ++i) {
        result[y[indices[i]]]++;
    }
    return result;
}

TEST_CASE("StratifiedKFold Test", "[Folding]")
{
    // Initialize a StratifiedKFold object with k=3, using the y std::vector, and a seed of 17.
    std::string file_name = GENERATE("iris", "diabetes", "glass");
    int nFolds = GENERATE(3, 5, 10);
    auto raw = RawDatasets(file_name, true);
    folding::StratifiedKFold stratified_kfoldt(nFolds, raw.yt, 17);
    folding::StratifiedKFold stratified_kfoldv(nFolds, raw.yv, 17);
    int number = raw.nSamples * (stratified_kfoldt.getNumberOfFolds() - 1) / stratified_kfoldt.getNumberOfFolds();

    SECTION("Stratified Number of Folds")
    {
        REQUIRE(stratified_kfoldt.getNumberOfFolds() == nFolds);
    }
    SECTION("Stratified Fold Test")
    {
        // Test each fold's size and contents.
        auto counts = map<int, std::vector<int>>();
        // Initialize the counts per Fold
        for (int i = 0; i < nFolds; ++i) {
            counts[i] = std::vector<int>(raw.classNumStates, 0);
        }
        // Check fold and compute counts of each fold
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
            auto train_t = torch::tensor(train_indicest);
            auto ytrain = raw.yt.index({ train_t });
            // Check that the class labels have been equally assign to each fold
            for (const auto& idx : train_indicest) {
                counts[fold][raw.yt[idx].item<int>()]++;
            }
        }
        // Test the fold counting of every class
        for (int fold = 0; fold < nFolds; ++fold) {
            for (int j = 1; j < nFolds - 1; ++j) {
                for (int k = 0; k < raw.classNumStates; ++k) {
                    REQUIRE(abs(counts.at(fold).at(k) - counts.at(j).at(k)) <= 1);
                }
            }
        }
    }
}
