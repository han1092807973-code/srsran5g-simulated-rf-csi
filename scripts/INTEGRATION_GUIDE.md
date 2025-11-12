# Integration Guide: Adding CSI Logging to srsRAN

This guide provides step-by-step instructions for integrating CSI logging into the srsRAN Project source code.

## Step 1: Locate the SRS Processor File

Run the find script to locate SRS processing code:

```bash
./scripts/find_srs_csi_estimation.sh /opt/srsRAN_Project
```

Common locations:
- `lib/phy/upper/channel_processors/srs_processor.cpp`
- `lib/phy/upper/signal_processors/srs_processor.cpp`
- `lib/phy/upper/channel_processors/channel_estimator.cpp`

## Step 2: Copy CSI Logger Files

Copy the CSI logger files to the srsRAN source tree:

```bash
# Find the appropriate include directory
SRSRAN_INCLUDE="/opt/srsRAN_Project/lib/include/srsran"

# Copy header file
cp scripts/csi_logger.h $SRSRAN_INCLUDE/phy/upper/

# Find the source directory (usually same structure)
SRSRAN_SRC="/opt/srsRAN_Project/lib/src/phy/upper"

# Copy implementation file
cp scripts/csi_logger.cpp $SRSRAN_SRC/channel_processors/
```

## Step 3: Modify CMakeLists.txt

Add the CSI logger source file to the build system. Find the relevant `CMakeLists.txt` (usually in `lib/src/phy/upper/` or `lib/src/phy/upper/channel_processors/`) and add:

```cmake
# Add csi_logger.cpp to the source files list
list(APPEND SOURCES
    ...
    channel_processors/csi_logger.cpp
    ...
)
```

## Step 4: Find the SRS Channel Estimation Function

In the SRS processor file, locate the function that:
1. Receives SRS signals
2. Performs channel estimation
3. Returns or stores channel measurements

Look for function signatures like:
```cpp
void process_srs(const srs_processor_config& config, ...);
channel_measurement estimate_channel(const srs_signal& signal, ...);
void handle_srs(const srs_resource& resource, ...);
```

## Step 5: Add CSI Logging Call

After the channel estimation is complete, add a call to log the CSI. The exact code depends on the data structures used, but here are common patterns:

### Pattern 1: If you have a channel_measurement object

```cpp
#include "srsran/phy/upper/csi_logger.h"

// In your SRS processing function, after channel estimation:
channel_measurement csi = estimate_channel_from_srs(...);

// Extract channel matrix and log
std::vector<std::vector<std::vector<std::complex<float>>>> channel_matrix;
// ... populate channel_matrix from csi object ...

srsran::csi_logger::log_csi(
    slot_idx,
    symbol_idx,
    rnti,
    prb_start,
    nof_prb,
    channel_matrix,
    csi.get_rsrp_db(),  // if available
    csi.get_rsrq_db(),  // if available
    csi.get_sinr_db()   // if available
);
```

### Pattern 2: If you have direct access to channel coefficients

```cpp
#include "srsran/phy/upper/csi_logger.h"

// After channel estimation:
// Assuming you have channel coefficients stored as:
// channel_coeffs[prb][re] = complex<float>

std::vector<std::vector<std::complex<float>>> channel_coeffs;
// ... populate from your channel estimation ...

srsran::csi_logger::log_csi_simple(
    slot_idx,
    symbol_idx,
    rnti,
    prb_start,
    nof_prb,
    channel_coeffs,
    rsrp_db  // if available
);
```

### Pattern 3: If using srsRAN's channel_state_information structure

You may need to adapt based on the actual structure. Example:

```cpp
#include "srsran/phy/upper/csi_logger.h"

// After channel estimation:
channel_state_information csi = ...;

// Convert to the format expected by logger
std::vector<std::vector<std::vector<std::complex<float>>>> channel_matrix;

for (unsigned port = 0; port < csi.get_nof_rx_ports(); ++port) {
    std::vector<std::vector<std::complex<float>>> port_data;
    for (unsigned layer = 0; layer < csi.get_nof_tx_layers(); ++layer) {
        std::vector<std::complex<float>> layer_data;
        for (unsigned prb = 0; prb < csi.get_nof_prb(); ++prb) {
            std::vector<std::complex<float>> prb_data;
            for (unsigned re = 0; re < 12; ++re) {
                prb_data.push_back(csi.get_channel(port, layer, prb, re));
            }
            layer_data.insert(layer_data.end(), prb_data.begin(), prb_data.end());
        }
        port_data.push_back(layer_data);
    }
    channel_matrix.push_back(port_data);
}

srsran::csi_logger::log_csi(
    slot_idx,
    symbol_idx,
    rnti,
    csi.get_start_prb(),
    csi.get_nof_prb(),
    channel_matrix,
    csi.get_rsrp_db(),
    csi.get_rsrq_db(),
    csi.get_sinr_db()
);
```

## Step 6: Initialize Logger (Optional)

If you want to specify a custom log directory, initialize the logger early (e.g., in the SRS processor constructor or initialization function):

```cpp
// In initialization code:
srsran::csi_logger::init("/path/to/csi/logs");
```

Otherwise, it will auto-initialize to `/tmp/csi_logs` on first use.

## Step 7: Rebuild srsRAN

```bash
cd /opt/srsRAN_Project/build
make clean
make -j$(nproc)
sudo make install
```

## Step 8: Test

1. Start your gNodeB:
   ```bash
   sudo /opt/srsRAN_Project/build/apps/gnb/gnb -c /etc/srsran/gnb.yml
   ```

2. Connect a UE and ensure SRS is being transmitted

3. Check for CSI log files:
   ```bash
   ls -lh /tmp/csi_logs/
   ```

4. Verify the logs contain data:
   ```bash
   head -20 /tmp/csi_logs/csi_*.csv
   ```

## Troubleshooting

### Compilation Errors

- **Missing header**: Ensure `csi_logger.h` is in the include path
- **C++17 features**: Ensure your CMakeLists.txt sets `CXX_STANDARD 17` or higher
- **filesystem library**: On older systems, you may need to link `-lstdc++fs`

### No CSI Logs Generated

1. Check if the logging function is being called (add debug prints)
2. Verify file permissions on `/tmp/csi_logs/`
3. Check gNodeB logs for errors
4. Ensure SRS signals are actually being received and processed

### Incorrect Data Format

- Adjust the logging call based on your actual data structures
- Check the CSV output format matches your expectations
- Verify PRB indices and RE indices are correct

## Customization

### Change Log Directory

```cpp
srsran::csi_logger::init("/custom/path/to/logs");
```

### Change Log Format

Modify `csi_logger.cpp` to change the CSV format or add additional fields.

### Add Filtering

Modify the logger to only log specific RNTIs, slots, or PRB ranges.

## Next Steps

After successfully logging CSI:

1. Use `scripts/extract_csi.py` to analyze the data
2. Identify which PRBs have CSI (partial bandwidth from SRS)
3. Develop your CSI extrapolation algorithms
4. Validate against ground truth if available

