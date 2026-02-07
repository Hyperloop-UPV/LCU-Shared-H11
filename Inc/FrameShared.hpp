#ifndef FRAME_SHARED_HPP
#define FRAME_SHARED_HPP

#include "HALAL/Models/MDMA/MDMA.hpp"
#include "C++Utilities/CppImports.hpp"

template <typename Tuple>
consteval size_t layout_bytes_v() {
    return []<typename... Ts>(std::type_identity<std::tuple<Ts...>>) {
        return (sizeof(std::remove_pointer_t<Ts>) + ... + 0);
    }(std::type_identity<Tuple>{});
}

template <typename Tuple>
consteval size_t layout_count_v() {
    return []<typename... Ts>(std::type_identity<std::tuple<Ts...>>) {
        return sizeof...(Ts);
    }(std::type_identity<Tuple>{});
}

template <bool IsMaster, typename TxTuple, typename RxTuple>
class Frame;

template <bool IsMaster, typename... TxSyncables, typename... RxSyncables>
class Frame<IsMaster, std::tuple<TxSyncables...>, std::tuple<RxSyncables...>> {
public:
    // ===========================================
    // 1. Compile-Time Deduction
    // ===========================================

    template <typename T>
    using TxLayoutT = std::conditional_t<IsMaster,
        decltype(std::declval<T>().getDownLinkLayout()), 
        decltype(std::declval<T>().getUpLinkLayout())
    >;

    template <typename T>
    using RxLayoutT = std::conditional_t<IsMaster, 
        decltype(std::declval<T>().getUpLinkLayout()), 
        decltype(std::declval<T>().getDownLinkLayout())
    >;

    static constexpr size_t TxDataSize = (layout_bytes_v<TxLayoutT<TxSyncables>>() + ... + 0);
    static constexpr size_t RxDataSize = (layout_bytes_v<RxLayoutT<RxSyncables>>() + ... + 0);
    static constexpr size_t TotalSize = std::max(TxDataSize, RxDataSize);

    // Node Counts
    static constexpr size_t TxNodeCount = (layout_count_v<TxLayoutT<TxSyncables>>() + ... + 0);
    static constexpr size_t RxNodeCount = (layout_count_v<RxLayoutT<RxSyncables>>() + ... + 0);


    // ===========================================
    // 2. Storage
    // ===========================================

    alignas(32) D1_NC static inline uint8_t tx_buffer[TotalSize]; // Cache line alignment just in case
    alignas(32) D1_NC static inline uint8_t rx_buffer[TotalSize]; // Cache line alignment just in case

    struct NodeWrapper { // Mdma::LinkedListNode doesn't have a default constructor
        alignas(alignof(MDMA::LinkedListNode)) uint8_t data[sizeof(MDMA::LinkedListNode)];
    };

    // Ensure array size is at least 1 to satisfy C++ standards, may change later to something else
    static inline D1_NC NodeWrapper tx_node_storage[TxNodeCount > 0 ? TxNodeCount : 1];
    static inline D1_NC NodeWrapper rx_node_storage[RxNodeCount > 0 ? RxNodeCount : 1];


    // ===========================================
    // 3. Logic
    // ===========================================

    static void init(TxSyncables&... tx_parts, RxSyncables&... rx_parts) {
        if constexpr (TxNodeCount > 0) initDirection<true>(tx_parts...);
        if constexpr (RxNodeCount > 0) initDirection<false>(rx_parts...);
    }

    static void update_tx() { 
        if constexpr (TxNodeCount > 0) MDMA::transfer_list(reinterpret_cast<MDMA::LinkedListNode*>(&tx_node_storage[0]));
    }

    static void update_rx() { 
        if constexpr (RxNodeCount > 0) MDMA::transfer_list(reinterpret_cast<MDMA::LinkedListNode*>(&rx_node_storage[0]));
    }

private:
   
    template <bool IsTx, typename... Syncables>
    static void initDirection(Syncables&... parts) {
        size_t buffer_offset = 0;
        size_t node_idx = 0;

        auto processObject = [&](auto& part) {
            // Deduce layout tuple
            auto layout = [&]() {
                if constexpr (IsTx) {
                    if constexpr (IsMaster) return part.getDownLinkLayout();
                    else return part.getUpLinkLayout();
                } else {
                    if constexpr (IsMaster) return part.getUpLinkLayout();
                    else return part.getDownLinkLayout();
                }
            }();

            // Unpack tuple and generate nodes
            std::apply([&](auto*... args) {
                ((createNode<IsTx>(
                    node_idx, 
                    buffer_offset, 
                    args
                 )), ...);
            }, layout);
        };

        (processObject(parts), ...);
        
        // Terminate the linked list
        if (node_idx > 0) {
             NodeWrapper* storage = IsTx ? tx_node_storage : rx_node_storage;
             reinterpret_cast<MDMA::LinkedListNode*>(&storage[node_idx - 1])->set_next(nullptr);
        }
    }

    template <bool IsTx, typename T>
    static void createNode(size_t& idx, size_t& offset, T* ptr) {
        using ValueType = std::remove_pointer_t<T>;
        size_t size = sizeof(ValueType);
        
        NodeWrapper* storage = IsTx ? tx_node_storage : rx_node_storage;
        
        auto* buffer  = IsTx ? tx_buffer : rx_buffer;

        void* src = IsTx ? (void*)ptr : (void*)(buffer + offset);
        void* dst = IsTx ? (void*)(buffer + offset) : (void*)ptr;

        MDMA::LinkedListNode* node = new (&storage[idx]) MDMA::LinkedListNode(src, dst, size);

        if (idx > 0) {
            MDMA::LinkedListNode* prev = reinterpret_cast<MDMA::LinkedListNode*>(&storage[idx - 1]);
            prev->set_next(node->get_node());
        }

        offset += size;
        idx++;
    }
};

/**
 * @brief Direction-aware Frame Alias.
 */
template<bool IsMaster, typename MasterToSlaveStream, typename SlaveToMasterStream>
using DuplexFrame = typename std::conditional_t<
    IsMaster,
    Frame<true, MasterToSlaveStream, SlaveToMasterStream>, 
    Frame<false, SlaveToMasterStream, MasterToSlaveStream>
>;

#endif // FRAME_SHARED_HPP