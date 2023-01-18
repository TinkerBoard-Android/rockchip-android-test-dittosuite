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

#pragma once

#include <string>

#include <ditto/time_sampler.h>

namespace dittosuite {

enum ReadWriteType { kSequential, kRandom };

class Instruction {
 public:
  explicit Instruction(const std::string& name, int repeat);
  virtual ~Instruction() = default;

  virtual void SetUp();
  void Run();
  virtual void TearDown();

  static void SetAbsolutePathKey(int absolute_path_key);
  static int GetAbsolutePathKey();

 protected:
  static int absolute_path_key_;
  std::string name_;
  int repeat_;
  TimeSampler time_sampler_;

 private:
  virtual void RunSingle() = 0;
};

} // namespace dittosuite
