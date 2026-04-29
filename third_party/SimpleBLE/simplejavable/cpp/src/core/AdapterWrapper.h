#pragma once
#include <simpleble/SimpleBLE.h>
#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"
#include "org/simplejavable/AdapterCallback.h"

class AdapterWrapper {
  public:
    explicit AdapterWrapper(const SimpleBLE::Adapter& adapter);
    ~AdapterWrapper();

    // Accessor to the underlying adapter
    SimpleBLE::Adapter& get();
    const SimpleBLE::Adapter& get() const;

    void setCallback(Org::SimpleJavaBLE::AdapterCallback& callback);

    size_t getHash();

  private:
    SimpleBLE::Adapter _adapter;
    Org::SimpleJavaBLE::AdapterCallback _callback;
};