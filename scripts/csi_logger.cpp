/**
 * CSI Logger Implementation
 * 
 * Implementation of CSI logging functionality for srsRAN
 */

#include "csi_logger.h"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <cmath>

namespace srsran {

// Static member initialization
std::ofstream csi_logger::log_file_;
std::mutex csi_logger::log_mutex_;
std::string csi_logger::log_dir_ = "/tmp/csi_logs";
std::string csi_logger::current_file_;
bool csi_logger::initialized_ = false;
uint64_t csi_logger::file_rotation_counter_ = 0;

void csi_logger::init(const std::string& log_dir) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    log_dir_ = log_dir;
    
    // Create log directory if it doesn't exist
    try {
        std::filesystem::create_directories(log_dir_);
    } catch (const std::exception& e) {
        std::cerr << "Error creating CSI log directory: " << e.what() << std::endl;
        return;
    }
    
    rotate_file();
    initialized_ = true;
    
    std::cout << "CSI Logger initialized. Logging to: " << current_file_ << std::endl;
}

void csi_logger::rotate_file() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
    
    // Create filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << log_dir_ << "/csi_"
       << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S")
       << "_" << std::setfill('0') << std::setw(3) << ms.count()
       << "_" << file_rotation_counter_++
       << ".csv";
    
    current_file_ = ss.str();
    log_file_.open(current_file_, std::ios::out | std::ios::app);
    
    if (log_file_.is_open()) {
        // Write CSV header
        log_file_ << "timestamp,slot,symbol,rnti,"
                  << "prb_start,nof_prb,prb_idx,re_idx,"
                  << "rx_port,tx_layer,"
                  << "csi_real,csi_imag,csi_magnitude,csi_phase,"
                  << "rsrp_db,rsrq_db,sinr_db\n";
        log_file_.flush();
    } else {
        std::cerr << "Error opening CSI log file: " << current_file_ << std::endl;
    }
}

std::string csi_logger::get_timestamp_string() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
       << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

void csi_logger::log_csi(uint32_t slot_idx,
                        uint32_t symbol_idx,
                        uint16_t rnti,
                        uint32_t prb_start,
                        uint32_t nof_prb,
                        const std::vector<std::vector<std::vector<std::complex<float>>>>& channel_matrix,
                        float rsrp_db,
                        float rsrq_db,
                        float sinr_db) {
    if (!initialized_) {
        init();
    }
    
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (!log_file_.is_open()) {
        return;
    }
    
    std::string timestamp = get_timestamp_string();
    
    // Log each channel coefficient
    for (size_t port = 0; port < channel_matrix.size(); ++port) {
        for (size_t layer = 0; layer < channel_matrix[port].size(); ++layer) {
            for (size_t prb_idx = 0; prb_idx < channel_matrix[port][layer].size(); ++prb_idx) {
                // Each PRB has 12 REs (resource elements)
                const auto& prb_data = channel_matrix[port][layer][prb_idx];
                for (size_t re_idx = 0; re_idx < prb_data.size() && re_idx < 12; ++re_idx) {
                    const auto& csi_value = prb_data[re_idx];
                    float magnitude = std::abs(csi_value);
                    float phase = std::arg(csi_value);
                    
                    log_file_ << timestamp << ","
                              << slot_idx << ","
                              << symbol_idx << ","
                              << rnti << ","
                              << prb_start << ","
                              << nof_prb << ","
                              << (prb_start + prb_idx) << ","
                              << re_idx << ","
                              << port << ","
                              << layer << ","
                              << std::scientific << std::setprecision(6)
                              << csi_value.real() << ","
                              << csi_value.imag() << ","
                              << magnitude << ","
                              << phase;
                    
                    // Only write measurements once per sample (use first RE)
                    if (port == 0 && layer == 0 && prb_idx == 0 && re_idx == 0) {
                        if (!std::isnan(rsrp_db)) {
                            log_file_ << "," << rsrp_db;
                        } else {
                            log_file_ << ",";
                        }
                        if (!std::isnan(rsrq_db)) {
                            log_file_ << "," << rsrq_db;
                        } else {
                            log_file_ << ",";
                        }
                        if (!std::isnan(sinr_db)) {
                            log_file_ << "," << sinr_db;
                        } else {
                            log_file_ << ",";
                        }
                    } else {
                        log_file_ << ",,,";
                    }
                    
                    log_file_ << "\n";
                }
            }
        }
    }
    
    log_file_.flush();
}

void csi_logger::log_csi_simple(uint32_t slot_idx,
                               uint32_t symbol_idx,
                               uint16_t rnti,
                               uint32_t prb_start,
                               uint32_t nof_prb,
                               const std::vector<std::vector<std::complex<float>>>& channel_coeffs,
                               float rsrp_db) {
    if (!initialized_) {
        init();
    }
    
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (!log_file_.is_open()) {
        return;
    }
    
    std::string timestamp = get_timestamp_string();
    
    // Log each channel coefficient (1x1 MIMO case)
    for (size_t prb_idx = 0; prb_idx < channel_coeffs.size() && prb_idx < nof_prb; ++prb_idx) {
        const auto& prb_data = channel_coeffs[prb_idx];
        for (size_t re_idx = 0; re_idx < prb_data.size() && re_idx < 12; ++re_idx) {
            const auto& csi_value = prb_data[re_idx];
            float magnitude = std::abs(csi_value);
            float phase = std::arg(csi_value);
            
            log_file_ << timestamp << ","
                      << slot_idx << ","
                      << symbol_idx << ","
                      << rnti << ","
                      << prb_start << ","
                      << nof_prb << ","
                      << (prb_start + prb_idx) << ","
                      << re_idx << ","
                      << "0,"  // rx_port
                      << "0,"  // tx_layer
                      << std::scientific << std::setprecision(6)
                      << csi_value.real() << ","
                      << csi_value.imag() << ","
                      << magnitude << ","
                      << phase;
            
            // Only write measurements once per sample
            if (prb_idx == 0 && re_idx == 0) {
                if (!std::isnan(rsrp_db)) {
                    log_file_ << "," << rsrp_db << ",,";
                } else {
                    log_file_ << ",,,";
                }
            } else {
                log_file_ << ",,,";
            }
            
            log_file_ << "\n";
        }
    }
    
    log_file_.flush();
}

void csi_logger::close() {
    std::lock_guard<std::mutex> lock(log_mutex_);
    if (log_file_.is_open()) {
        log_file_.close();
    }
    initialized_ = false;
}

} // namespace srsran

