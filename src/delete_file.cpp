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

#include <ditto/delete_file.h>

#include <unistd.h>

#include <fstream>

#include <ditto/shared_variables.h>
#include <ditto/logger.h>

namespace dittosuite {

DeleteFile::DeleteFile(int repeat, const std::string& path_name)
    : Instruction(kName, repeat), path_name_(GetAbsolutePath() + path_name), input_key_(-1) {}

DeleteFile::DeleteFile(int repeat, int input_key)
    : Instruction(kName, repeat), input_key_(input_key) {}

void DeleteFile::SetUpSingle() {
  if (input_key_ != -1) {
    path_name_ = GetAbsolutePath() + std::get<std::string>(SharedVariables::Get(input_key_));
  }
  Instruction::SetUpSingle();
}

void DeleteFile::RunSingle() {
  if (unlink(path_name_.c_str()) == -1) {
    LOGF("Error while calling unlink()");
  }
}

}  // namespace dittosuite
