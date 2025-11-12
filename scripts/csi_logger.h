/**
 * CSI Logger Header File
 * 
 * Include this header in your SRS processor file to add CSI logging functionality
 */

#ifndef SRSRAN_CSI_LOGGER_H
#define SRSRAN_CSI_LOGGER_H

#include <fstream>
#include <iomanip>
#include <chrono>
#include <string>
#include <mutex>
#include <complex>
#include <vector>

namespace srsran {

/**
 * Logger for Channel State Information (CSI) extracted from SRS signals
 */
class csi_logger {
public:
    /**
     * Initialize the CSI logger
     * @param log_dir Directory where CSI log files will be written
     */
    static void init(const std::string& log_dir = "/tmp/csi_logs");

    /**
     * Log channel measurement to file
     * @param slot_idx Slot index
     * @param symbol_idx Symbol index within slot
     * @param rnti Radio Network Temporary Identifier (UE identifier)
     * @param prb_start Starting PRB index
     * @param nof_prb Number of PRBs
     * @param channel_matrix Channel coefficients [port][layer][prb][re]
     * @param rsrp_db RSRP in dB (optional, NaN if not available)
     * @param rsrq_db RSRQ in dB (optional, NaN if not available)
     * @param sinr_db SINR in dB (optional, NaN if not available)
     */
    static void log_csi(uint32_t slot_idx,
                       uint32_t symbol_idx,
                       uint16_t rnti,
                       uint32_t prb_start,
                       uint32_t nof_prb,
                       const std::vector<std::vector<std::vector<std::complex<float>>>>& channel_matrix,
                       float rsrp_db = NAN,
                       float rsrq_db = NAN,
                       float sinr_db = NAN);

    /**
     * Log channel measurement (simplified version for 1x1 MIMO)
     * @param slot_idx Slot index
     * @param symbol_idx Symbol index within slot
     * @param rnti Radio Network Temporary Identifier
     * @param prb_start Starting PRB index
     * @param nof_prb Number of PRBs
     * @param channel_coeffs Channel coefficients per PRB [prb][re] (12 REs per PRB)
     * @param rsrp_db RSRP in dB
     */
    static void log_csi_simple(uint32_t slot_idx,
                              uint32_t symbol_idx,
                              uint16_t rnti,
                              uint32_t prb_start,
                              uint32_t nof_prb,
                              const std::vector<std::vector<std::complex<float>>>& channel_coeffs,
                              float rsrp_db = NAN);

    /**
     * Close the log file (call on shutdown)
     */
    static void close();

private:
    static std::ofstream log_file_;
    static std::mutex log_mutex_;
    static std::string log_dir_;
    static std::string current_file_;
    static bool initialized_;
    static uint64_t file_rotation_counter_;

    static void rotate_file();
    static std::string get_timestamp_string();
};

} // namespace srsran

#endif // SRSRAN_CSI_LOGGER_H

