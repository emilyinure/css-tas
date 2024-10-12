#include "library/utils.h"

class model_t;
class studio_header_t;

class model_info_t {
public:
  model_t* GetModel(int index) {
    typedef model_t*(__thiscall * GetModelFn)(void*, int);
    return utils::get_virtual_function<GetModelFn>(this, 1)(this, index);
  }

  int GetModelIndex(const char* name) {
    typedef int(__thiscall * GetModelIndexFn)(void*, const char*);
    return utils::get_virtual_function<GetModelIndexFn>(this, 2)(this, name);
  }

  const char* GetModelName(const model_t* model) {
    typedef const char*(__thiscall * GetModelNameFn)(void*, const model_t*);
    return utils::get_virtual_function<GetModelNameFn>(this, 3)(this, model);
  }

  studio_header_t* GetStudiomodel(const model_t* mod) {
    typedef studio_header_t*(__thiscall * GetStudiomodelFn)(void*, const model_t*);
    return utils::get_virtual_function<GetStudiomodelFn>(this, 28)(this, mod);
  }
};
