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

// ============================================
// Compile-Time Tuple Filtering Utilities
// ============================================

namespace detail {
    // Concatenate tuples
    template<typename... Tuples>
    struct tuple_concat;
    
    template<>
    struct tuple_concat<> {
        using type = std::tuple<>;
    };
    
    template<typename... T>
    struct tuple_concat<std::tuple<T...>> {
        using type = std::tuple<T...>;
    };
    
    template<typename... T1, typename... T2, typename... Rest>
    struct tuple_concat<std::tuple<T1...>, std::tuple<T2...>, Rest...> {
        using type = typename tuple_concat<std::tuple<T1..., T2...>, Rest...>::type;
    };
    
    template<typename... Tuples>
    using tuple_concat_t = typename tuple_concat<Tuples...>::type;
}

// Filter tuple based on predicate
template<typename Tuple, template<typename> typename Predicate>
struct FilterTuple;

template<typename... Types, template<typename> typename Predicate>
struct FilterTuple<std::tuple<Types...>, Predicate> {
    using type = detail::tuple_concat_t<
        std::conditional_t<Predicate<Types>::value, std::tuple<Types>, std::tuple<>>...
    >;
};

template<typename Tuple, template<typename> typename Predicate>
using FilterTuple_t = typename FilterTuple<Tuple, Predicate>::type;

// Method existence detection
template<typename T>
struct HasDownLinkMethod : std::false_type {};

template<typename T>
    requires requires(T t) { t.getDownLinkLayout(); }
struct HasDownLinkMethod<T> : std::true_type {};

template<typename T>
struct HasUpLinkMethod : std::false_type {};

template<typename T>
    requires requires(T t) { t.getUpLinkLayout(); }
struct HasUpLinkMethod<T> : std::true_type {};

template <bool IsMaster, typename TxTuple, typename RxTuple>
class Frame;

template <bool IsMaster, typename... TxSyncables, typename... RxSyncables>
class Frame<IsMaster, std::tuple<TxSyncables...>, std::tuple<RxSyncables...>> {
public:
    // ===========================================
    // Frame Protocol Constants
    // ===========================================
    static constexpr uint8_t START_BYTE = 0xAB;
    static constexpr uint8_t END_BYTE = 0xCD;

    // ===========================================
    // 1. Compile-Time Deduction
    // ===========================================
    // Note: TxSyncables are already filtered to have getDownLinkLayout()
    //       RxSyncables are already filtered to have getUpLinkLayout()

    template <typename T>
    using TxLayoutT = decltype(std::declval<T>().getDownLinkLayout());

    template <typename T>
    using RxLayoutT = decltype(std::declval<T>().getUpLinkLayout());

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

    static void update_tx(volatile bool *flag = nullptr) { 
        if constexpr (TxNodeCount > 0) MDMA::transfer_list(reinterpret_cast<MDMA::LinkedListNode*>(&tx_node_storage[0]), flag);
    }

    static void update_rx(volatile bool *flag = nullptr) { 
        if constexpr (RxNodeCount > 0) MDMA::transfer_list(reinterpret_cast<MDMA::LinkedListNode*>(&rx_node_storage[0]), flag);
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
 * Automatically filters types into Downlink and Uplink based on method existence.
 */
template<bool IsMaster, typename... SyncableTypes>
using DuplexFrame = typename std::conditional_t<
    IsMaster,
    Frame<true, FilterTuple_t<std::tuple<SyncableTypes...>, HasDownLinkMethod>, FilterTuple_t<std::tuple<SyncableTypes...>, HasUpLinkMethod>>, 
    Frame<false, FilterTuple_t<std::tuple<SyncableTypes...>, HasUpLinkMethod>, FilterTuple_t<std::tuple<SyncableTypes...>, HasDownLinkMethod>>
>;

#endif // FRAME_SHARED_HPP