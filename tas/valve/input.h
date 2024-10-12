#pragma once

#include "usercmd.h"

#include <cstdint>

#pragma pack(push, 1)

constexpr std::int32_t MULTIPLAYER_BACKUP = 90;

class cinput_t {
public:
  virtual ~cinput_t()                                          = 0;
  virtual void       pad_01()                                  = 0;
  virtual void       pad_02()                                  = 0;
  virtual void       pad_03()                                  = 0;
  virtual void       pad_04()                                  = 0;
  virtual void       pad_05()                                  = 0;
  virtual void       pad_06()                                  = 0;
  virtual void       pad_07()                                  = 0;
  virtual usercmd_t* get_usercmd(std::int32_t sequence_number) = 0;

private:
  uint8_t m_pad_00[192];

public:
  usercmd_t* cmds;
  // CVerifiedUserCmd* m_pVerifiedCommands;
};

static_assert(offsetof(cinput_t, cmds) == 0xC4);

#pragma pack(pop)