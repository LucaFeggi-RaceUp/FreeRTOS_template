#include <array>

#include "adc.hpp"
#include "mapping.hpp"
#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
namespace {
constexpr std::size_t adc_index(const AdcId id) noexcept {
  return static_cast<std::size_t>(id);
}

constexpr std::size_t k_adc_count = static_cast<std::size_t>(AdcId::COUNT);
constexpr std::size_t k_adc_sample_modulus = k_adc_count == 0U ? 1U : k_adc_count;
constexpr std::size_t k_dma_frames = 16U;
constexpr std::size_t k_dma_buffer_length = k_adc_count * k_dma_frames;
constexpr std::size_t k_dma_half_buffer_length = k_dma_buffer_length / 2U;

struct adc_dma_channel_config {
  ADC_TypeDef* p_instance;
  GPIO_TypeDef* p_port;
  uint16_t pin;
  uint32_t channel;
};

ADC_HandleTypeDef& adc_handle(const AdcId id) noexcept {
  static std::array<ADC_HandleTypeDef, k_adc_count> handles{};
  static ADC_HandleTypeDef dummy_handle{};
  if constexpr (k_adc_count == 0U) {
    (void)id;
    return dummy_handle;
  }

  const auto index = adc_index(id);
  return index < handles.size() ? handles[index] : handles.front();
}

constexpr std::array<adc_dma_channel_config, k_adc_count> k_adc_dma_configs = {
#define RU_STM32H5XX_ADC_DMA_ENTRY(name, instance, port, pin, channel) \
  adc_dma_channel_config{instance, port, pin, channel},
    RU_STM32H5XX_ADC_MAP(RU_STM32H5XX_ADC_DMA_ENTRY)
#undef RU_STM32H5XX_ADC_DMA_ENTRY
};

constexpr opaque_adc make_opaque(const AdcId id) noexcept {
  switch (id) {
#define RU_STM32H5XX_ADC_CASE(name, instance, port, pin, channel)                        \
    case AdcId::name:                                                                    \
      return opaque_adc{instance, port, pin, channel};
    RU_STM32H5XX_ADC_MAP(RU_STM32H5XX_ADC_CASE)
#undef RU_STM32H5XX_ADC_CASE
    default:
      return opaque_adc{};
  }
}

struct dma_adc_backend {
  ADC_HandleTypeDef adc_handle{};
  DMA_NodeTypeDef dma_node{};
  DMA_QListTypeDef dma_queue{};
  DMA_HandleTypeDef dma_channel_handle{};
  std::array<uint16_t, k_dma_buffer_length> dma_buffer{};
  std::array<uint64_t, k_adc_count> sums{};
  std::array<uint32_t, k_adc_count> sample_counts{};
  std::size_t processed_samples_in_cycle{0U};
  bool init_attempted{false};
  bool running{false};
};

dma_adc_backend& shared_dma_backend() noexcept {
  static dma_adc_backend backend{};
  return backend;
}

void init_dma_pins() noexcept {
  for (const auto& config : k_adc_dma_configs) {
    init_analog_pin(config.p_port, config.pin);
  }
}

void accumulate_dma_samples(dma_adc_backend& backend, const std::size_t begin_sample,
                            const std::size_t end_sample) noexcept {
  if constexpr (k_adc_count == 0U) {
    (void)backend;
    (void)begin_sample;
    (void)end_sample;
    return;
  }

  const std::size_t begin = std::min(begin_sample, k_dma_buffer_length);
  const std::size_t end = std::min(end_sample, k_dma_buffer_length);

  for (std::size_t sample = begin; sample < end; ++sample) {
    const auto channel = sample % k_adc_sample_modulus;
    backend.sums[channel] += backend.dma_buffer[sample];
    backend.sample_counts[channel] += 1U;
  }
}

void accumulate_dma_until(dma_adc_backend& backend, const std::size_t boundary) noexcept {
  if (boundary <= backend.processed_samples_in_cycle) {
    return;
  }

  accumulate_dma_samples(backend, backend.processed_samples_in_cycle, boundary);
  backend.processed_samples_in_cycle = boundary;
}

bool start_dma_backend() noexcept {
  if constexpr (k_adc_count == 0U) {
    return false;
  }

  auto& backend = shared_dma_backend();
  if (backend.init_attempted) {
    return backend.running;
  }

  backend.init_attempted = true;

  __HAL_RCC_ADC_CLK_ENABLE();
  __HAL_RCC_GPDMA1_CLK_ENABLE();
  init_dma_pins();

  DMA_NodeConfTypeDef node_config{};
  node_config.NodeType = DMA_GPDMA_LINEAR_NODE;
  node_config.Init.Request = GPDMA1_REQUEST_ADC1;
  node_config.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
  node_config.Init.Direction = DMA_PERIPH_TO_MEMORY;
  node_config.Init.SrcInc = DMA_SINC_FIXED;
  node_config.Init.DestInc = DMA_DINC_INCREMENTED;
  node_config.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_HALFWORD;
  node_config.Init.DestDataWidth = DMA_DEST_DATAWIDTH_HALFWORD;
  node_config.Init.SrcBurstLength = 1U;
  node_config.Init.DestBurstLength = 1U;
  node_config.Init.TransferAllocatedPort =
      DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
  node_config.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
  node_config.Init.Mode = DMA_NORMAL;
  node_config.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
  node_config.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
  node_config.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;

  if (HAL_DMAEx_List_BuildNode(&node_config, &backend.dma_node) != HAL_OK ||
      HAL_DMAEx_List_InsertNode(&backend.dma_queue, nullptr, &backend.dma_node) != HAL_OK ||
      HAL_DMAEx_List_SetCircularMode(&backend.dma_queue) != HAL_OK) {
    return false;
  }

  backend.dma_channel_handle.Instance = GPDMA1_Channel0;
  backend.dma_channel_handle.InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
  backend.dma_channel_handle.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
  backend.dma_channel_handle.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
  backend.dma_channel_handle.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
  backend.dma_channel_handle.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;

  if (HAL_DMAEx_List_Init(&backend.dma_channel_handle) != HAL_OK ||
      HAL_DMAEx_List_LinkQ(&backend.dma_channel_handle, &backend.dma_queue) != HAL_OK ||
      HAL_DMA_ConfigChannelAttributes(&backend.dma_channel_handle, DMA_CHANNEL_NPRIV) !=
          HAL_OK) {
    return false;
  }

  HAL_NVIC_SetPriority(GPDMA1_Channel0_IRQn, 6U, 0U);
  HAL_NVIC_EnableIRQ(GPDMA1_Channel0_IRQn);

  backend.adc_handle.Instance = k_adc_dma_configs.front().p_instance;
  backend.adc_handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  backend.adc_handle.Init.Resolution = ADC_RESOLUTION_12B;
  backend.adc_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  backend.adc_handle.Init.ScanConvMode = ADC_SCAN_ENABLE;
  backend.adc_handle.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  backend.adc_handle.Init.LowPowerAutoWait = DISABLE;
  backend.adc_handle.Init.ContinuousConvMode = ENABLE;
  backend.adc_handle.Init.NbrOfConversion = static_cast<uint32_t>(k_adc_count);
  backend.adc_handle.Init.DiscontinuousConvMode = DISABLE;
  backend.adc_handle.Init.NbrOfDiscConversion = 1U;
  backend.adc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  backend.adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  backend.adc_handle.Init.SamplingMode = ADC_SAMPLING_MODE_NORMAL;
  backend.adc_handle.Init.DMAContinuousRequests = ENABLE;
  backend.adc_handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  backend.adc_handle.Init.OversamplingMode = DISABLE;
  __HAL_LINKDMA(&backend.adc_handle, DMA_Handle, backend.dma_channel_handle);

  if (HAL_ADC_Init(&backend.adc_handle) != HAL_OK) {
    return false;
  }

  ADC_ChannelConfTypeDef channel_config{};
  channel_config.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
  channel_config.SingleDiff = ADC_SINGLE_ENDED;
  channel_config.OffsetNumber = ADC_OFFSET_NONE;
  channel_config.Offset = 0U;
  channel_config.OffsetSign = ADC_OFFSET_SIGN_NEGATIVE;
  channel_config.OffsetSaturation = DISABLE;

  for (std::size_t rank = 0U; rank < k_adc_dma_configs.size(); ++rank) {
    channel_config.Channel = k_adc_dma_configs[rank].channel;
    channel_config.Rank = static_cast<uint32_t>(rank + 1U);
    if (HAL_ADC_ConfigChannel(&backend.adc_handle, &channel_config) != HAL_OK) {
      return false;
    }
  }

  if (HAL_ADCEx_Calibration_Start(&backend.adc_handle, ADC_SINGLE_ENDED) != HAL_OK ||
      HAL_ADC_Start_DMA(&backend.adc_handle,
                        reinterpret_cast<uint32_t*>(backend.dma_buffer.data()),
                        static_cast<uint32_t>(backend.dma_buffer.size())) != HAL_OK) {
    return false;
  }

  backend.running = true;
  return true;
}

uint32_t lock_irq() noexcept {
  const uint32_t primask = __get_PRIMASK();
  __disable_irq();
  return primask;
}

void unlock_irq(const uint32_t primask) noexcept {
  __set_PRIMASK(primask);
}

expected::expected<uint16_t, result> read_dma_average(const AdcId id) noexcept {
  auto& backend = shared_dma_backend();
  const auto channel = adc_index(id);
  if (channel >= k_adc_count) {
    return expected::unexpected(result::RECOVERABLE_ERROR);
  }

  const uint32_t primask = lock_irq();
  const uint64_t sum = backend.sums[channel];
  const uint32_t count = backend.sample_counts[channel];
  backend.sums[channel] = 0U;
  backend.sample_counts[channel] = 0U;
  unlock_irq(primask);

  if (count == 0U) {
    return expected::unexpected(result::RECOVERABLE_ERROR);
  }

  return static_cast<uint16_t>(sum / count);
}

expected::expected<std::optional<uint16_t>, result> try_read_dma_average(
    const AdcId id) noexcept {
  auto& backend = shared_dma_backend();
  const auto channel = adc_index(id);
  if (channel >= k_adc_count) {
    return expected::unexpected(result::RECOVERABLE_ERROR);
  }

  const uint32_t primask = lock_irq();
  const uint64_t sum = backend.sums[channel];
  const uint32_t count = backend.sample_counts[channel];
  backend.sums[channel] = 0U;
  backend.sample_counts[channel] = 0U;
  unlock_irq(primask);

  if (count == 0U) {
    return std::optional<uint16_t>{};
  }

  return std::optional<uint16_t>{static_cast<uint16_t>(sum / count)};
}
}  // namespace

