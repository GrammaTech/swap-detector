//===- main.cpp -------------------------------------------------*- C++ -*-===//
//
//  Copyright (C) 2020 GrammaTech, Inc.
//
//  This code is licensed under the MIT license. See the LICENSE file in the
//  project root for license terms.
//
// This material is based on research sponsored by the Department of Homeland
// Security (DHS) Office of Procurement Operations, S&T acquisition Division via
// contract number 70RSAT19C00000056. The views and conclusions contained herein
// are those of the authors and should not be interpreted as necessarily
// representing the official policies or endorsements, either expressed or
// implied, of the Department of Homeland Security.
//
//===----------------------------------------------------------------------===//
#include <gmock/gmock.h>
int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
