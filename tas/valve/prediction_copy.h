#pragma once

#include "datamap.h"
#include "../core/offsets/offsets.h"

enum {
  PC_EVERYTHING = 0,
  PC_NON_NETWORKED_ONLY,
  PC_NETWORKED_ONLY,
};

#define PC_DATA_PACKED true
#define PC_DATA_NORMAL false

typedef void (*FN_FIELD_COMPARE)(const char* classname, const char* fieldname,
                                 const char* fieldtype, bool networked, bool noterrorchecked,
                                 bool differs, bool withintolerance, const char* value);

class prediction_copy_t {

  typedef enum {
    DIFFERS = 0,
    IDENTICAL,
    WITHINTOLERANCE,
  } difftype_t;
  int         type;
  void*       dest;
  void const* src;
  int         dest_offset_index;
  int         src_offset_index;

  bool        error_check;
  bool        report_errors;
  bool        describe_fields;
  void*       current_field;
  char const* current_class_name;
  datamap_t*  current_map;
  bool        should_report;
  bool        should_describe;
  int         error_count;
  bool        perform_copy;

  FN_FIELD_COMPARE field_compare_func;

  void*       watch_field;
  char const* operation;
  char        idk[100];

public:
  int transfer_data(const char* operation, int entindex, datamap_t* dmap) {
    typedef int(__thiscall * sub_1017D4D0)(void*, const char*, int, datamap_t*);
    return ((sub_1017D4D0)offsets.functions.transfer_data)(this, operation, entindex, dmap);
  }

  prediction_copy_t(int type, void* dest, bool dest_packed, void const* src, bool src_packed,
                    bool counterrors = false, bool reporterrors = false,
                    bool performcopy = true, bool describefields = false,
                    FN_FIELD_COMPARE func = NULL) {

    this->type              = type;
    this->dest              = dest;
    this->src               = src;
    this->dest_offset_index = dest_packed ? TD_OFFSET_PACKED : TD_OFFSET_NORMAL;
    this->src_offset_index  = src_packed ? TD_OFFSET_PACKED : TD_OFFSET_NORMAL;
    this->error_check       = counterrors;
    this->report_errors     = reporterrors;
    this->perform_copy      = performcopy;
    this->describe_fields   = describefields;

    this->current_field      = NULL;
    this->current_map        = NULL;
    this->current_class_name = NULL;
    this->should_report      = false;
    this->should_describe    = false;
    this->error_count        = 0;

    this->field_compare_func = func;
  }
};
