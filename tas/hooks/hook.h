#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>

class hook_t {
  uintptr_t                    base;
  void**                       m_old_vmt;
  std::unique_ptr<uintptr_t[]> new_vmt;
  size_t                       size;
  bool                         rtti;

  __forceinline size_t count_methods() const {
    auto i{0u};

    while (m_old_vmt[i] != nullptr)
      ++i;

    return i;
  }

public:
  bool initialize(uintptr_t base, bool rtti = true) {
    // save base class.
    this->base = base;

    // get ptr to old VMT.
    m_old_vmt = *reinterpret_cast<void***>(base);
    if (!m_old_vmt)
      return false;

    // count number of methods in old VMT.
    size = count_methods();
    if (!size)
      return false;

    // allocate new VMT.
    new_vmt = std::make_unique<uintptr_t[]>(rtti ? size + 1 : size);

    this->rtti = rtti;
    if (!new_vmt)
      return false;
    // get raw memory ptr.
    const auto vmt = reinterpret_cast<uintptr_t>(new_vmt.get());

    if (rtti) {
      // copy VMT, starting from RTTI.
      std::memcpy(reinterpret_cast<uintptr_t*>(vmt), m_old_vmt - 1,
                  (size + 1) * sizeof(uintptr_t));

      // VMTs are ( usually ) stored in the .data section we should be able to just overwrite
      // it, so let's do that here. also, since we've copied RTTI ptr then point the new table
      // at index 1 ( index 0 contains RTTI ptr ).
      *reinterpret_cast<uintptr_t*>(base) = (vmt + sizeof(uintptr_t));

      // we've sucesfully copied the RTTI ptr.
    }

    else {
      // copy vmt.
      std::memcpy(reinterpret_cast<uintptr_t*>(vmt), m_old_vmt, size * sizeof(uintptr_t));

      // since VMTs are ( usually ) stored in the .data section we should be able to just
      // overwrite it, so let's do that here.
      *reinterpret_cast<uintptr_t*>(base) = vmt;
    }
    return true;
  }

  void undo_hooks() {
    new_vmt.reset();

    if (base)
      *(uintptr_t*)base = (uintptr_t)m_old_vmt;

    base      = uintptr_t{};
    m_old_vmt = nullptr;
    size      = 0;
    rtti      = false;
  }

  void setup_hook(size_t function_index, uintptr_t func) {
    const auto vmt_index{rtti ? function_index + 1 : function_index};

    // sanity check some stuff first.
    if (!m_old_vmt || !new_vmt || vmt_index > size)
      return;

    // redirect.
    new_vmt[vmt_index] = func;
  }

  template <typename T> T get_virtual_function(size_t function_index) {
    return reinterpret_cast<T>(m_old_vmt[function_index]);
  }
};
