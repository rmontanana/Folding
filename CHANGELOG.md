# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.1] 2024-04-03

### Added

- Possibility to check the indices of the training and testing sets in the K-Fold and Stratified K-Fold partitioning. Now disabled due to apple's clang compiler mt19937 implementation.

### Changed

- Random number generator is changed to mt19937 to improve the robustness of the models generated.

## [1.0.0] 2024-01-08

### Added

- K-Fold partitioning for training and testing
- Stratified K-Fold partitioning for training and testing
