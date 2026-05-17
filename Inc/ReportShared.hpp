#ifndef REPORT_SHARED_HPP
#define REPORT_SHARED_HPP

#include <cstring>
#include <type_traits>

#include "C++Utilities/CppImports.hpp"
#include "HALAL/Services/Diagnostics/Diagnostics.hpp"

class ReportBase {
public:
    static_assert(
        std::is_trivially_copyable_v<Diagnostics::DiagnosticRecord>,
        "DiagnosticRecord must be trivially copyable for byte-copy updates."
    );

    auto get_uplink_layout() {
        return std::make_tuple(&record, &seq_num);
    }

    auto get_downlink_layout() {
        return std::tuple<>{};
    }

    void set_from_record(const Diagnostics::DiagnosticRecord& rec) {
        // Byte copy into volatile storage (e.g., HW-visible buffer).
        std::memcpy(
            const_cast<void*>(static_cast<const volatile void*>(&record)),
            &rec,
            sizeof(record)
        );
        has_report = true;
        auto current_seq = seq_num;
        seq_num = current_seq + 1;
    }

    void clear() {
        has_report = false;
    }

    bool is_valid() const { return has_report; }

private:
    volatile Diagnostics::DiagnosticRecord record;
    volatile uint32_t seq_num{0};
    volatile bool has_report = false;
};

#endif // REPORT_SHARED_HPP
