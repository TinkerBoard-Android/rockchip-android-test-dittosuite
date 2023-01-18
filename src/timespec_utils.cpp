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

#include <ditto/logger.h>
#include <ditto/time_sampler.h>

int64_t TimespecToNs(const timespec& t) {
  return t.tv_sec * 1e9 + t.tv_nsec;
}

timespec NsToTimespec(const int64_t& t) {
  timespec result;
  result.tv_sec = t / 1e9;
  result.tv_nsec = t % static_cast<int64_t>(1e9);
  return result;
}

bool operator==(const timespec& t1, const timespec& t2) {
  return t1.tv_sec == t2.tv_sec && t1.tv_nsec == t2.tv_nsec;
}

bool operator<(const timespec& t1, const timespec& t2) {
  return ((t1.tv_sec < t2.tv_sec) || (t1.tv_sec == t2.tv_sec && t1.tv_nsec < t2.tv_nsec));
}

bool operator<=(const timespec& t1, const timespec& t2) {
  return t1 == t2 || t1 < t2;
}

bool operator>(const timespec& t1, const timespec& t2) {
  return !(t1 <= t2);
}

bool operator>=(const timespec& t1, const timespec& t2) {
  return t1 == t2 || t1 > t2;
}

// return the value of t1 - t2, if t1 >= t2
// return {0, 0} and display an error if t1 < t2
timespec operator-(const timespec& t1, const timespec& t2) {
  timespec result = {0, 0};
  if (t1 >= t2) {
    result.tv_sec = t1.tv_sec - t2.tv_sec;
    if (t1.tv_nsec < t2.tv_nsec) {
      result.tv_sec--;
      result.tv_nsec = 1e9 - t2.tv_nsec + t1.tv_nsec;
    } else {
      result.tv_nsec = t1.tv_nsec - t2.tv_nsec;
    }
    return result;
  }
  LOGE("At timer, end time is smaller than start time");
  return result;
}

timespec operator+(const timespec& t1, const timespec& t2) {
  timespec result = {0, 0};
  result.tv_sec = t1.tv_sec + t2.tv_sec;
  if (t1.tv_nsec + t2.tv_nsec >= 1e9) {
    result.tv_sec++;
    result.tv_nsec = t1.tv_nsec + t2.tv_nsec - 1e9;
  } else {
    result.tv_nsec = t1.tv_nsec + t2.tv_nsec;
  }
  return result;
}
