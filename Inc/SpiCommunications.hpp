#ifndef SPI_COMMUNICATIONS_HPP
#define SPI_COMMUNICATIONS_HPP

#include "C++Utilities/CppImports.hpp"
#include "HALAL/Services/InfoWarning/InfoWarning.hpp"
#include "HALAL/Services/Time/Scheduler.hpp"


/**
 * @brief A template class to manage SPI communications, handling transmission, reception, validation, and error management.
 * 
 * @tparam SpiReady A function pointer that returns true when the SPI peripheral is ready for a new transaction.
 * @tparam max_errors The maximum number of errors before taking corrective action. Set to 0 to disable error counting.
 * @tparam spi_timeout_limit The number of update cycles to wait before considering a timeout. Set to 0 to disable timeout handling.
 */
template <auto& SpiWrapper, typename FrameType, bool (*SpiReady)(), void (*OnTx)(), void (*OnRxReceived)(), void (*OnRxValid)(), void (*OnRxInvalid)(), void (*OnTimeout)(), void (*OnMaxErrors)(), size_t max_errors = 0, size_t spi_timeout_limit = 0>
class SpiCommunications {
public:
    void update() {
        // Bootstrap the communications
        if (!operation_flag) {
            operation_flag = true;
            receive_flag = true;
            if constexpr (spi_timeout_limit > 0) timeout_counter = Scheduler::get_global_tick();
            FrameType::update_tx(&send_flag);

        } else if (send_flag && receive_flag && SpiReady()) {
            send_flag = false;
            receive_flag = false;
            if constexpr (requires {
                              SpiWrapper.transceive_DMA(
                                  FrameType::tx_buffer,
                                  FrameType::rx_buffer,
                                  &spi_flag
                              );
                          }) {
                SpiWrapper.transceive_DMA(FrameType::tx_buffer, FrameType::rx_buffer, &spi_flag);
            } else {
                SpiWrapper.transceive(FrameType::tx_buffer, FrameType::rx_buffer, &spi_flag);
            }
            OnTx();

        } else if (spi_flag) {
            spi_flag = false;
            OnRxReceived();

            if (FrameType::validate()) {
                FrameType::update_rx(&receive_flag);
                FrameType::update_tx(&send_flag);
                if constexpr (spi_timeout_limit > 0) timeout_counter = Scheduler::get_global_tick();
                if constexpr (max_errors > 0) {
                    if (error_count > 0) error_count--;
                }
                has_ever_connected = true;
                OnRxValid();
            } else {
                error_occurred();
                OnRxInvalid();
            }

        }

        if constexpr (spi_timeout_limit > 0) {
            if (operation_flag && (Scheduler::get_global_tick() - timeout_counter >= spi_timeout_limit)) {
                if (!has_ever_connected) {
                    return; // Don't trigger timeout if we haven't connected yet
                }
                error_occurred();
                OnTimeout();
            }
        }
    }

    void error_occurred() {
        WARNING("SPI Communication Error Occurred");
        if constexpr (max_errors > 0) {
            error_count++;
            if (error_count >= max_errors) {
                error_count = max_errors;
                OnMaxErrors();
            }
        }
        reset_state_machine();
    }

    void reset_state_machine() {
        operation_flag = false;
        send_flag = false;
        spi_flag = false;
        receive_flag = false;
    }

    bool is_connected() const {
        if constexpr (max_errors > 0) {
            return error_count < max_errors && has_ever_connected;
        } else {
            return has_ever_connected;
        }
    }

private:
    volatile bool send_flag = false;
    volatile bool spi_flag = false;
    volatile bool receive_flag = false;
    volatile bool operation_flag = false;

    uint32_t error_count = 0;
    uint64_t timeout_counter = 0;
    bool has_ever_connected = false;
};

#endif // SPI_COMMUNICATIONS_HPP
