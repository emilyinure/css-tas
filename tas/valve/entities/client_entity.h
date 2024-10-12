#include "library/math.h"

#include "client_networkable.h"
#include "client_unknown.h"
#include "client_renderable.h"
#include "client_thinkable.h"

class client_entity_t : public client_unknown_t,
                        public client_renderable_t,
                        public client_networkable_t,
                        public client_thinkable_t {
public:
  virtual void             release(void)              = 0;
  virtual const vector3_t& get_abs_origin(void) const = 0;
  virtual const vector3_t& get_abs_angles(void) const = 0;
  virtual void*            get_mouth(void)            = 0;
  virtual bool             get_sound_spatialization() = 0;
};
