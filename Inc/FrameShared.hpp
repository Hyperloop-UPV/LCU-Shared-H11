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

template <bool IsMaster, typename... Syncables>
class Frame {
public:
    template <typename T>
    using TxLayoutT = std::conditional_t<IsMaster,
        decltype(std::declval<T>().get_downlink_layout()),
        decltype(std::declval<T>().get_uplink_layout())
    >;

    template <typename T>
    using RxLayoutT = std::conditional_t<IsMaster,
        decltype(std::declval<T>().get_uplink_layout()),
        decltype(std::declval<T>().get_downlink_layout())
    >;

    static constexpr uint8_t START_BYTE = 0xAB;
    static constexpr uint8_t END_BYTE = 0xCD;

    static constexpr size_t TxDataSize = (layout_bytes_v<TxLayoutT<Syncables>>() + ... + 0);
    static constexpr size_t RxDataSize = (layout_bytes_v<RxLayoutT<Syncables>>() + ... + 0);
    static constexpr size_t TotalSize = std::max(TxDataSize, RxDataSize) + 2;

    static constexpr size_t TxNodeCount = (layout_count_v<TxLayoutT<Syncables>>() + ... + 0);
    static constexpr size_t RxNodeCount = (layout_count_v<RxLayoutT<Syncables>>() + ... + 0);

    alignas(32) D1_NC static inline uint8_t tx_buffer[TotalSize];
    alignas(32) D1_NC static inline uint8_t rx_buffer[TotalSize];

    struct NodeWrapper {
        alignas(alignof(MDMA::LinkedListNode)) uint8_t data[sizeof(MDMA::LinkedListNode)];
    };

    static inline D1_NC NodeWrapper tx_node_storage[TxNodeCount + 2];
    static inline D1_NC NodeWrapper rx_node_storage[RxNodeCount + 2];

    static void init(Syncables&... parts) {
        if constexpr (TxNodeCount > 0) init_direction<true>(parts...);
        if constexpr (RxNodeCount > 0) init_direction<false>(parts...);
    }

    static void update_tx(volatile bool* flag = nullptr) {
        if constexpr (TxNodeCount > 0) {
            MDMA::transfer_list(
                reinterpret_cast<volatile MDMA::LinkedListNode*>(&tx_node_storage[0]),
                flag
            );
        }
    }

    static void update_rx(volatile bool* flag = nullptr) {
        if constexpr (RxNodeCount > 0) {
            MDMA::transfer_list(
                reinterpret_cast<volatile MDMA::LinkedListNode*>(&rx_node_storage[0]),
                flag
            );
        }
    }

    static bool validate() {
        return rx_buffer[0] == START_BYTE && rx_buffer[TotalSize - 1] == END_BYTE;
    }

private:
    template <bool IsTx, typename... Parts>
    static void init_direction(Parts&... parts) {
        size_t buffer_offset = 0;
        size_t node_idx = 0;

        auto* storage = IsTx ? tx_node_storage : rx_node_storage;
        auto* buffer = IsTx ? tx_buffer : rx_buffer;

        if constexpr (IsTx) {
            new (const_cast<NodeWrapper*>(&storage[node_idx])) volatile MDMA::LinkedListNode(
                const_cast<uint8_t*>(&START_BYTE), const_cast<uint8_t*>(buffer), 1);
            node_idx++;
        } else {
            new (const_cast<NodeWrapper*>(&storage[node_idx])) volatile MDMA::LinkedListNode(
                const_cast<uint8_t*>(buffer), const_cast<uint8_t*>(&START_BYTE), 1);
            node_idx++;
        }

        auto process_object = [&](auto& part) {
            auto layout = [&]() {
                if constexpr (IsTx) {
                    if constexpr (IsMaster) return part.get_downlink_layout();
                    else return part.get_uplink_layout();
                } else {
                    if constexpr (IsMaster) return part.get_uplink_layout();
                    else return part.get_downlink_layout();
                }
            }();

            std::apply([&](auto*... args) {
                ((create_node<IsTx>(node_idx, buffer_offset, args)), ...);
            }, layout);
        };

        (process_object(parts), ...);

        if constexpr (IsTx) {
            new (const_cast<NodeWrapper*>(&storage[node_idx])) volatile MDMA::LinkedListNode(
                const_cast<uint8_t*>(&END_BYTE), const_cast<uint8_t*>(buffer + TotalSize - 1), 1);
        } else {
            new (const_cast<NodeWrapper*>(&storage[node_idx])) volatile MDMA::LinkedListNode(
                const_cast<uint8_t*>(buffer + TotalSize - 1), const_cast<uint8_t*>(&END_BYTE), 1);
        }

        reinterpret_cast<volatile MDMA::LinkedListNode*>(&storage[node_idx - 1])
            ->set_next(reinterpret_cast<volatile MDMA::LinkedListNode*>(&storage[node_idx])->get_node());

        node_idx++;

        reinterpret_cast<volatile MDMA::LinkedListNode*>(&storage[node_idx - 1])->set_next(nullptr);
    }

    template <bool IsTx, typename T>
    static void create_node(size_t& idx, size_t& offset, T* ptr) {
        size_t size = sizeof(T);

        auto* storage = IsTx ? tx_node_storage : rx_node_storage;
        auto* buffer = IsTx ? tx_buffer + 1 : rx_buffer + 1;

        void* src = IsTx ? const_cast<void*>(static_cast<const volatile void*>(ptr)) : const_cast<uint8_t*>(buffer + offset);
        void* dst = IsTx ? const_cast<uint8_t*>(buffer + offset) : const_cast<void*>(static_cast<const volatile void*>(ptr));

        volatile MDMA::LinkedListNode* node = new (const_cast<NodeWrapper*>(&storage[idx])) volatile MDMA::LinkedListNode(src, dst, size);

        volatile MDMA::LinkedListNode* prev = reinterpret_cast<volatile MDMA::LinkedListNode*>(&storage[idx - 1]);
        prev->set_next(node->get_node());

        offset += size;
        idx++;
    }
};

#endif // FRAME_SHARED_HPP