result Adc::start() noexcept {
  return result::OK;
}

Adc::Adc(const AdcId id) noexcept : m_id(id), m_opaque(make_opaque(id)) {
}

result Adc::init() noexcept {
  if (start_dma_backend()) {
    return result::OK;
  }

  return m_opaque.init(&adc_handle(m_id));
}

result Adc::stop() noexcept {
  if (shared_dma_backend().running) {
    return result::OK;
  }

  return m_opaque.stop(&adc_handle(m_id));
}

expected::expected<uint16_t, result> Adc::read() noexcept {
  if (shared_dma_backend().running) {
    return read_dma_average(m_id);
  }

  uint16_t value{0U};
  const auto status = m_opaque.read(&adc_handle(m_id), value);
  if (status != result::OK) {
    return expected::unexpected(status);
  }

  return value;
}

expected::expected<std::optional<uint16_t>, result> Adc::try_read() noexcept {
  if (shared_dma_backend().running) {
    return try_read_dma_average(m_id);
  }

  bool has_value{false};
  uint16_t value{0U};
  const auto status = m_opaque.try_read(&adc_handle(m_id), has_value, value);
  if (status != result::OK) {
    return expected::unexpected(status);
  }

  if (!has_value) {
    return std::optional<uint16_t>{};
  }

  return std::optional<uint16_t>{value};
}

void adc_dma_half_transfer_callback(ADC_HandleTypeDef* hadc) noexcept {
  auto& backend = shared_dma_backend();
  if (hadc != &backend.adc_handle) {
    return;
  }

  accumulate_dma_until(backend, k_dma_half_buffer_length);
}

void adc_dma_full_transfer_callback(ADC_HandleTypeDef* hadc) noexcept {
  auto& backend = shared_dma_backend();
  if (hadc != &backend.adc_handle) {
    return;
  }

  accumulate_dma_until(backend, k_dma_buffer_length);
  backend.processed_samples_in_cycle = 0U;
}

void adc_dma_irq_handler() noexcept {
  auto& backend = shared_dma_backend();
  HAL_DMA_IRQHandler(&backend.dma_channel_handle);
}
}  // namespace ru::driver

extern "C" void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
  ru::driver::adc_dma_half_transfer_callback(hadc);
}

extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
  ru::driver::adc_dma_full_transfer_callback(hadc);
}

extern "C" void GPDMA1_Channel0_IRQHandler(void) {
  ru::driver::adc_dma_irq_handler();
}
