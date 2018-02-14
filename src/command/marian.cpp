/* All or part of this file was contributed by Intel under license:
 *   Copyright (C) 2017-2018 Intel Corporation
 *   SPDX-License-Identifier: MIT
 */

#include "marian.h"

#include "models/model_task.h"
#include "training/graph_group.h"

int main(int argc, char** argv) {
  using namespace marian;

  auto options = New<Config>(argc, argv);
  auto devices = options->get<std::vector<size_t>>("devices");

  if(devices.size() > 1)
    WrapModelType<Train, AsyncGraphGroup>(options)->run();
  else
    WrapModelType<Train, SingletonGraph>(options)->run();

  return 0;
}
