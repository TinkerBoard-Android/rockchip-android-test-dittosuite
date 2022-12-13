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

#include <unistd.h>

#include <gtest/gtest.h>

#include <ditto/create_file.h>
#include <ditto/delete_file.h>

TEST(DeleteFileTest, DeleteFileTestRun) {
  int repeat = 1;
  std::string file = "/data/local/tmp/newfile.txt";

  dittosuite::CreateFile create_file_instruction(repeat, file);
  create_file_instruction.Run();
  ASSERT_EQ(access(file.c_str(), F_OK), 0);

  dittosuite::DeleteFile delete_file_instruction(repeat, file);
  delete_file_instruction.Run();
  ASSERT_EQ(access(file.c_str(), F_OK), -1);
}