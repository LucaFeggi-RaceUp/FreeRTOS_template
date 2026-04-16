#pragma once

#include <cstdint>

#include "stm32h5xx_hal.h"

namespace ru::driver {
struct opaque_can {
  constexpr opaque_can() noexcept = default;
  constexpr opaque_can(FDCAN_GlobalTypeDef* const p_instance, GPIO_TypeDef* const p_port_rx,
                       const uint16_t rx_pin, GPIO_TypeDef* const p_port_tx,
                       const uint16_t tx_pin, const uint32_t alternate) noexcept
      : m_p_instance(p_instance),
        m_p_port_rx(p_port_rx),
        m_rx_pin(rx_pin),
        m_p_port_tx(p_port_tx),
        m_tx_pin(tx_pin),
        m_alternate(alternate) {}

  FDCAN_GlobalTypeDef* m_p_instance{nullptr};
  GPIO_TypeDef* m_p_port_rx{nullptr};
  uint16_t m_rx_pin{0U};
  GPIO_TypeDef* m_p_port_tx{nullptr};
  uint16_t m_tx_pin{0U};
  uint32_t m_alternate{0U};
};
}  // namespace ru::driver
