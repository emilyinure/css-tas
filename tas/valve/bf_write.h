#pragma once

#pragma pack(push, 1)

class bf_write_t {
public:
  bf_write_t() noexcept
      : m_pData{}, m_nDataBytes{}, m_nDataBits{-1}, m_iCurBit{}, m_bOverflow{},
        m_bAssertOnOverflow{true}, m_pDebugName{} {}

  const auto& IsOverflowed() const noexcept { return m_bOverflow; }

  std::uint32_t* m_pData;
  std::int32_t   m_nDataBytes;
  std::int32_t   m_nDataBits;
  std::int32_t   m_iCurBit;
  bool           m_bOverflow;
  bool           m_bAssertOnOverflow;

private:
  std::uint8_t m_pad_00[2];

public:
  const char* m_pDebugName;
};

#pragma pack(pop)