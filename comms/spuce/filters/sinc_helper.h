#pragma once
#include <vector>
//! Band pass: sample rate, lower frequency, upper frequency, window
std::vector<double> sincBPF(const size_t numTaps, const double Fl, const double Fu);
//! Band stop: sample rate, lower frequency, upper frequency, window
std::vector<double> sincBSF(const size_t numTaps, const double Fl, const double Fu);
