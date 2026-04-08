#ifndef SHARED_COMMUNICATIONS_LOGIC_HPP
#define SHARED_COMMUNICATIONS_LOGIC_HPP

#include "C++Utilities/CppImports.hpp"
#include "HALAL/Services/InfoWarning/InfoWarning.hpp"

// ============================================
// Callbacks Policy
// ============================================
struct DefaultCallbacks {
    static void on_prepare_tx() {}
    static void on_spi_start() {}
    static void on_spi_complete() {}
    static void on_data_received() {}
    static void on_frame_error() {}
};


// ============================================
// Shared SPI Communication State Machine
// ============================================
// Template Parameters:
//   FrameT: Frame type (e.g., LCU_Master::Frame, LCU_Slave::Frame)
//   SPIInstanceT: SPI wrapper type
//   CallbacksT: User-provided callbacks (default: DefaultCallbacks)
//   IsMaster: True for master mode, Flase for Slave
//   EnableErrorHandling: Track error counter and exponential backoff
//   MaxErrors: Maximum error counter value (clamping)
//   EnableTimeout: Use time-based timeout
//   TimeoutMs: SPI timeout in milliseconds (if EnableTimeout=true)
template<
    typename FrameT, 
    typename SPIInstanceT, 
    typename CallbacksT = DefaultCallbacks,
    bool IsMaster = true,
    bool EnableErrorHandling = false,
    uint32_t MaxErrors = 10,
    bool EnableTimeout = false,
    uint32_t TimeoutMs = 100
>
class SharedSPICommunicationLogic {
public:
    // ============================================
    // State Management
    // ============================================
    volatile bool send_flag = false;
    volatile bool spi_flag = false;
    volatile bool receive_flag = false;
    volatile bool operation_flag = false;

    // Error tracking (compile-time conditional)
    uint32_t spi_error_counter = 0;
    
    // Time-based tracking (compile-time conditional, milliseconds)
    uint32_t operation_start_time = 0;
    static constexpr uint32_t SPI_TIMEOUT_MS = TimeoutMs;
    
    // Ready-to-transfer state (for polling readiness)
    bool waiting_for_ready = false;

    // ============================================
    // Configuration
    // ============================================
    SPIInstanceT* spi_ptr = nullptr;

    void init(SPIInstanceT* spi) {
        spi_ptr = spi;
    }

    // User calls this when slave is ready (e.g., from EXTI handler)
    void ready_for_transfer() {
        waiting_for_ready = false;
    }

    void update() {
        
        if constexpr (EnableErrorHandling) {
            if (operation_flag && spi_ptr->was_aborted()) {
                spi_ptr->clear_abort_flag();
                spi_error_counter++;
                reset_state();
                CallbacksT::on_frame_error();
                return;
            }
        }

        if constexpr (EnableTimeout) {
            if (operation_flag) {
                uint32_t elapsed = HAL_GetTick() - operation_start_time;
                if (elapsed > SPI_TIMEOUT_MS) {
                    if constexpr (EnableErrorHandling) {
                        spi_error_counter++;
                        CallbacksT::on_frame_error();
                    } else {
                        WARNING("SPI timeout exceeded - restarting communication");
                    }
                    reset_state();
                    return;
                }
            }
        }

        // ===== STATE MACHINE =====
        
        if (!operation_flag) {
            // ===== STATE 1: Prepare Transmission =====
            CallbacksT::on_prepare_tx();
            operation_flag = true;
            waiting_for_ready = true;  // Start waiting for readiness check
            FrameT::update_tx(&send_flag);
            
            if constexpr (EnableTimeout) {
                operation_start_time = HAL_GetTick();
            }

        } else if (send_flag && !waiting_for_ready) {
            // ===== STATE 2: Initiate SPI Transfer =====
            send_flag = false;
            
            // Master: Use transceive_DMA with operation flag
            // Slave: Use transceive which handles DMA internally
            if constexpr (IsMaster) {
                // Master mode: explicit DMA with operation flag
                spi_ptr->transceive_DMA(FrameT::tx_buffer, FrameT::rx_buffer, &spi_flag);
            } else {
                // Slave mode: transceive already uses DMA
                spi_ptr->transceive(FrameT::tx_buffer, FrameT::rx_buffer, &spi_flag);
            }

            CallbacksT::on_spi_start();

        } else if (spi_flag) {
            // ===== STATE 3: SPI Complete, Validate Frame =====
            spi_flag = false;
            CallbacksT::on_spi_complete();

            if (!validate_frame_boundaries()) {
                if constexpr (EnableErrorHandling) {
                    spi_error_counter++;
                    if (spi_error_counter > MaxErrors) {
                        spi_error_counter = MaxErrors;
                    }
                } else {
                    WARNING("Invalid frame boundaries (START/END byte mismatch)");
                }
                
                CallbacksT::on_frame_error();
                operation_flag = false;
                return;
            }

            // Frame valid: proceed to RX processing
            FrameT::update_rx(&receive_flag);

        } else if (receive_flag) {
            // ===== STATE 4: Process Received Data =====
            receive_flag = false;
            operation_flag = false;
            waiting_for_ready = false;
            CallbacksT::on_data_received();

            if constexpr (EnableErrorHandling) {
                if (spi_error_counter > 0) {
                    spi_error_counter--;
                }
            }
        }
    }

    // ============================================
    // Frame Validation
    // ============================================
    [[nodiscard]] bool validate_frame_boundaries() const {
        // Check START_BYTE and END_BYTE at expected locations
        if (FrameT::rx_buffer[0] != FrameT::START_BYTE) {
            return false;
        }
        
        // Check END_BYTE at the last position using sizeof
        constexpr size_t frame_size = sizeof(FrameT::rx_buffer);
        if (FrameT::rx_buffer[frame_size - 1] != FrameT::END_BYTE) {
            return false;
        }

        return true;
    }

    void reset_state() {
        operation_flag = false;
        send_flag = false;
        spi_flag = false;
        receive_flag = false;
        waiting_for_ready = false;
    }

    // ============================================
    // EXTI & NSS Integration
    // ============================================
    // EXTI Handling Pattern:
    // ──────────────────────
    // The EXTI-based readiness signaling is handled via callbacks and the waiting_for_ready flag:
    //
    // 1. on_prepare_tx():
    //    Slave: Toggle slave_ready signal, assert_nss (software) and wait_for_ready = false (the slave doesn't need to wait normally, but you could use nss here if you have multiple slaves on the same bus or something)
    //
    // 2. on_spi_complete():
    //    Slave: Deassert slave_ready signal
    //
    // 3. ready_for_transfer():
    //   Master: Called from EXTI handler when slave signals ready (e.g., slave
    //   Slave: Can be just callend on_prepare_tx() to simulate immediate readiness, or called from another EXTI if slave has a separate ready signal
    //
    // NSS (Chip Select) Handling:
    // ──────────────────────────
    // For hardware NSS: SPI peripheral handles automatically via hardware pin
    //
    // For software NSS (Slave mode):
    //   - Call assert_nss() in on_prepare_tx() to select slave (set NSS low)
    //   - Call deassert_nss() in on_data_received() to deselect (set NSS high)
    //
    // Example setup:
    //      struct MySlaveCallbacks {
    //          static void on_prepare_tx() {
    //              my_slave.assert_nss();  // Select slave for next potential master
    //          }
    //          static void on_data_received() {
    //              my_slave.deassert_nss();  // Deselect after data processed
    //          }
    //      };
};

#endif // SHARED_COMMUNICATIONS_LOGIC_HPP
