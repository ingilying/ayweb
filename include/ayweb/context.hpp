#pragma once

#include "ayweb/export.hpp"
#include "ayweb/router.hpp"
namespace ayweb
{

  AYWEB_EXPORT struct GlobalContext
  {
    explicit GlobalContext() = default;
    ~GlobalContext() = default;
    GlobalContext(GlobalContext&&) = default;
    GlobalContext& operator=(GlobalContext&&) = default;
    GlobalContext(const GlobalContext&) = delete;
    GlobalContext& operator=(const GlobalContext&) = delete;

    // Data
    Router router;
  };
}  // namespace ayweb
