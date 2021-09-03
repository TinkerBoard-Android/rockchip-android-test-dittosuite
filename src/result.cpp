// Copyright (C) 2021 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <ditto/result.h>
#include <ditto/statistics.h>
#include <ditto/timespec_utils.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <string>

const int kTimeSampleDisplayWidth = 11;  // this width is used displaying a time sample value
const int kTableWidth = 164;  // table width; can be adjusted in case of longer instruction paths
const char* kTableDivider = " | ";   // table character divider
const int kMaxHistogramHeight = 20;  // used for normalizing the histogram (represents the
                                     //  maximum height of the histogram)
const int kMaxHistogramWidth = 50;   // used for normalizing the histogram (represents the
                                     // maximum width of the histogram)
const char kCsvDelimiter = ',';      // delimiter used for .csv files
static int bin_size;                 // bin size corresponding to the normalization
                                     // of the Oy axis of the histograms

namespace dittosuite {

Result::Result(const std::string& name) : name_(name) {}

void Result::AddMeasurement(const std::string& name, const std::vector<int64_t>& samples) {
  samples_[name] = samples;
  AnalyseMeasurement(name);
}

void Result::AddSubResult(std::unique_ptr<Result> result) {
  sub_results_.push_back(std::move(result));
}

// analyse the measurement with the given name, and store
// the results in the statistics_ map
void Result::AnalyseMeasurement(const std::string& name) {
  statistics_[name].min = StatisticsGetMin(samples_[name]);
  statistics_[name].max = StatisticsGetMax(samples_[name]);
  statistics_[name].mean = StatisticsGetMean(samples_[name]);
  statistics_[name].median = StatisticsGetMedian(samples_[name]);
  statistics_[name].sd = StatisticsGetSd(samples_[name]);
}

std::string Result::ComputeNextInstructionPath(const std::string& instruction_path) {
  return instruction_path + (instruction_path != "" ? "/" : "") + name_;
}

void Result::PrintMeasurement(const std::string& name) {
  int dividing_factor = 1;
  std::string unit_name = "";

  if (name == "duration") {
    time_unit_ = GetTimeUnit(statistics_[name].min);
    dividing_factor = time_unit_.dividing_factor;
    unit_name = time_unit_.name;
  } else if (name == "bandwidth") {
    unit_name = " KB/s";
  }

  std::cout << name << ":" << std::endl;
  std::cout << "Min: " << statistics_[name].min / dividing_factor << unit_name << std::endl;
  std::cout << "Max: " << statistics_[name].max / dividing_factor << unit_name << std::endl;
  std::cout << "Mean: " << statistics_[name].mean / dividing_factor << unit_name << std::endl;
  std::cout << "Median: " << statistics_[name].median / dividing_factor << unit_name << std::endl;
  std::cout << "Sd: " << statistics_[name].sd / dividing_factor << std::endl;

  std::cout << std::endl;
}

void Result::Print(const std::string& instruction_path) {
  std::string next_instruction_path = ComputeNextInstructionPath(instruction_path);
  std::cout << next_instruction_path << std::endl;

  for (const auto& s : samples_) PrintMeasurement(s.first);

  for (const auto& sub_result : sub_results_) {
    sub_result->Print(next_instruction_path);
  }
}

void PrintTableBorder() {
  std::cout << std::endl;
  for (int i = 0; i < kTableWidth; i++) std::cout << "-";
  std::cout << std::endl;
}

void PrintStatisticsTableHeader() {
  std::cout << "\x1b[1m";  // beginning of bold
  PrintTableBorder();
  std::cout << "| ";  // beginning of table row
  std::cout << std::setw(70) << std::left << "Instruction name";
  std::cout << kTableDivider;
  std::cout << std::setw(15) << std::left << " Min";
  std::cout << kTableDivider;
  std::cout << std::setw(15) << std::left << " Max";
  std::cout << kTableDivider;
  std::cout << std::setw(15) << std::left << " Mean";
  std::cout << kTableDivider;
  std::cout << std::setw(15) << std::left << " Median";
  std::cout << kTableDivider;
  std::cout << std::setw(15) << std::left << " SD";
  std::cout << kTableDivider;
  PrintTableBorder();
  std::cout << "\x1b[0m";  // ending of bold
}

void PrintMeasurementInTable(const int64_t& measurement, const std::string& measurement_name) {
  if (measurement_name == "duration") {
    std::cout << std::setw(13) << measurement << "ns";
  } else if (measurement_name == "bandwidth") {
    std::cout << std::setw(11) << measurement << "KB/s";
  }
}

// Recursive function to print one row at a time
// of statistics table content (the instruction path, min, max and mean).
void Result::PrintStatisticsTableContent(const std::string& instruction_path,
                                         const std::string& measurement_name) {
  std::string next_instruction_path = ComputeNextInstructionPath(instruction_path);
  int subinstruction_level =
      std::count(next_instruction_path.begin(), next_instruction_path.end(), '/');
  // If the instruction path name contains too many subinstrions,
  // print only the last 2 preceded by "../".
  if (subinstruction_level > 2) {
    std::size_t first_truncate_pos = next_instruction_path.find('/');
    next_instruction_path = ".." + next_instruction_path.substr(first_truncate_pos);
  }

  // Print table row
  if (samples_.find(measurement_name) != samples_.end()) {
    std::cout << "| ";  // started new row
    std::cout << std::setw(70) << std::left << next_instruction_path << std::right;
    std::cout << kTableDivider;
    PrintMeasurementInTable(statistics_[measurement_name].min, measurement_name);
    std::cout << kTableDivider;
    PrintMeasurementInTable(statistics_[measurement_name].max, measurement_name);
    std::cout << kTableDivider;
    PrintMeasurementInTable(statistics_[measurement_name].mean, measurement_name);
    std::cout << kTableDivider;
    PrintMeasurementInTable(statistics_[measurement_name].median, measurement_name);
    std::cout << kTableDivider;
    std::cout << std::setw(15)
              << statistics_[measurement_name].sd;  // SD is always printed without measurement unit
    std::cout << kTableDivider;                     // ended current row
    PrintTableBorder();
  }

  for (const auto& sub_result : sub_results_) {
    sub_result->PrintStatisticsTableContent(next_instruction_path, measurement_name);
  }
}

std::set<std::string> Result::GetMeasurementsNames() {
  std::set<std::string> names;
  for (const auto& it : samples_) names.insert(it.first);
  for (const auto& sub_result : sub_results_) {
    for (const auto& sub_name : sub_result->GetMeasurementsNames()) names.insert(sub_name);
  }
  return names;
}

void Result::PrintStatisticsTables() {
  std::set<std::string> measurement_names = GetMeasurementsNames();
  for (const auto& s : measurement_names) {
    std::cout << std::endl << s << " statistics:";
    PrintStatisticsTableHeader();
    PrintStatisticsTableContent("", s);
    std::cout << std::endl;
  }
}

void Result::PrintHistogramHeader(const std::string& measurement_name) {
  if (measurement_name == "duration") {
    std::cout.width(kTimeSampleDisplayWidth - 3);
    std::cout << "Time(" << time_unit_.name << ") |";
    std::cout << " Normalized number of time samples";
    std::cout << std::endl;
  }
  for (int i = 0; i <= kMaxHistogramWidth + 15; i++) std::cout << "-";
  std::cout << std::endl;
}

// makes (normalized) histogram from vector
void Result::MakeHistogramFromVector(const std::vector<int>& freq_vector, const int& min_value) {
  int sum = 0;
  int max_frequency = *std::max_element(freq_vector.begin(), freq_vector.end());
  for (unsigned int i = 0; i < freq_vector.size(); i++) {
    std::cout.width(kTimeSampleDisplayWidth);
    std::cout << min_value + bin_size * i << kTableDivider;
    for (int j = 0; j < freq_vector[i] * kMaxHistogramWidth / max_frequency; j++) std::cout << "x";
    std::cout << " {" << freq_vector[i] << "}";
    sum += freq_vector[i];
    std::cout << std::endl;
  }

  std::cout << "Total samples: { " << sum << " }" << std::endl;
  std::cout << std::endl;
}

// makes and returns the normalized frequency vector
std::vector<int> Result::ComputeNormalizedFrequencyVector(const std::string& measurement_name) {
  int64_t min_value = statistics_[measurement_name].min;
  if (measurement_name == "duration") min_value /= time_unit_.dividing_factor;

  std::vector<int> freq_vector(kMaxHistogramHeight, 0);
  for (const auto& sample : samples_[measurement_name]) {
    int64_t sample_copy =
        measurement_name == "duration" ? (sample / time_unit_.dividing_factor) : sample;
    int64_t bin = (sample_copy - min_value) / bin_size;

    freq_vector[bin]++;
  }
  return freq_vector;
}

Result::TimeUnit Result::GetTimeUnit(const int64_t& min_value) {
  TimeUnit result;
  if (min_value <= 1e7) {
    // time unit in nanoseconds
    result.dividing_factor = 1;
    result.name = "ns";
  } else if (min_value <= 1e10) {
    // time unit in microseconds
    result.dividing_factor = 1e3;
    result.name = "us";
  } else if (min_value <= 1e13) {
    // time unit in milliseconds
    result.dividing_factor = 1e6;
    result.name = "ms";
  } else {
    // time unit in seconds
    result.dividing_factor = 1e9;
    result.name = "s";
  }
  return result;
}

void Result::PrintHistograms(const std::string& instruction_path) {
  for (const auto& sample : samples_) {
    std::string next_instruction_path = ComputeNextInstructionPath(instruction_path);

    std::cout << std::endl;
    std::cout << "\x1b[1m";  // beginning of bold
    std::cout << "Instruction path: " << next_instruction_path;
    std::cout << "\x1b[0m" << std::endl;  // ending of bold
    std::cout << std::endl;

    int64_t min_value = statistics_[sample.first].min;
    int64_t max_value = statistics_[sample.first].max;
    if (sample.first == "duration") {
      time_unit_ = GetTimeUnit(statistics_[sample.first].min);
      min_value /= time_unit_.dividing_factor;
      max_value /= time_unit_.dividing_factor;
    }
    bin_size = (max_value - min_value) / kMaxHistogramHeight + 1;

    std::vector<int> freq_vector = ComputeNormalizedFrequencyVector(sample.first);
    PrintHistogramHeader(sample.first);
    MakeHistogramFromVector(freq_vector, min_value);
    std::cout << std::endl << std::endl;

    for (const auto& sub_result : sub_results_) {
      sub_result->PrintHistograms(next_instruction_path);
    }
  }
}

// Print statistic measurement with given name in .csv
void Result::PrintMeasurementStatisticInCsv(std::ostream& csv_stream, const std::string& name) {
  csv_stream << statistics_[name].min << kCsvDelimiter;
  csv_stream << statistics_[name].max << kCsvDelimiter;
  csv_stream << statistics_[name].mean << kCsvDelimiter;
  csv_stream << statistics_[name].median << kCsvDelimiter;
  csv_stream << statistics_[name].sd;
  csv_stream << std::endl;  // ending of row
}

// Recursive function to print one row at a time using the .csv stream given as a parameter
// of statistics table content (the instruction path, min, max, mean and SD).
void Result::PrintStatisticInCsv(std::ostream& csv_stream, const std::string& instruction_path,
                                 const std::string& measurement_name) {
  std::string next_instruction_path = ComputeNextInstructionPath(instruction_path);
  csv_stream << next_instruction_path << kCsvDelimiter;

  PrintMeasurementStatisticInCsv(csv_stream, measurement_name);

  for (const auto& sub_result : sub_results_) {
    sub_result->PrintStatisticInCsv(csv_stream, next_instruction_path, measurement_name);
  }
}

void Result::MakeStatisticsCsv() {
  std::ostream csv_stream(std::cout.rdbuf());

  for (const auto& s : samples_) {
    csv_stream << "Instruction path" << kCsvDelimiter;
    csv_stream << "Min" << kCsvDelimiter;
    csv_stream << "Max" << kCsvDelimiter;
    csv_stream << "Mean" << kCsvDelimiter;
    csv_stream << "Median" << kCsvDelimiter;
    csv_stream << "SD" << std::endl;

    PrintStatisticInCsv(csv_stream, "", s.first);
  }
}

}  // namespace dittosuite
