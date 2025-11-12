# CSI Logging for srsRAN 5G NR Research

This guide explains how to extract Channel State Information (CSI) from SRS (Sounding Reference Signals) in your srsRAN 5G NR simulation for research on CSI extrapolation.

## Overview

In TDD 5G NR systems, the base station (gNodeB) estimates CSI using uplink SRS signals. Since UEs send SRSs only over partial bandwidths (due to power limits), the gNodeB receives only partial CSI and needs to extrapolate the rest. This setup helps you capture the partial CSI for your research.

## Quick Start

1. **Locate the SRS channel estimation code:**
   ```bash
   cd /path/to/srsran5g-simulated-rf
   chmod +x scripts/find_srs_csi_estimation.sh
   ./scripts/find_srs_csi_estimation.sh /opt/srsRAN_Project
   ```

2. **Review modification instructions:**
   ```bash
   chmod +x scripts/add_csi_logging.sh
   ./scripts/add_csi_logging.sh /opt/srsRAN_Project /tmp/csi_logs
   cat /tmp/csi_logs/MODIFICATION_INSTRUCTIONS.md
   ```

3. **Apply the modifications** to the srsRAN source code as described in the instructions

4. **Rebuild srsRAN:**
   ```bash
   cd /opt/srsRAN_Project/build
   make -j$(nproc)
   sudo make install
   ```

5. **Run your simulation** and CSI data will be logged to `/tmp/csi_logs/`

6. **Extract and analyze CSI:**
   ```bash
   python3 scripts/extract_csi.py /tmp/csi_logs --analyze
   python3 scripts/extract_csi.py /tmp/csi_logs --export csi_data.npz
   ```

## Understanding the Code Structure

### Where CSI Estimation Happens

In srsRAN Project, SRS processing and channel estimation typically occur in:

- **`lib/phy/upper/channel_processors/`** - Channel processing functions
- **`lib/phy/upper/signal_processors/`** - Signal processing functions
- **`lib/phy/upper/`** - Upper PHY layer functions

Look for files containing:
- `srs_processor` - Main SRS processing logic
- `channel_estimator` - Channel estimation functions
- Functions named `process_srs()`, `estimate_channel()`, `handle_srs()`

### Key Data Structures

The CSI is typically stored in a structure like:
- `channel_measurement` - Contains estimated channel coefficients
- `srs_processor_result` - Result of SRS processing
- Channel state matrices (complex values per resource element)

## Modification Details

### What to Modify

You need to find the function that:
1. Receives SRS signals from the UE
2. Performs channel estimation on the SRS
3. Stores the estimated channel coefficients

Then add code to:
1. Extract the channel coefficients (CSI)
2. Write them to a file with metadata (timestamp, slot, symbol, RNTI, PRB indices, etc.)

### Example Modification Pattern

```cpp
// In the SRS processing function, after channel estimation:

// Original code:
channel_measurement csi = estimate_channel_from_srs(srs_signal, ...);

// Add logging:
log_csi_to_file(csi, slot_idx, symbol_idx, rnti, prb_start, prb_count);
```

### CSI Data Format

The logged CSI should include:
- **Timestamp** - When the CSI was estimated
- **Slot/Symbol** - Time domain location
- **RNTI** - Radio Network Temporary Identifier (UE identifier)
- **PRB information** - Which physical resource blocks were used
- **Channel coefficients** - Complex values (real + imaginary parts)
- **Power measurements** - RSRP, RSRQ, SINR if available

## Using the Analysis Scripts

### Extract CSI Data

```bash
# Analyze CSI statistics
python3 scripts/extract_csi.py /tmp/csi_logs --analyze

# Export to NumPy format for further analysis
python3 scripts/extract_csi.py /tmp/csi_logs --export csi_data.npz

# Load in Python for research
import numpy as np
data = np.load('csi_data.npz')
csi = data['csi']  # Complex channel coefficients
metadata = data['metadata']  # Associated metadata
```

### Analyzing Partial CSI

Since you're researching CSI extrapolation, you'll want to:
1. Identify which PRBs have CSI (from SRS)
2. Identify which PRBs are missing (need extrapolation)
3. Extract the partial CSI matrix for your extrapolation algorithms

## Troubleshooting

### Can't find SRS processing code?

1. Make sure srsRAN Project is installed at `/opt/srsRAN_Project`
2. Check if the source code is available (not just binaries)
3. Search manually:
   ```bash
   find /opt/srsRAN_Project -name "*srs*" -type f | grep -E "\.(cpp|h)$"
   ```

### CSI not being logged?

1. Check file permissions on `/tmp/csi_logs/`
2. Verify the logging function is being called
3. Check gNodeB logs for errors
4. Ensure the channel estimation function is actually being executed

### Need to rebuild?

After modifying source code:
```bash
cd /opt/srsRAN_Project/build
make clean
make -j$(nproc)
sudo make install
# Restart gNodeB
```

## Research Workflow

1. **Capture partial CSI** - Use this setup to log CSI from SRS
2. **Identify coverage** - Determine which frequency bands have CSI
3. **Develop extrapolation** - Create algorithms to predict missing CSI
4. **Validate** - Compare extrapolated CSI with ground truth (if available)

## Additional Resources

- [srsRAN Project Documentation](https://docs.srsran.com/)
- [srsRAN GitHub Repository](https://github.com/srsran/srsRAN_Project)
- [3GPP TS 38.211 - Physical channels and modulation](https://www.3gpp.org/DynaReport/38211.htm)
- [3GPP TS 38.214 - Physical layer procedures for data](https://www.3gpp.org/DynaReport/38214.htm)

## Notes

- The exact code structure may vary between srsRAN versions
- You may need to adjust the logging code based on the actual data structures used
- Consider the performance impact of file I/O during real-time operation
- For high-throughput scenarios, consider using binary formats or buffering

