# CSI Logging Scripts

These scripts help you add CSI logging functionality to srsRAN for your research.

## Availability on Experimental Node

When you deploy this profile on POWDER, these scripts are automatically available in two locations:

1. **`/local/repository/scripts/`** - Direct access to the profile repository
2. **`/opt/csi_logging/`** - Copied during deployment for easy access
3. **`~/csi_logging/`** - Symlink to `/opt/csi_logging/` in your home directory

## Quick Access

After your experiment is deployed, you can immediately use:

```bash
# Find SRS processing code
~/csi_logging/find_srs_csi_estimation.sh /opt/srsRAN_Project

# Get modification instructions
~/csi_logging/add_csi_logging.sh /opt/srsRAN_Project /tmp/csi_logs

# Analyze logged CSI
python3 ~/csi_logging/extract_csi.py /tmp/csi_logs --analyze
```

## Files

- `find_srs_csi_estimation.sh` - Locates SRS channel estimation code
- `add_csi_logging.sh` - Generates modification instructions
- `csi_logger.h` - CSI logger header file
- `csi_logger.cpp` - CSI logger implementation
- `extract_csi.py` - Python tool to analyze logged CSI
- `INTEGRATION_GUIDE.md` - Detailed integration instructions

## Documentation

See `/opt/csi_logging/CSI_LOGGING_README.md` for full documentation.

