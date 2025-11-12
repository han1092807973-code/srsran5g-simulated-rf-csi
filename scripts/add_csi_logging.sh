#!/bin/bash
# Script to add CSI logging to srsRAN Project
# This modifies the SRS channel estimation code to write CSI to a file

set -e

SRSRAN_DIR="${1:-/opt/srsRAN_Project}"
CSI_LOG_DIR="${2:-/tmp/csi_logs}"

if [ ! -d "$SRSRAN_DIR" ]; then
    echo "Error: srsRAN Project directory not found at $SRSRAN_DIR"
    echo "Usage: $0 [srsran_project_path] [csi_log_directory]"
    exit 1
fi

echo "Adding CSI logging to srsRAN Project"
echo "===================================="
echo "srsRAN Directory: $SRSRAN_DIR"
echo "CSI Log Directory: $CSI_LOG_DIR"
echo ""

# Create CSI log directory
mkdir -p "$CSI_LOG_DIR"
echo "Created CSI log directory: $CSI_LOG_DIR"

# Find the SRS processor file
echo "Searching for SRS processor files..."
SRS_FILES=$(find "$SRSRAN_DIR/lib/phy" -type f \( -name "*srs*processor*" -o -name "*srs*channel*" -o -name "*srs*estimator*" \) 2>/dev/null | head -5)

if [ -z "$SRS_FILES" ]; then
    echo "Warning: Could not find SRS processor files automatically."
    echo "Please run find_srs_csi_estimation.sh first to locate the files."
    echo ""
    echo "Common locations to check manually:"
    echo "  - $SRSRAN_DIR/lib/phy/upper/channel_processors/"
    echo "  - $SRSRAN_DIR/lib/phy/upper/signal_processors/"
    exit 1
fi

echo "Found SRS-related files:"
echo "$SRS_FILES"
echo ""

# Create a backup
BACKUP_DIR="$SRSRAN_DIR/.csi_logging_backup_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"
echo "Created backup directory: $BACKUP_DIR"

# Instructions for manual modification
cat > "$CSI_LOG_DIR/MODIFICATION_INSTRUCTIONS.md" << 'EOF'
# Instructions for Adding CSI Logging to srsRAN

## Step 1: Locate the SRS Channel Estimation Function

The SRS channel estimation typically happens in one of these files:
- `lib/phy/upper/channel_processors/srs_processor.cpp`
- `lib/phy/upper/signal_processors/srs_processor.cpp`
- `lib/phy/upper/channel_processors/channel_estimator.cpp`

Look for functions that:
- Process SRS signals
- Estimate channel from SRS
- Have names like `process_srs()`, `estimate_channel()`, `handle_srs()`

## Step 2: Add File I/O Headers

At the top of the file, add:
```cpp
#include <fstream>
#include <iomanip>
#include <chrono>
```

## Step 3: Add CSI Logging Function

Add this helper function to the class or as a static function:

```cpp
static void log_csi_to_file(const std::string& filename,
                           const channel_measurement& csi,
                           uint32_t slot_idx,
                           uint32_t symbol_idx,
                           uint16_t rnti) {
    static std::ofstream csi_file;
    static std::string current_file;
    
    // Create new file every minute or when filename changes
    std::string timestamp = std::to_string(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
    std::string full_path = "/tmp/csi_logs/csi_" + timestamp + ".csv";
    
    if (current_file != full_path) {
        if (csi_file.is_open()) {
            csi_file.close();
        }
        csi_file.open(full_path, std::ios::app);
        current_file = full_path;
        
        // Write header if new file
        csi_file << "timestamp,slot,symbol,rnti,"
                 << "nof_prb,start_prb,"
                 << "nof_rx_ports,nof_tx_layers,"
                 << "csi_real,csi_imag,"
                 << "rsrp_db,rsrq_db,sinr_db\n";
    }
    
    if (csi_file.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        csi_file << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
                 << "." << std::setfill('0') << std::setw(3) << ms.count()
                 << "," << slot_idx
                 << "," << symbol_idx
                 << "," << rnti;
        
        // Log channel measurement data
        // Adjust these based on the actual channel_measurement structure
        for (unsigned port = 0; port < csi.get_nof_ports(); ++port) {
            for (unsigned layer = 0; layer < csi.get_nof_layers(); ++layer) {
                for (unsigned prb = 0; prb < csi.get_nof_prb(); ++prb) {
                    for (unsigned re = 0; re < 12; ++re) { // 12 REs per PRB
                        auto csi_value = csi.get_channel(port, layer, prb, re);
                        csi_file << "," << csi.get_nof_prb()
                                 << "," << csi.get_start_prb()
                                 << "," << csi.get_nof_rx_ports()
                                 << "," << csi.get_nof_tx_layers()
                                 << "," << csi_value.real()
                                 << "," << csi_value.imag();
                    }
                }
            }
        }
        
        // Log measurements if available
        if (csi.has_measurements()) {
            csi_file << "," << csi.get_rsrp_db()
                     << "," << csi.get_rsrq_db()
                     << "," << csi.get_sinr_db();
        } else {
            csi_file << ",,,";
        }
        
        csi_file << "\n";
        csi_file.flush();
    }
}
```

## Step 4: Call the Logging Function

In the function that processes SRS and estimates the channel, add a call to log the CSI:

```cpp
// After channel estimation is complete
channel_measurement csi = estimate_channel_from_srs(...);

// Log CSI to file
log_csi_to_file("/tmp/csi_logs/csi.csv", csi, slot_idx, symbol_idx, rnti);
```

## Step 5: Rebuild srsRAN

After making the modifications:

```bash
cd /opt/srsRAN_Project/build
make -j$(nproc)
sudo make install
```

## Step 6: Restart gNodeB

After rebuilding, restart the gNodeB to use the modified code.

## Note

The exact structure of `channel_measurement` may vary. You'll need to inspect the actual class definition to adjust the logging code accordingly.
EOF

echo "Created modification instructions at: $CSI_LOG_DIR/MODIFICATION_INSTRUCTIONS.md"
echo ""
echo "Next steps:"
echo "1. Review the instructions in $CSI_LOG_DIR/MODIFICATION_INSTRUCTIONS.md"
echo "2. Locate the exact SRS channel estimation function using find_srs_csi_estimation.sh"
echo "3. Apply the modifications as described"
echo "4. Rebuild srsRAN: cd $SRSRAN_DIR/build && make -j\$(nproc) && sudo make install"

