# Quick Start: CSI Logging for srsRAN Research

## Overview

This profile helps you extract partial CSI from SRS signals in your 5G NR simulation for CSI extrapolation research.

## Quick Steps

### 1. Find the CSI Estimation Code

```bash
cd /path/to/srsran5g-simulated-rf
./scripts/find_srs_csi_estimation.sh /opt/srsRAN_Project
```

This will show you where SRS processing happens in srsRAN.

### 2. Add CSI Logging

1. **Copy the logger files** to srsRAN source:
   ```bash
   cp scripts/csi_logger.h /opt/srsRAN_Project/lib/include/srsran/phy/upper/
   cp scripts/csi_logger.cpp /opt/srsRAN_Project/lib/src/phy/upper/channel_processors/
   ```

2. **Modify the SRS processor** to call the logger (see `INTEGRATION_GUIDE.md`)

3. **Update CMakeLists.txt** to include `csi_logger.cpp`

4. **Rebuild srsRAN**:
   ```bash
   cd /opt/srsRAN_Project/build
   make -j$(nproc)
   sudo make install
   ```

### 3. Run Your Simulation

Start gNodeB and UE as usual. CSI will be logged to `/tmp/csi_logs/`.

### 4. Extract CSI Data

```bash
# Analyze the logged CSI
python3 scripts/extract_csi.py /tmp/csi_logs --analyze

# Export for research
python3 scripts/extract_csi.py /tmp/csi_logs --export csi_data.npz
```

## Files Created

- `scripts/find_srs_csi_estimation.sh` - Locates SRS processing code
- `scripts/add_csi_logging.sh` - Generates modification instructions
- `scripts/csi_logger.h` - CSI logger header
- `scripts/csi_logger.cpp` - CSI logger implementation
- `scripts/extract_csi.py` - Analysis tool for logged CSI
- `CSI_LOGGING_README.md` - Detailed documentation
- `INTEGRATION_GUIDE.md` - Step-by-step integration instructions

## Key Points

- **Partial CSI**: SRS only covers part of the bandwidth → you get partial CSI
- **Log Location**: `/tmp/csi_logs/csi_*.csv` by default
- **Data Format**: CSV with timestamp, slot, symbol, RNTI, PRB, RE, and channel coefficients
- **Research Use**: Identify which PRBs have CSI, then extrapolate the missing ones

## Need Help?

- See `CSI_LOGGING_README.md` for detailed documentation
- See `INTEGRATION_GUIDE.md` for integration steps
- Check srsRAN documentation for exact data structure names

