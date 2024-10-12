#pragma once

class base_entity_t;

enum class trace_type_t {
  TRACE_EVERYTHING = 0,
  TRACE_WORLD_ONLY,
  TRACE_ENTITIES_ONLY,
  TRACE_EVERYTHING_FILTER_PROPS
};

class i_trace_filter_t {
  virtual bool         should_hit_entity(base_entity_t* ent, int contents_mask) = 0;
  virtual trace_type_t get_type() const                                         = 0;
};

class trace_filter_t : public i_trace_filter_t {
  virtual bool should_hit_entity(base_entity_t* ent, int contents_mask) {
    return ent != skip_entity;
  }

  virtual trace_type_t get_type() const { return trace_type_t::TRACE_EVERYTHING; }

public:
  trace_filter_t(base_entity_t* skip_entity_) : skip_entity(skip_entity_) {}

  base_entity_t* skip_entity;
};

class trace_filter_simple_t : public i_trace_filter_t {
  typedef bool (*should_hit_t)(base_entity_t* handle_entity, int contents_mask);

  base_entity_t* pass_ent;
  int            collision_group;
  should_hit_t   should_hit_extra_func;

public:
  trace_filter_simple_t(base_entity_t* pass_entity, int collision_group,
                        should_hit_t should_hit_func = 0) {
    pass_ent                    = pass_entity;
    this->collision_group       = collision_group;
    this->should_hit_extra_func = should_hit_func;
  }

  virtual trace_type_t get_type() const { return trace_type_t::TRACE_EVERYTHING; }

  virtual bool should_hit_entity(base_entity_t* handle_entity, int contents_mask);

  virtual void set_pass_entity(base_entity_t* handle_entity) { pass_ent = handle_entity; }

  virtual void set_collision_group(int collision_group) {
    this->collision_group = collision_group;
  }
};

class trace_filter_ignore_players_t : public trace_filter_simple_t {
public:
  trace_filter_ignore_players_t(base_entity_t* passentity, int collision_group)
      : trace_filter_simple_t(passentity, collision_group) {}

  virtual bool should_hit_entity(base_entity_t* server_entity, int contents_mask);
};

class trace_filter_ignore_teammates_t : public trace_filter_simple_t {
public:
  trace_filter_ignore_teammates_t(base_entity_t* passentity, int collision_group,
                                  int ignore_team)
      : trace_filter_simple_t(passentity, collision_group), ignore_team(ignore_team) {}

  virtual bool should_hit_entity(base_entity_t* server_entity, int contents_mask);

  int ignore_team;
};
