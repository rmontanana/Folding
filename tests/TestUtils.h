#ifndef TEST_UTILS_H
#define TEST_UTILS_H
#include <fstream>
#include <torch/torch.h>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include "ArffFiles.hpp"
#include "fimdlp/CPPFImdlp.h"
#include "folding_config.h"

bool file_exists(const std::string& name);
std::pair<vector<mdlp::labels_t>, map<std::string, int>> discretize(std::vector<mdlp::samples_t>& X, mdlp::labels_t& y, std::vector<string> features);
std::vector<mdlp::labels_t> discretizeDataset(std::vector<mdlp::samples_t>& X, mdlp::labels_t& y);
std::tuple<vector<vector<int>>, std::vector<int>, std::vector<string>, std::string, map<std::string, std::vector<int>>> loadFile(const std::string& name);
std::tuple<torch::Tensor, torch::Tensor, std::vector<string>, std::string, map<std::string, std::vector<int>>> loadDataset(const std::string& name, bool class_last, bool discretize_dataset);

class RawDatasets {
public:
    RawDatasets(const std::string& file_name, bool discretize)
    {
        // Xt can be either discretized or not
        tie(Xt, yt, featurest, classNamet, statest) = loadDataset(file_name, true, discretize);
        // Xv is always discretized
        tie(Xv, yv, featuresv, classNamev, statesv) = loadFile(file_name);
        auto yresized = torch::transpose(yt.view({ yt.size(0), 1 }), 0, 1);
        dataset = torch::cat({ Xt, yresized }, 0);
        nSamples = dataset.size(1);
        weights = torch::full({ nSamples }, 1.0 / nSamples, torch::kDouble);
        weightsv = std::vector<double>(nSamples, 1.0 / nSamples);
        classNumStates = discretize ? statest.at(classNamet).size() : 0;
    }
    torch::Tensor Xt, yt, dataset, weights;
    std::vector<vector<int>> Xv;
    std::vector<double> weightsv;
    std::vector<int> yv;
    std::vector<string> featurest, featuresv;
    map<std::string, std::vector<int>> statest, statesv;
    std::string classNamet, classNamev;
    int nSamples, classNumStates;
    double epsilon = 1e-5;
};
class Paths {
public:
    static std::string datasets()
    {
        std::string result = { folding_data_path.begin(), folding_data_path.end() };
        return result + "/";
    }
    static std::string csv()
    {
        return datasets() + "../csv/";
    }
};
class CSVFiles {
public:
    static void write_csv(std::string fname, std::vector<int> indices)
    {
        std::ofstream file(Paths::csv() + fname);
        for (auto i = 0; i < indices.size(); ++i) {
            file << indices[i] << std::endl;
        }
        file.close();
    }
    static std::vector<int> read_csv(std::string fname)
    {
        std::ifstream file(Paths::csv() + fname);
        std::vector<int> indices;
        std::string line;
        while (std::getline(file, line)) {
            indices.push_back(std::stoi(line));
        }
        file.close();
        return indices;
    }
};
#endif //TEST_UTILS_H
