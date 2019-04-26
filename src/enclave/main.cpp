// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#include "../ds/spinlock.h"
#include "enclave.h"

#include <ccf_t.h>

// the central enclave object
static SpinLock create_lock;
static enclave::Enclave* e;
static uint8_t* reserved_memory;

extern "C"
{
  bool enclave_create_node(
    void* config,
    uint8_t* node_cert,
    size_t node_cert_size,
    size_t* node_cert_len,
    uint8_t* quote,
    size_t quote_size,
    size_t* quote_len,
    bool recover)
  {
    std::lock_guard<SpinLock> guard(create_lock);

    if (e != nullptr)
      return false;

    EnclaveConfig* ec = (EnclaveConfig*)config;

#ifdef DEBUG_CONFIG
    reserved_memory = new uint8_t[ec->debug_config.memory_reserve_startup];
#endif

    e = new enclave::Enclave(ec);

    auto ret = e->create_node(
      node_cert,
      node_cert_size,
      node_cert_len,
      quote,
      quote_size,
      quote_len,
      recover);

    return ret;
  }

  bool enclave_run()
  {
    if (e != nullptr)
      return e->run();
    else
      return false;
  }

  bool enclave_tick(size_t now, size_t elapsed)
  {
    std::chrono::milliseconds elapsed_{elapsed};
    std::chrono::system_clock::time_point now_{
      std::chrono::system_clock::duration{now}};
    if (e != nullptr)
      return e->tick(now_, elapsed_);
    else
      return false;
  }
}
