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

#include <ditto/close_file.h>

#include <unistd.h>

#include <cstdlib>

#include <ditto/logger.h>
#include <ditto/shared_variables.h>

namespace dittosuite {

CloseFile::CloseFile(int repeat) : Instruction(repeat), input_fd_key_(-1) {}

void CloseFile::SetUp() {}

void CloseFile::RunSingle() {
  int fd = std::get<int>(SharedVariables::Get(input_fd_key_));

  if (close(fd) != 0) {
    LOGE("Error while closing the file");
    exit(EXIT_FAILURE);
  }
}

void CloseFile::TearDown() {}

int CloseFile::GetInputFdKey() {
  return input_fd_key_;
}

void CloseFile::SetInputFdKey(int input_fd_key) {
  input_fd_key_ = input_fd_key;
}

}  // namespace dittosuite
